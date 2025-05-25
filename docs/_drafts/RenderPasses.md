---
title: "Render Passes"
order: 90
author: Antoine Lelievre
category: RenderPipeline 
layout: post
---

In a render pipeline, several render passes are chained to form the final image. We can categorize these render passes into two big categories:

- Object passes
- Fullscreen pass

These two kind of passes contains several sub-gender that serve particular purpose in the pipeline, to better understand how they are used in the rendering pipeline, let's see some examples of passes.

## Object passes

The objects pass purpose is to draw geometry into one or several render targets. This geometry can come from 3D models directly loaded with the graphics API, or generated meshes from code. The point is that we draw meshes into textures, executing all the stages of the graphics pipeline (vertex transformation, rasterization, fragment shading and output of the texture(s)).

Another way to see object passes is a pass that project the geometric information of a list of 3D objects on a 2D texture, during this projection, any geometric attribute, surface attribute or lighting data can be extracted, computed and outputed to the resulting images. This give extreme flexibility in the algorithms you can implement later on with the intermediate data.

### Forward Rendering Case

A simple example of rendering objects into a color texture is forward rendering. As previously seen in the [Forward Opaque Rendering](../_articles/TheRenderPipeline.md#forward-opaque-rendering) section, this render pass relies entirely on the object's shader to do the vertex transformation and shading in a single render pass. The output in this case is the color buffer of the camera as well as often accompanied with a depth buffer.

![](../assets/Recordings/The%20Render%20Pipeline%20-%20Forward.gif)

### Deferred Rendering Case

Another case would be the part of deferred rendering where objects are rendered in the GBuffer color and depth textures.

![](../assets/Recordings/The%20Render%20Pipeline%20-%20GBuffer.gif)

### Object Transparency Rendering Case

Transparent object can also be rendered with the forward path but this time using the [Hardware Blending](https://vkguide.dev/docs/new_chapter_3/blending/) of the GPU to simulate the effect of transparency.

![](../assets/Recordings/Render%20Passes%20-%20Transparency.gif)

## Full-Screen passes

A Full-Screen pass refers to a shader being executed on the whole texture as a single pass. A full-screen pass can either be executed directly on the color buffer of the camera or on other buffers that are not directly visible. Full-Screen passes are very commonly used as intermediate passes in an algorithm.

There are two main way to do a full-screen pass, either with a [Compute Shader](../_articles/ComputeShaders.md) or with a fragment shader. Both provide different functionalilties that are best adapted for different purposes.
- Compute shader gives you access to the fast LDS memory that allows to share data between threads. It's ideal when doing blurring effects where you need to access neighbouring pixel information.
- Fragment shader has access to the ROP which adds the possibility to perform Hardware Blending, but prevents you from reading and writing to the same texture. Additionally you need to do a special mesh shader that transforms vertices to cover the screen.

### Copy or Blit Example

One of the simplest example of a full-screen pass is the copy or blit which replicates the content of a texture to another one. While there are graphics API commands that can do this operation for you without the need to write a shader ([vkCmdBlitImage](https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdBlitImage.html) or [vkCmdCopyImage](https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdCopyImage.html)) you'll certainly need to do a custom one at some point either to convert the format of the texture, or because the API doesn't support it.

The difference between a copy and a blit is that the copy will just do a memory copy of the data whereas the blit can also scale it and do some texture format conversions if necessary. The Copy command can run on an async queue on the GPU which allows to be more performant if used well.

![](../assets/Recordings/Render%20Passes%20-%20Copy.gif)

### Post Processing Example

Another very common example is with post processing, the concept is very similar to the copy pass as a simple color post process takes one input image and outputs to anoter one.

![](../assets/Recordings/Render%20Passes%20-%20Post%20Process.gif)

### Deferred Lighting Example

### Downsampling / Pyramid Example

## Render Pass in the Hardware

There is a distinction between a Render Pass used as a high-level structure to organize the graphics pipeline and the Render Pass that is described in low-level graphics APIs. Even though the content can be pretty similar.

## Conclusion

## References

https://vkguide.dev/docs/new_chapter_3/blending/
https://developer.arm.com/documentation/102479/0100/How-Render-Passes-Work
https://developer.samsung.com/galaxy-gamedev/resources/articles/renderpasses.html