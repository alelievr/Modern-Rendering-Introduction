# The Render Pipeline

Now that we know how to render objects using the GPU, we'll look at the different steps needed to render a complete scene with various graphics features like transparency, shadows, fog, etc. This list of steps is called the **Render Pipeline**, which essentially describes all the processes needed to compose the final image on your screen. Each step in the render pipeline is often referred to as a **Render Pass**, and the render pipeline can then be organized as a list of passes, each playing a different role in the rendering process.

The following diagram provides a high-level overview of the content of a generic render pipeline:

![](Media/Images/RenderPipeline.png)

Note that the techniques used to render each box vary greatly depending on the requirements of the renderer. To achieve optimal performance, the design of the renderer must take into account the type of content in your 3D scenes:

- Lots of static geometry vs. dynamic geometry
- Large open worlds vs. small maps
- Massive instancing and reuse of models vs. lots of unique assets
- Many materials vs. a few materials
- Highly detailed meshes with small triangles and automatic LODs vs. manually authored LODs
- Numerous dynamic lights vs. baked lighting

All these choices help determine the best technology to adopt for your renderer.

## Offscreen Shadows

One of the first steps in the render pipeline is to compute offscreen shadows. This is necessary if the renderer is using [shadow maps](https://en.wikipedia.org/wiki/Shadow_mapping), for example, but not required if you plan on using other shadowing techniques like ray-traced shadows.

This step consists of rendering visibility information (depth) for each light in your scene so that they can cast shadows.

We will explore shadowing techniques in more detail in a future chapter. In the meantime, if you're interested in how shadow maps work, you can read [Common Techniques to Improve Shadow Depth Maps](https://learn.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps).

## Opaque Rendering

Rendering opaque objects is often the first step because it allows for generating a depth buffer used by the following passes, minimizing the amount of overdraw in the scene. Additionally, the depth buffer is useful for effects like **Screen Space Ambient Occlusion**, **Depth of Field**, **Contact Shadows**, etc.

There are many techniques to render opaque geometry, which can be split into two main categories: Forward and Deferred rendering.

### Forward Opaque Rendering

![](Media/Recordings/The%20Render%20Pipeline%20-%20Forward.gif)

Forward rendering is the simplest way to render an object. This is because it consists of directly rendering the object into the main color and depth textures. Due to this single rendering pass, the shader for these objects has to handle vertex transformations, evaluate/load material properties (constants, textures, procedural functions, etc.), and shade the surface with the scene's lighting setup. Usually, this results in shaders that consume a lot of resources and limit parallelism on the GPU.

Forward rendering is particularly useful when rendering complex materials. These are materials with parametrizations that differ from "standard" objects in a scene, such as hair, skin, eyes, car paints, etc. With the forward approach, these parameters don't need to be stored for shading later, which would otherwise require a lot of memory.

One of the main problems with forward rendering is overdraw. Each pixel evaluated by forward opaque objects is quite expensive, so it's important to avoid shading more than what is visible. The issue arises because there is no pre-existing depth buffer as it is being constructed during the forward opaque pass. This means many objects get shaded only to be overwritten by another surface closer to the camera. For this reason, most engines use a variant of deferred rendering to handle opaque geometry.

> Note that on some GPUs, forward rendering can still be the most performant option, particularly on mobile devices that use Tile-Based Deferred Rendering technology. In this case, the deferred part is handled in hardware, eliminating overdraw without the need to prepare a depth buffer beforehand.

High pixel shading costs also exacerbate the [quad occupancy](Rasterization.md#quads--helper-pixels) problem, as the helper pixels will share this high cost, causing the GPU to use significantly more resources than necessary.

### Deferred Opaque Rendering

Deferred rendering, as the name implies, involves shading the pixels in two or more separate passes. A set of intermediate textures that stores data related to the geometry and material properties is then used to transfer data from one pass to the next. There are many variants of deferred rendering, each with its own pros and cons depending on the scene's content.

#### Depth Pre-Pass

![](Media/Recordings/The%20Render%20Pipeline%20-%20Depth%20Prepass.gif)

As the name suggests, this pass renders the depth of every object into a depth buffer. This depth buffer can then be used to render objects without any overdraw by leveraging the [early depth testing](Rasterization.md) feature of the rasterizer.

The depth pre-pass can be combined with other techniques, such as a geometry buffer pass (G-Buffer), or it can be followed directly by a forward opaque pass.

One of the key advantages of the depth pre-pass is that most objects can be rendered with the same shader code, as long as the shader doesn't modify the pixel depth. This results in faster render loop execution on the CPU.

It’s worth noting that every object needs to be rendered twice in this approach, once for the depth pre-pass and again for the material or shading pass. This means that adding a depth pre-pass effectively introduces redundant work, such as executing the vertex transformation twice for each visible vertex. However, in most cases, having a depth pre-pass is still advantageous, as overdraw is often more computationally expensive than vertex transformations.

#### Geometry Buffer (G-Buffer)

![](Media/Recordings/The%20Render%20Pipeline%20-%20GBuffer.gif)

The geometry buffer technique involves rendering the geometric attributes and material properties into multiple render targets to be used later in a lighting pass. This technique allows the decoupling of material evaluation and lighting computation into two separate passes. Decoupling material and lighting is beneficial because it enables control over the complexity of the materials and geometry without affecting the cost of the lighting pass.

In a physically based renderer, the G-Buffer stores PBR material properties such as albedo, normal, roughness, ambient occlusion, etc. Due to the amount of data being stored, the G-Buffer can become quite large, and it's essential to compress and quantize every property to maximize bandwidth usage, as it is one of the main limiting factors of the G-Buffer pass.

The G-Buffer pass can also output depth. If the material evaluation is fast, overdraw becomes less of an issue (this would be similar to a depth pre-pass but with more data as output). On the other hand, if complex material evaluations are required—such as parallax occlusion mapping, triplanar mapping, or procedural material evaluation—it becomes useful to include a depth pre-pass before the G-Buffer pass to eliminate the overdraw issue.

#### Visibility Buffer

![](Media/Recordings/The%20Render%20Pipeline%20-%20Visibility%20Buffer.gif)

If the purpose of the G-Buffer is to decouple materials from lighting, then the purpose of the visibility buffer is to decouple geometry from both materials and lighting. The advantage of decoupling geometry is that the objects are only rendered once, unlike with the depth pre-pass approach. Furthermore, it also prevents overdraw, as the visibility buffer stores both visibility information in a color target and depth.

The visibility buffer builds upon the principle of the depth pre-pass, as it essentially uses the same concept but adds an additional output for visibility information. This visibility data contains everything needed to reconstruct the geometric structure of the mesh in subsequent passes. One way to encode visibility is by storing both the triangle ID and draw ID in a single 32-bit texture. The 32-bit size limit is important because we want to maximize the bandwidth usage of this pass. It would be problematic if we ended up with a G-Buffer-like structure, where the bandwidth becomes the limiting factor.

The triangle ID refers to an index pointing to the vertices of a triangle. With this index, we can retrieve the 3 vertices that form the triangle. This buffer can either be the vertex buffer of the mesh, arranged in a specific way, or a dedicated vertex buffer storing multiple meshes.

The draw call ID is also an index, this time pointing to the draw data, which includes the object matrix, material ID, etc.

The material ID is an index as well, pointing to the material data. This ID is sufficient to evaluate the material properties required for the shading.

Similar to the depth pre-pass, the visibility buffer can be combined with the G-Buffer pass. However, in this case, the G-Buffer pass is a single pass that converts the visibility information into a G-Buffer. This pass can also be referred to as deferred material, as there is no need to draw the objects but only fetch data from the visibility buffer, reconstruct interpolators, and evaluate the materials.

The visibility buffer approach is becoming increasingly popular, as it opens the door to new optimizations and techniques (e.g., materials in compute shaders, software rasterization, etc.). If you're interested in this topic, check out [Visibility Buffer Rendering with Material Graphs](http://filmicworlds.com/blog/visibility-buffer-rendering-with-material-graphs/).

### Lighting

Opaque lighting is a complex topic with many existing solutions. As mentioned earlier, there are forward lighting and deferred lighting techniques. Fortunately, both use identical lighting algorithms, so we only need to focus on lighting a surface, whether it comes from a pixel rendered in forward or deferred, it doesn't matter.

We'll explore how the lighting is structured in future chapters when discussing how to approach different light types. However, to give you an overview, we like to break down the complex lighting problem into smaller parts that can be solved separately and then combined to form a final approximation.

We can categorize the lighting of opaque surfaces into four different types:

- **Direct diffuse lighting**: Illumination caused by light hitting a surface directly and scattering uniformly in all directions.
- **Direct specular lighting**: Illumination from light hitting a surface directly and reflecting in a specific direction, creating sharp highlights on smooth surfaces.
- **Indirect diffuse lighting**: Light that first bounces off other surfaces before reaching the target surface, scattering uniformly in all directions. This is often misnamed as "Global Illumination."
- **Indirect specular lighting**: Light that first bounces off other surfaces before reaching the target surface and reflecting in a specific direction, creating highlights and reflections.

Each of these categories will be addressed by a specific algorithm or system. Note that some of these systems are also used for transparent objects in later passes.

## Sky Rendering

![](Media/Recordings/The%20Render%20Pipeline%20-%20Sky.png)

> HDRI sky background with a chrome ball reflecting it.

Sky rendering is pretty straightforward and consists of a full-screen pass (a shader executed on all pixels of the screen). This shader uses the depth test to avoid overwriting opaque objects. It has the added benefit that the sky is not computed on any pixels with opaque objects already rendered, making this pass very cheap if the sky is barely visible.

We can split sky rendering into two categories:

- Physically based skies
- Artistic skies

Physically based skies are derived from an approximation of the equation of light scattering in the atmosphere. Most skies in AAA games are based on similar techniques. The physical sky is great because it behaves naturally in any lighting condition, which makes it easy to implement dynamic time-of-day, night sky rendering, cloud integration, atmosphere viewed from space, etc.

On the other hand, artistic skies rely on procedural or baked data to achieve a compelling look. They are often more static and require tweaking per location in the world to match the desired visuals. A simple example of an artistic sky is the HDRI sky, a spherical HDRI (High Dynamic Range Image) like the one above.

As you can see in this example, the "sky" doesn't only contain the rendering of the atmosphere but also a bunch of other objects. The idea is to treat them as if they were at an infinite distance from the camera, which is why the term "sky" is used when referring to them.

## Transparency and Visual Effects

This is probably the largest category in the render pipeline. It includes any effect that cannot be represented by opaque geometry and is not infinitely far from the camera. For example, this category includes:

- Transparency, translucency
- Order Independent Transparency
- Volumetric fog/clouds
- Transparent reflections
- Refraction
- Decals
- Hair rendering with a software rasterizer
- Water systems
- Etc.

In future chapters, we'll have the opportunity to discuss some of these effects in more detail. In the meantime, here are a few links you can check out if you're interested:

- [Why Transparency Is Hard](https://shaderfun.com/2020/09/20/why-transparency-is-hard/)
- [Creating the Atmospheric World of Red Dead Redemption 2: A Complete and Integrated Solution [PPTX]](https://advances.realtimerendering.com/s2019/slides_public_release.pptx)
- [Volumetric Fog in Enshrouded](https://www.youtube.com/watch?v=OR8HbFnQdlk)
- [Decals (deferred rendering)](https://mtnphil.wordpress.com/2014/05/24/decals-deferred-rendering/)
- [Interactive Wind and Vegetation in 'God of War'](https://www.youtube.com/watch?v=MKX45_riWQA)

## Post Processing

Post processing is the last step of the render pipeline and consists of applying a set of image-based effects to the main color buffer.

Post-processing is often tweaked artistically to achieve a certain visual style. It can also help enhance the visual quality of the image by reducing aliasing or adding filmic effects.

A big part of post processing comes from emulating the behavior of a real camera. Following the principles of PBR, the camera has physical properties exposed, such as exposure, aperture, ISO, sensor size, etc. These properties translate directly to post-processing effects such as [bloom](https://en.wikipedia.org/wiki/Bloom_(shader_effect)), [dynamic exposure](https://en.wikipedia.org/wiki/Exposure_(photography)), [motion blur](https://en.wikipedia.org/wiki/Motion_blur), [depth of field](https://en.wikipedia.org/wiki/Depth_of_field), [film grain (sensor noise)](https://en.wikipedia.org/wiki/Film_grain), [chromatic aberration](https://en.wikipedia.org/wiki/Chromatic_aberration), [vignetting](https://en.wikipedia.org/wiki/Vignetting), and [lens flares](https://en.wikipedia.org/wiki/Lens_flare).

Other post-processing effects serve a purely artistic purpose, like color [curves control](https://en.wikipedia.org/wiki/Curve_(tonality)), changing HUE, etc. This is something that we also find in image editing software. In the same category, we can include gameplay-based post-processing effects, such as displaying blood on the edges of the screen when the character's health is low, etc.

[Anti-aliasing](https://en.wikipedia.org/wiki/Anti-aliasing_filter) post-processing effects like TAA or SMAA aim to reduce staircase effects on the screen at the cost of some introduced blurriness. Nowadays, anti-aliasing algorithms are often combined with upsampling (super sampling or upscaling), allowing the game to run at a lower resolution while maintaining acceptable performance during gameplay (DLSS, FSR, XeSS, PSSR, Arm ASR, etc.).

Finally, tone mapping is one of the most important steps of post-processing. This process actually requires information from the screen to work, and it's often the last post-process applied. Its goal is to convert the linear HDR colors used throughout the rendering into something that the screen can display. You can think of this operation as a remapping of values, which is especially important to support HDR screens.

## Render Pipeline Design in Code

While this is not part of the render pipeline itself, I still wanted to mention the common architecture used to represent render pipelines in code.

As the complexity of rendering highly detailed scenes or photorealistic content increases, it becomes harder to organize the data layout and the code that executes the rendering. A recent development has led graphics engineers to adopt a graph representation of the frame, where each render pass is a node with clear and fixed inputs and outputs.

This graph representation is often called a **Frame Graph** or **Render Graph**, and it simplifies synchronization between resources that need to be shared between render passes. It also makes it easier to reuse resources between passes, saving a significant amount of render target memory. Additionally, analyzing the graph allows for the dynamic removal of unused passes, which simplifies the overall logic of the pipeline and helps modularize the code for specific features.

Of course, introducing a graph comes with a CPU performance cost, as more calculations are required to record the graph layout, cull nodes, etc. However, the benefits far outweigh the downsides, and several strategies exist to alleviate this performance cost, such as caching the graph layout when the feature set doesn't change.

The implementation of a render graph is a complex topic, so if you want to learn more, I recommend reading [Render Graphs](https://logins.github.io/graphics/2021/05/31/RenderGraphs.html).

> While render graphs are implemented in all major game engines, they are not necessary, especially if your renderer is very simple and you don't plan on adding a lot of features on top of it.

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
