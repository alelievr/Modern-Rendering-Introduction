# Introduction To Modern Rendering

Welcome to the introduction to modern rendering, this course will go over everything you need to build a simple rendering engine exploiting recent hardware capabilities.

To make tings simpler, we won't talk about the details of a particular graphics API like D3D12 or Vulkan, instead, all the examples and source code will use the [LiteFX](https://litefx.crudolph.io/) library. This will allow us to focus on designing a good renderer without worrying too much onb the low-level driver interactions of the GPU.

It's important to note that the rendering solutions presented in this book are not always optimal in term of performances, generally heavy optimizations in rendering tends to complicate things a lot and require a vast knowledge and are not suited for an introduction course. Which is why I decided to sacrifice a bit of performance (but not too much) for the simplicity of the algorithms.

## Who is this course for?

This book is written for anyone interested in graphics programming or wanting to learn more about rendering. While it's not necessary to know a shader language to follow this course, it's better to be proficient in a programming language. Similarly, having some notions of linear algebra and trigonometry will help you, but it's not a hard requirement as we'll go over intuitive ways to describe the virtual 3D world with equations.

## Subjects covered

This course will cover basic mathematical knowledge related to 3D, which will be a strong foundation for both the ray tracing and rasterization. Physically based rendering plays an important part of the course as it has become a standard in the video game industry.

Of course we'll see a large range of rendering techniques from path-tracing to real-time lighting and post processes. But we'll also focus a lot on materials and how we describe their interaction with the lights in our scene.

As I said earlier, this course will not cover graphics API (OpenGL, DirectX, etc.) specifics. It will also not cover advanced rendering techniques or optimizations, but we'll have several external resources if you want to know more about a particular subject.

We'll also discuss about the hardware architecture of the GPU as it's essential to know the hardware to write good performant code and understand how the machine works and be more efficient when coding on the GPU.

## Hardware requirements

To run the code of this course, your graphics card need to support mesh shaders. You can use the [Vulkan Hardware database](https://vulkan.gpuinfo.org/listdevicescoverage.php?extensionname=VK_EXT_mesh_shader&extensionfeature=meshShader) to check if your GPU supports it.
