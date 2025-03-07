---
title: "Introduction"
order: 10
author: Antoine Lelievre
category: Fundamentals 
layout: post
---

Welcome to the introduction to modern rendering! This course will cover everything you need to build a simple rendering engine while leveraging recent hardware capabilities.

To make things simpler, we won't discuss the details of a specific graphics API like D3D12 or Vulkan. Instead, all examples and source code will use the [FlyCube](https://github.com/andrejnau/FlyCube) library. This will allow us to focus on designing a good renderer without worrying too much about the low-level driver interactions of the GPU.

It's important to note that the rendering solutions presented in this course are not always optimal in terms of performance. Generally, heavy optimizations in rendering tend to complicate things significantly, requiring vast knowledge and making them unsuitable for an introductory course. This is why I have chosen to sacrifice a bit of performance (but not too much) for the sake of simplicity in the algorithms.

## Who is this course for?

This course is written for anyone interested in graphics programming or looking to learn more about rendering. While it's not necessary to know a shader language to follow this course, being proficient in a programming language is beneficial. Similarly, having some knowledge of linear algebra and trigonometry will be helpful, but it's not a strict requirement, as we will cover intuitive ways to describe the virtual 3D world using equations.

## Subjects Covered

This course will cover basic mathematical knowledge related to 3D, which will serve as a strong foundation for both ray tracing and rasterization. Physically based rendering plays an important role in the course, as it has become a standard in the video game industry.

Of course, we'll explore a wide range of rendering techniques, from path tracing to real-time lighting and post-processing. However, we'll also place a strong emphasis on materials and how they interact with light in our scenes.

As I mentioned earlier, this course will not cover graphics API specifics (OpenGL, DirectX, etc.). It will also not cover advanced rendering techniques, as they are beyond the scope of this introductory course, but we will provide several external resources if you want to explore a particular topic in more depth.

We'll also discuss GPU hardware architecture, as understanding the hardware is essential for writing efficient, high-performance code. Knowing how the machine works will help you become more effective when coding on the GPU.

## Hardware requirements

To run the code of this course, your graphics card need to support mesh shaders. You can use the [Vulkan Hardware database](https://vulkan.gpuinfo.org/listdevicescoverage.php?extensionname=VK_EXT_mesh_shader&extensionfeature=meshShader) to check if your GPU supports it.
