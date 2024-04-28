# Introduction To Modern Rendering

Welcome to the introduction to modern rendering, this course will go over everything you need to build a simple rendering engine exploiting recent hardware capabilities.

To make tings simpler, we won't talk about a particular graphics API like D3D12 or Vulkan, instead, all the examples and source code will use the [Bevy](https://bevyengine.org/) engine.

It's important to note that the rendering solutions presented in this book are not optimal in term of performances, generally heavy optimizations in rendering tends to complicate things a lot and require a vast knowledge and are not suited for an introduction course.

## Who is this course for?

This book is written for anyone interested in graphics programming or wanting to learn more about rendering. While it's not necessary to know a shader language to follow this course, it's better to feel confident in a programming language. Similarly, having some notions of linear algebra and trigonometry will help you, but it's not a hard requirement as we'll go over intuitive ways to describe the virtual 3D world with equations.

## Subjects covered

This course will cover basic mathematical knowledge related to 3D, which will be a strong foundation for both the ray tracing and rasterization renderers. Physically based rendering will play an important part of the course as it has become a standard in the video game industry.

This course will not cover graphics API (OpenGL, DirectX, etc.). It will also not cover advanced rendering techniques or optimizations, but we'll try to have several external resources if you want to know more about a particular subject.

## Hardware requirements

To run the code of this course, your graphics card need to support mesh shaders. You can use the [Vulkan Hardware database](https://vulkan.gpuinfo.org/listdevicescoverage.php?extensionname=VK_EXT_mesh_shader&extensionfeature=meshShader) to check if your GPU supports it.