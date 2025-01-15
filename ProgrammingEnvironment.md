# Programming Environment

In this chapiter we'll spend a bit of time exploring the offering of IDE, debuggers and profilers available to you when making a 3D application. Unlike programming languages on the CPU that have a very broad offering of products to develop in lots of different languages, you'll see that it's a different story on the GPU.

## Shader Languages

### HLSL

### GLSL

### Slang

TODO: https://github.com/shader-slang/slang

## HLSL Editors

IDE/Code editors that support well HLSL are pretty scarce, while most of the common IDE have HLSL or shader extension, they often lack good autocomplete functionalities, linting or symbol parsing.

Editor | Extension
--- | ---
Visual Studio | [HLSL Tools for Visual Studio](https://marketplace.visualstudio.com/items?itemName=TimGJones.HLSLToolsforVisualStudio)
Visual Studio Code | [HLSL Tools](https://marketplace.visualstudio.com/items?itemName=TimGJones.hlsltools)
Rider | [Built-in](https://www.jetbrains.com/help/rider/Unreal_Engine__HLSL_Shaders.html)

## CPU side: C++ / Rust, gfx library

When constructing the back end of a renderer, a variety of high-quality libraries are available to support your application development. The choice of library often hinges on the graphics API abstraction level (such as OpenGL, DirectX, Vulkan) and the specific features you require. Each library offers different degrees of cross-platform compatibility, and many support multiple languages, including Rust, C++, C# and more through bindings.

Selecting the right library will depend on both the feature set you aim to implement and your comfort with graphics programming. Higher-level libraries generally provide greater ease of use by abstracting a part of the complexity, while low-level APIs, like DirectX12 or Vulkan, grant full control at the cost of increased complexity and a steeper learning curve.

For those new to graphics programming, I recommend beginning with a modern, high-level graphics library. This approach allows you to focus on foundational concepts without being overwhelmed by the demanding specifics of low-level graphics API management.

Here's a list of the most famous ones: https://github.com/jslee02/awesome-graphics-libraries.

## Debugging & Profiling

Most of the IDE doesn't support debugging GPU code, this has to do with the fact that the GPU is a more closed and restricted programming environment. It is also impossible to put the execution of a shader on pause for debugging like breakpoints do for CPU code. Because of this external debuggers are used to debug and profile the GPU code, they provide different functionality to inspect the workload that has been submitted to the GPU during a frame and provide replay tools to analyse in detail what happened.

> Note that most of the GPU debugging tools exist only on Windows, this is the platform that offers the most support for investigating real-time workloads and debugging.

One of the best debugging tool is [RenderDoc](https://renderdoc.org/) it is easy to integrate in your application or attach to any existing ones and has all the features needed to debug a frame as well as a good shader debugger.

[PIX](https://devblogs.microsoft.com/pix/) is another great debugging tool, this time only for windows. It has the particularity of being up to date with the latest DirectX features, so if you're trying out new features it's your choice of tool. PIX is also able to profile and analyze the performances of your application.

[GPA](https://www.intel.com/content/www/us/en/developer/tools/graphics-performance-analyzers/overview.html) TODO

[NVIDIA NSight Graphics](https://developer.nvidia.com/nsight-graphics) TODO

[Xcode Metal Debugger](https://developer.apple.com/documentation/xcode/metal-debugger) TODO

Another great tool to have is validation layers, they are additional checks that can be enabled on low-levels APIs that catches errors before they are sent to the GPU, they are very useful to detect issues in setup and order in the GPU commands sent to the GPU
