---
title: "Render Passes"
order: 90
author: Antoine Lelievre
category: RenderPipeline 
layout: post
---

In a render pipeline, several render passes (sometimes simply called "passes") are chained together to produce the final image. These render passes can be broadly categorized into two main groups:

- Object passes  
- Fullscreen passes

Each of these types contains several subcategories that serve specific purposes within the pipeline. To better understand their roles, let's look at some examples.

## Object passes

The purpose of an object pass is to render geometry into one or more render targets. This geometry can come from 3D models loaded via the graphics API, from procedurally generated meshes, or other geometric representation of the scene. In essence, object passes draw meshes into textures, executing the full sequence of the graphics pipeline: vertex transformation, rasterization, fragment shading, and writing to output textures.

Another way to think about object passes is as projections of 3D scene data onto 2D textures. During this projection, any geometric attributes, surface properties, or lighting information can be extracted, computed, and written to the resulting images. This provides a high degree of flexibility, enabling the implementation of complex algorithms that operate on this intermediate data.

### Forward Rendering Example

A simple example of rendering objects into a color texture is forward rendering. As previously discussed in the [Forward Opaque Rendering](../_articles/TheRenderPipeline.md#forward-opaque-rendering) section, this render pass relies entirely on the object's shader to perform both vertex transformation and shading in a single step. The output in this case is the object's final color into the camera color buffer, often accompanied by a depth buffer.

![](../assets/Recordings/The%20Render%20Pipeline%20-%20Forward.gif)

### Deferred Rendering Example

Another example is the geometry phase of deferred rendering, where objects are rendered into the GBuffer's color and depth textures. In this setup, different surface attributes like albedo, normals, roughness, etc. are encoded into multiple render targets. These textures are then used in a later lighting pass.

![](../assets/Recordings/The%20Render%20Pipeline%20-%20GBuffer.gif)

### Object Transparency Rendering Example

Transparent objects can also be rendered using the forward path, but this time leveraging [hardware blending](https://vkguide.dev/docs/new_chapter_3/blending/) on the GPU to simulate the effect of transparency. In this case, blending equations combine the fragment’s color with what’s already in the render target, based on alpha values.

![](../assets/Recordings/Render%20Passes%20-%20Transparency.gif)

## Full-Screen Passes

A full-screen pass refers to a shader that processes an entire texture in a single pass. It can be executed directly on the camera’s color buffer or on intermediate buffers not directly visible to the user. Full-screen passes are commonly used as intermediate steps in more complex rendering algorithms.

There are two main ways to perform a full-screen pass: using either a [Compute Shader](../_articles/ComputeShaders.md) or a fragment shader. Each approach offers different functionalities suited for different use cases:

- **Compute shaders** provide access to fast local data share (LDS) memory, allowing efficient data sharing between threads. They are ideal for operations like blurring, where access to neighboring pixels is necessary.
- **Fragment shaders** have access to the ROP (Raster Operations Pipeline), enabling hardware blending. However, they do not allow simultaneous read and write on the same texture. Implementing a full-screen fragment shader also requires a special mesh (typically a screen-space triangle or quad) that covers the entire screen.

### Copy or Blit Example

One of the simplest full-screen passes is a copy or [blit](https://en.wikipedia.org/wiki/Bit_blit) operation, which replicates the content of one texture into another. While many graphics APIs offer built-in commands for this (like [vkCmdBlitImage](https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdBlitImage.html) or [vkCmdCopyImage](https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdCopyImage.html)), there are cases where a custom shader-based implementation is necessary—for instance, to convert texture formats or to support scenarios not handled by the graphics APIs.

The main difference between a copy and a blit:
- **Copy**: Performs a direct memory copy of the texture data.
- **Blit**: Can scale and convert formats during the copy.

> Note: Copy operations can also run on an asynchronous queue, which may lead to performance benefits if managed properly.

![](../assets/Recordings/Render%20Passes%20-%20Copy.gif)

### Post-Processing Example

A common use case for full-screen passes is post-processing. The concept is similar to a copy pass, but the input image is modified (e.g., color grading, bloom, vignette, tone mapping) before being written to an output buffer. More advanced post-processing passes may take multiple inputs such as the color buffer, depth buffer, motion vectors, etc. and can use several full-screen draws chained together in a single pass.

![](../assets/Recordings/Render%20Passes%20-%20Post%20Process.gif)

### MIP Chain Generation Example

A MIP chain or [MIP map](https://fr.wikipedia.org/wiki/MIP_mapping) is a set of texture levels, each level is half the resolution of the previous one. While many graphics APIs can automatically generate mipmaps, some (like DirectX 12) do not, requiring manual implementation.

To generate a MIP chain, multiple full-screen passes are executed, each taking the previous MIP level as input. The [downsampling](https://en.wikipedia.org/wiki/Downsampling_(signal_processing)) typically relies on the hardware sampler using bilinear interpolation, effectively averaging a 2×2 pixel region to produce the next MIP level. For higher quality, more sophisticated filters like Gaussian blur can be used.

The choice of algorithm depends on how the mipmaps will be used—whether for LOD, bloom, texture filtering, or custom effects.

In the animation below, the MIP levels of a texture are visualized using a point sampler (no interpolation) to clearly show the distinct levels.

![](../assets/Recordings/Render%20Passes%20-%20Mip%20Generation.gif)

> Note, sometimes the MIP chain is also called a pyramid due to it's shape if you overlap all the MIP levels. Depending on the content of the texture, it can be called a color pyramid or a depth pyramid.

[![](https://elementalray.wordpress.com/wp-content/uploads/2012/04/pyramid_image.jpg)](https://elementalray.wordpress.com/2012/04/17/texture-publishing-to-mental-ray/)
> image from [Texture Publishing to mental ray](https://elementalray.wordpress.com/2012/04/17/texture-publishing-to-mental-ray/).

## Render Pass in the Hardware

There is a distinction between a render pass as a high-level abstraction used to organize the rendering pipeline, and a render pass as defined by low-level graphics APIs. While their purposes may align conceptually, low-level render passes are typically more constrained to allow the driver to optimize execution based on hardware specifics.

For instance, low-level render pass APIs enable the use of tile memory on [tile-based GPUs](https://en.wikipedia.org/wiki/Tiled_rendering), which can significantly reduce bandwidth usage by keeping intermediate data in on-chip memory. However, this comes with certain limitations—such as restricted access patterns or fixed render target layouts—to ensure compatibility with the tile-based architecture.

In contrast, high-level render passes, such as those used in a [Frame Graph or Render Graph](https://logins.github.io/graphics/2021/05/31/RenderGraphs.html), serve as flexible scheduling and dependency-tracking structures. These passes are not tied to specific hardware constraints and can mix different types of work (graphics, compute, ray tracing, etc.). They also support dynamic reconfiguration of render targets between draws, which is typically not allowed within a single low-level API render pass.

In summary:
- **High-level render passes** are conceptual units used for scheduling and resource management. They are flexible and unrestricted.
- **Low-level render passes** are concrete constructs defined by the graphics API. They are optimized for hardware but come with operational constraints.

> Note that these two concepts are not incompatible as they are often mixed together to allow high-level algorithm design to benefit from the low-level render pass API optimizations.

## Conclusion

Render passes break the rendering process into stages. Object passes render 3D geometry into intermediate textures, handling tasks like shading, GBuffer generation, and transparency. Full-screen passes operate on textures, applying effects such as post-processing, downsampling, compositing or lighting.

At a higher level, render passes are scheduling units that can mix graphics and compute tasks with flexible resource usage. At the API level, they follow stricter rules to optimize for hardware, especially on tile-based GPUs.

Understanding both levels—and when to use each—helps balance performance with control in a rendering pipeline.

## References

- 📄 [Hardware Blending - Vulkan Guide](https://vkguide.dev/docs/new_chapter_3/blending/)
- 📄 [How Render Passes Work - Arm Developer](https://developer.arm.com/documentation/102479/0100/How-Render-Passes-Work)
- 📄 [Understanding Render Passes - Samsung Developer](https://developer.samsung.com/galaxy-gamedev/resources/articles/renderpasses.html)
- 📄 [Dynamic Rendering Sample - Vulkan Docs](https://docs.vulkan.org/samples/latest/samples/extensions/dynamic_rendering/README.html)
- 📄 [Tiled Rendering - Wikipedia](https://en.wikipedia.org/wiki/Tiled_rendering)
- 📄 [Render Graphs - Logins.github.io](https://logins.github.io/graphics/2021/05/31/RenderGraphs.html)
- 📄 [MIP Mapping - Wikipedia (FR)](https://fr.wikipedia.org/wiki/MIP_mapping)
- 📄 [Downsampling (Signal Processing) - Wikipedia](https://en.wikipedia.org/wiki/Downsampling_(signal_processing))
