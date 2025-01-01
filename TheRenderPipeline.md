# The Render Pipeline

Now that we know how to render objects using the GPU, we'll look at the different steps needed to render a complete scene with many different graphics features like transparency, shadows, fog, etc. This list of steps is what we call the **Render Pipeline** and essentially describes all the process needed to compose the final image on your screen. Each step in the render pipeline is often referred to as a **Render Pass**, the render pipeline can then be organized as a list of passes each playing a different role in the render pipeline.

The following diagram provides a very high level overview of the content of a generic render pipeline.

![](Media/Images/RenderPipeline.png)

Note that the techniques used to render each box greatly varies depending on the requirements of the renderer. To achieve optimal performance, the design of the renderer must take in account the kind of content in your 3D scenes.

- Lots of static geometry VS dynamic geometry
- Large open world VS small map
- Massive instancing and reuse of models VS lots of unique assets
- Lots of materials VS a few materials
- Highly detailed meshes with small triangles and automatic LODs VS manually authored LODs
- Lots of dynamic lights VS baked lighting

All of these choices helps determine the best technology to adopt for your renderer.

## Offscreen Shadows

One of the first steps in the render pipeline is to compute offscreen shadows, this is necessary if the renderer is using [shadow maps](https://en.wikipedia.org/wiki/Shadow_mapping) for example but not required if you plan on using other shadowing techniques like ray-traced shadows.

This step consist into rendering visibility information (depth) for each light in your scene so that they can cast shadows.

We'll see in more details the shadowing techniques in a future chapter. In the mean time if you're interested in how shadow maps works, you can read [Common Techniques to Improve Shadow Depth Maps](https://learn.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps).

## Opaque Rendering

Rendering opaque objects is often the first step, this is because it allows to generate a depth buffer used by the following passes and thus minimizes the amount of overdraw in the scene. Additionally the depth buffer is useful for effect like **Screen Space Ambient Occlusion**, **Depth of Field**, **Contact Shadows**, etc.

There are a lot of techniques to render opaque geometry and they can be split in two main categories: Forward and Deferred rendering.

### Forward Opaque Rendering

![](Media/Recordings/The%20Render%20Pipeline%20-%20Forward.gif)

Forward rendering is the simplest way to render an object, this is because it consist into directly rendering the object in the main color and depth textures. Because of this single rendering pass, the shader of these objects have to handle the vertex transformation, evaluate/load the material properties (constants, textures, procedural functions, etc.) and shade the surface with the lighting setup of the scene. Usually this result in shaders that consume a lot of resources and are limits the parallelism on the GPU.

Forward rendering is very useful when rendering complex materials, these are materials that have a parametrization that is different from "standard" objects in a scene like Hairs, Skin, Eyes, car paints, etc. With the forward approach these parameters don't need to be stored to be shaded later on which would require a lot of memory.

One of the main problem with forward rendering is the overdraw, each pixels evaluated by forward opaque objects are quite expensive so it's important to avoid shading more than we can see. The issue here is that there is no depth buffer as it's being constructed in the forward opaque pass, which means that a lot of objects will get shaded and the overwritten by another surface closer to the camera. This is why most engines use a variant of deferred rendering for rendering opaque geometry.

> Note that on some GPU forward rendering can still the most performant option, especially mobile that uses Tile-Based Deferred Rendering technology. In this case the deferred part is done in hardware and eliminate overdraw without having to prepare a depth buffer beforehand.

Having a high pixel shading cost also worsen the [quad occupancy](Rasterization.md#quads--helper-pixels) problem as the helper pixels will share this high cost making the GPU use a lot more resources than necessary.

### Deferred Opaque Rendering

Deferred rendering like the name implies mean shading the pixels in two or more separate passes. An intermediate texture that stores data related to the geometry or material properties is then used to pass data from one pass to the other. There are a lot of variants of deferred rendering and each of them have their pros and cons depending on the content of the scene.

#### Depth Pre-Pass

![](Media/Recordings/The%20Render%20Pipeline%20-%20Depth%20Prepass.gif)

As the name implies, this pass renders the depth of every objects into a depth buffer, this depth buffer can then be use to render objects without any overdraw using the [early depth testing](Rasterization.md) feature of the rasterizer. 

The depth pre-pass can be combined with other techniques such as a geometry buffer pass (G-Buffer) or followed directly by a forward opaque pass.

One of the key advantage of the depth pre-pass is that most of the objects can be rendered with the same shader code as long as the shader doesn't modify the pixel depth. This result in a faster render loop execution on CPU.

Something worth noting with the depth pre-pass is that every objects need to be rendered twice (once for the depth pre-pass and once for the material or shading pass), so by adding a depth pre-pass we're effectively doing redundant work like executing the vertex transformation twice for each vertex in view. Though, most of the time, having a depth pre-pass is still a win as overdraw is often more expensive than vertex transformations.

#### Geometry Buffer (G-Buffer)

![](Media/Recordings/The%20Render%20Pipeline%20-%20GBuffer.gif)

The geometry buffer technique consist in render the geometric attributes and material properties into multiple render targets to later use in a lighting pass. This technique allows to decouple the material evaluation and lighting computation in two separate passes. Decoupling material and lighting is interesting as it allows to control the complexity of the materials and geometry without affecting the cost of the lighting pass.

In a physically based render, the G-Buffer stores PBR material properties such as albedo, normal, roughness, ambient occlusion, etc. Due to the amount of data to store, the G-Buffer can become quite big and it's essential to compress and quantize every property to maximize the bandwidth as it's one of the main limiting factor of the G-Buffer pass.

The G-Buffer pass can also output depth, if the material evaluation is fast, then overdraw is less an issue (it'd be like a depth pre-pass but with more data as output). On the other hand, having complex material evaluation requiring parallax occlusion mapping, triplanar mapping or procedural material evaluation make it interesting to have a depth pre-pass before the G-Buffer pass, eliminating the overdraw issue.

#### Visibility Buffer

![](Media/Recordings/The%20Render%20Pipeline%20-%20Visibility%20Buffer.gif)

If the purpose of the G-Buffer is to decouple material and lighting, then it can be said that the purpose of the visibility buffer is to decouple geometry from materials and lighting. The benefit of decoupling geometry is that the objects will only get rendered a single time unlike with the depth pre-pass approach, furthermore it also prevent overdraw because the visibility buffer stores both visibility information in a color target and depth.

The visibility buffer builds on the principle of the depth pre-pass as it's essentially the same principle but with an additional visibility information output. This visibility information contains everything necessary to rebuild the geometric structure of the mesh in the following passes. One way to encode visibility consist into storing both the triangle ID and draw ID packed into a single 32 bit texture. The size limit of 32 bit is very important as we want to maximize the bandwidth usage of this pass, it'd be bad if we ended-up with a G-Buffer like structure where the bandwidth becomes the limiting speed factor.

The triangle ID refers to an index pointing to the vertices of a triangle, with this index, we can get back the 3 vertices forming the triangle. This buffer can either be the vertex buffer of the mesh, ordered in a particular way or a dedicated vertex buffer storing multiple meshes.

The draw call ID is also an index, this time pointing to the draw data which contains the object matrix, material ID, etc.

The material ID is an index as well, this time pointing to the material data, this ID is enough to evaluate the material.

Like the depth pre-pass, the visibility buffer can be combined with the G-Buffer pass, but the G-Buffer pass in this case is a single pass that convert the visibility information into a G-Buffer. This pass can also be called deferred material as there is no need to draw the objects but only fetch data from the visibility buffer, reconstruct interpolators and evaluate the materials.

The visibility buffer approach is very popular right now as it opens the door to new optimizations and techniques (materials in compute shaders, software Rasterization, etc.). If you're interested in this topic, take a look at [Visibility Buffer Rendering with Material Graphs](http://filmicworlds.com/blog/visibility-buffer-rendering-with-material-graphs/).

### Lighting

Opaque lighting is also a complex topic with many existing solutions. As mentioned before, there is forward lighting and deferred lighting. Fortunately for us, they both use identical lighting algorithms so we only need to care about lighting a surface whether it comes from a pixel rendered in forward or deferred doesn't matter.

We'll see how the lighting is structured in future chapters when talking about how to approach the different light types but to give you an overview, we like to separate the big lighting problem into smaller ones that can be solved separately and then combined as a final approximation.

We can split the lighting of opaque surfaces in 4 different categories:

- Direct diffuse lighting: illumination caused by light hitting a surface directly and scattering uniformly in all directions.
- Direct specular lighting: illumination from light hitting a surface directly and reflecting in a specific direction, creating sharp highlights on smooth surfaces.
- Indirect diffuse lighting: Light that first bounces off other surfaces before reaching the target surface, scattering uniformly in all directions, often misnamed "Global Illumination".
- Indirect specular lighting: Light that first bounces off other surfaces before reaching the target surface and reflecting in a specific direction, creating highlights and reflections.

Each of those categories will be solved by a particular algorithm or system. Note that some of these systems are also used for transparent objects in later passes.

## Sky Rendering

![](Media/Recordings/The%20Render%20Pipeline%20-%20Sky.png)

> HDRI sky background with a chrome ball reflecting it.

Sky rendering is pretty straightforward and consist in a full-screen pass (a shader executed on all pixels of the screen), this shader uses the depth test to avoid overwriting the opaque objects. It has the added benefit that the sky is not computed on any pixels with opaque objects already rendered, which makes this pass very cheap if the sky is barely visible.

We can split the sky rendering in two categories:

- Physically based skies
- Artistic skies

The physically based skies are derived from an approximation of the equation of the scattering of light in the atmosphere. Most skies in AAA games are based on similar techniques. The physical sky is great because it behaves naturally in any lighting conditions, which makes it easy to do dynamic time of day, night sky rendering, integration with clouds, atmosphere viewed from space, etc.

On the other side, artistic skies relies on procedural or baked data to achieve compelling look. They are often more static and require to be tweaked per location in the world to match the required visuals. A simple example of artistic sky is the HDRI sky, a spherical HDRI (High Dynamic Range Image) like the image above.

As you can see in this example, the "sky" doesn't only contain the rendering of the atmosphere but also a bunch of other objects. The idea is to treat them as if they were at an infinite distance from the camera, hence the term "sky" is used when talking about them.

## Transparency and Visual Effects

This is probably the biggest category in the render pipeline, it contains any other effect that cannot be represented by opaque geometry and is not infinitely far away from the camera. For example this category includes:

- Transparency, translucency
- Order Independent Transparency
- Volumetric fog / clouds
- Transparent Reflections
- Refraction
- Decals
- Hair rendering with a software rasterizer
- Water system
- etc.

In future chapters we'll have the opportunity to talk about some of these effects, but in the mean time I'll drop a few links that you can check out if you're interested.

- [Why Transparency Is Hard](https://shaderfun.com/2020/09/20/why-transparency-is-hard/)
- [Creating the Atmospheric World of Red Dead Redemption 2: A Complete and Integrated Solution [PPTX]](https://advances.realtimerendering.com/s2019/slides_public_release.pptx)
- [Volumetric Fog in Enshrouded](https://www.youtube.com/watch?v=OR8HbFnQdlk)
- [Decals (deferred rendering)](https://mtnphil.wordpress.com/2014/05/24/decals-deferred-rendering/)
- [Interactive Wind and Vegetation in 'God of War'](https://www.youtube.com/watch?v=MKX45_riWQA)

## Post Processing

Post processing is the last step of the render pipeline and consist in applying a set of image-base effects to the main color buffer.

Post processing are often tweaked artistically to achieve a certain visual style, they can also also help enhancing the visual quality of the image by reducing aliasing or adding filmic effects.

One of the big part of post processing comes from emulating the behavior of a real camera. Following the principles of PBR, the camera has physical properties exposed such as exposure, aperture, ISO, sensor size, etc. These properties translates directly to post processing effects such as [bloom](https://en.wikipedia.org/wiki/Bloom_(shader_effect)), [dynamic exposure](https://en.wikipedia.org/wiki/Exposure_(photography)), [motion blur](https://en.wikipedia.org/wiki/Motion_blur), [depth of field](https://en.wikipedia.org/wiki/Depth_of_field), [film grain (sensor noise)](https://en.wikipedia.org/wiki/Film_grain), [chromatic aberration](https://en.wikipedia.org/wiki/Chromatic_aberration), [vignetting](https://en.wikipedia.org/wiki/Vignetting) and [lens flares](https://en.wikipedia.org/wiki/Lens_flare).

Other post processes have purely an artistic purpose like color [curves control](https://en.wikipedia.org/wiki/Curve_(tonality)), changing HUE, etc. This is something that we also find in image editing softwares. In the same category we can put gameplay-based post processes, for example displaying blood on the edges of the screen when the character health is low, etc.

[Anti-aliasing](https://en.wikipedia.org/wiki/Anti-aliasing_filter) post processes like TAA or SMAA have the purpose to reduce the staircase effects on screen to the cost of some blurriness introduced. Nowadays anti-aliasing algorithms are often combined with up-sampling (super sampling or up-scaling), allowing the game to run on a lower resolution to maintain acceptable performances during gameplay (DLSS, FSR, XeSS, PSSR, Arm ASR, etc.).

Finally, tone mapping is one of the most important step of post processing, this one actually requires information of the screen to work and it's often the last post process applied. It's goal is to convert the linear HDR colors used throughout the rendering into something that the screen can display. You can see this operation like a remapping of values, this is especially important to support HDR screens.

## Render Pipeline Design in code

While this is not a part of the render pipeline, I still wanted to mention the common architecture used to represent render pipelines in code.

You can imagine that with the increasing amount of feature needed to render highly detailed scenes or photorealistic content, it becomes hard to organize the data layout and code executing the rendering. A recent development has pushed graphics engineers towards a graph representation of the frame where each render pass is a node with clear and fixed inputs and outputs.

This graph representation is often called **Frame Graph** or **Render Graph**, and allows to simplify the synchronization between resources that need to happen between render passes. It also simplifies the re-using resources between passes to save a lot of render target memory. Additionally, analyzing the graph allows to dynamically remove unused passes, simplifying the overall logic of the pipeline and helping modularize the code of certain features.

Of course introducing a graph also comes with a CPU performance cost as more calculation are necessary to record the graph layout, cull nodes, etc. But the benefits far overcome the downsides and there are several strategies to alleviate this performance cost such as caching the layout of the graph if the feature set doesn't change.

The implementation of a render graph is a complex topic, so if you want to learn more I recommend reading [Render Graphs](https://logins.github.io/graphics/2021/05/31/RenderGraphs.html).

> While render graphs are implemented in all major game engines, they are not necessary, especially if your renderer is very simple and you don't plan on adding tons of stuff on top of it.

## Conclusion

The render pipeline is a modular system that takes 3D scene data and processes it into the final 2D image displayed on your screen. Each step, or Render Pass, contributes to different aspects of the image, from handling geometry and lighting to applying advanced post-processing effects. The design of the render pipeline is highly dependent on the requirements of the renderer, such as the types of scenes, target platforms, and performance constraints.

Modern renderers also heavily rely on tools like Frame Graphs to optimize resource usage and streamline complex dependencies between passes. If you're interested in seeing how these concepts are applied in real-world games, check out [Adrian Courrèges Graphics Studies Compilation](https://www.adriancourreges.com/blog/2020/12/29/graphics-studies-compilation/) for a breakdown of the render pipelines in popular titles. These analyses provide great insight into how different games leverage the render pipeline to achieve their specific visual goals while maintaining performance.

## References

https://docs.unity3d.com/Packages/com.unity.render-pipelines.high-definition@17.0/manual/rendering-execution-order.html

https://dev.epicgames.com/community/learning/tutorials/vyZ1/unreal-engine-begin-play-rendering

https://www.gdcvault.com/play/1024612/FrameGraph-Extensible-Rendering-Architecture-in

https://logins.github.io/graphics/2021/05/31/RenderGraphs.html

http://filmicworlds.com/blog/visibility-buffer-rendering-with-material-graphs/

https://en.wikipedia.org/wiki/Shadow_mapping

https://diaryofagraphicsprogrammer.blogspot.com/2018/03/triangle-visibility-buffer.html
