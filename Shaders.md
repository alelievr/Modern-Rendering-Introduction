# Shaders

A shader is a very lightweight program that runs on the GPU, it is designed to be spawned quickly from CPU commands such as dozens or hundreds of different shaders runs every frame. Shader are designed to work exclusively with the resources of GPU, so in term of feature set it's a lot more restricted than CPU programs (for example you cannot allocate memory on the GPU).
The API to use the GPU is still evolving fast a new features are becoming unlocked as new hardware and drivers are released, a lot of the design choices on the rendering architecture come from these limitations.

For example one of the most limiting things on the GPU right now is that it's impossible to run a GPU program from the GPU. It's always the CPU that triggers work on the GPU, but recently a new API is being designed to allow dispatching work from the GPU: the [Work Graphs](https://devblogs.microsoft.com/directx/d3d12-work-graphs/).

It's still very early and not much implementation exists yet so I'm not going to use it in this course. What we'll try to do is to maximize the use of the GPU for every part of the renderer so when the Work Graph API becomes more widely implemented, we can transition to it pretty easily.

## Different types of shaders

The GPU supports multiple kind of shaders, each of them have access to different hardware features and are integrated in the graphics pipeline of the GPU.

[![](Media/Images/MeshShaderPipeline.png)](https://devblogs.microsoft.com/directx/dev-preview-of-new-directx-12-features/)

### Compute Shaders

The compute shader is the simplest type of shader, it's multi-purpose and doesn't have access to any fixed hardware function

### Raytracing Shaders

https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html

### Vertex Shaders

### Fragment Shaders

### Tessellation Shaders

### Mesh Shaders

## GPU Architecture

Modern GPUs are massively parallel and allow to run tens of thousands of threads at the same time, so it is crucial to keep in mind that we need to dispatch a large amount of tasks each time we want to execute something. Luckily the workload of rendering an image can be easily divided by each texel of the resulting image, for reference in a full HD image, there is a bit more than 2 million texels (1920 x 1080) so it'll be plenty of work to fill the GPU.

The execution of programs on the GPU is mostly serial, which means that most of the time, only one program runs at a time. 

## How a shader gets executed


## Languages

## References

https://devblogs.microsoft.com/directx/d3d12-work-graphs/

https://devblogs.microsoft.com/directx/dev-preview-of-new-directx-12-features/

https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html
