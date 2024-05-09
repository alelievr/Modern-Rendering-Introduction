# Compute Shaders

A shader is a very lightweight program that runs on the GPU, it is designed to be spawned quickly from CPU commands such as dozens or hundreds of different shaders runs every frame. Shader are designed to work exclusively with the resources of GPU, so in term of feature set it's a lot more restricted than CPU programs (for example you cannot allocate memory on the GPU).
The API to use the GPU is still evolving fast a new features are becoming unlocked as new hardware and drivers are released, a lot of the design choices on the rendering architecture come from these limitations.

For example one of the most limiting things on the GPU right now is that it's impossible to run a GPU program from the GPU. It's always the CPU that triggers work on the GPU, but recently a new API is being designed to allow dispatching work from the GPU: the [Work Graphs](https://devblogs.microsoft.com/directx/d3d12-work-graphs/).

It's still very new and not much implementation exists yet so I'm not going to use it in this course. What we'll try to do is to maximize the use of the GPU for every part of the renderer so when the Work Graph API becomes more widely implemented, we can transition to it pretty easily.

## Different types of shaders

The GPU supports multiple kind of shaders, each of them have access to different hardware features and are integrated in the graphics pipeline of the GPU.

Several pipeline exist in the GPU, each pipeline has it's type of shaders that you can execute inside.

### The Graphics Pipeline

Here you can see in this diagram form the DirectX 12 documentation, the chain of shader types executed by the GPU when an object is rendered. The blue nodes represent fixed hardware functions, fixed hardware mean that it physically exists on the GPU silicium, hence it can't be programmed, that's why we call it fixed hardware functions, usually these fixed hardware functions act as the glue between the different stages of the shaders by passing and converting data from one stage to another.
[![](Media/Images/MeshShaderPipeline.png)](https://devblogs.microsoft.com/directx/dev-preview-of-new-directx-12-features/)

In this guide, we'll focus on the Mesh Shader Pipeline which is the new preferred way of rendering objects in new graphics APIs. You'll see that it's simpler to understand because it has less stages and fixed hardware functions. If you want to learn more about the legacy graphics pipeline stages, you can read more in the [DirectX 11 documentation](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-graphics-pipeline).

#### Amplification Shaders

The amplification shader is an optional stage in the mesh shader pipeline that can take any data as input and outputs a list of vertices and primitives for the next stage (Mesh Shader). A vertex stores all the data needed to describe a mesh, usually each vertex has it's position, UVs, normal, etc. but the encoding of such data is fully controllable so you could pack data in any particular way you want as long as you have the code to interpret it correctly in the mesh shader.

// TODO: find another way to say that, too technical too early
This stage is called Amplification because it can generate more geometry as output than it has as input. Such use case is very handy as it serves as a replacement for the tessellation and facilitate LOD technique implementation.

If this stage is not present, the mesh shader will be called for each vertex

#### Mesh Shaders

#### Rasterization

#### Fragment Shaders

### Compute Pipeline

The compute pipeline on the GPU only allows one type of shader to be executed: the compute shader.This is the simplest type of shader, it's multi-purpose and doesn't have access to any fixed hardware function, it's very good to do general purpose computation on the GPU ([GPGPU](https://fr.wikipedia.org/wiki/General-purpose_processing_on_graphics_processing_units)). Compute shaders are dispatched directly using a number of threads, it is the equivalent of having a job system handling all the boilerplate of starting jobs and distributing work between the workers.

## Languages

## References

https://devblogs.microsoft.com/directx/d3d12-work-graphs/

https://devblogs.microsoft.com/directx/dev-preview-of-new-directx-12-features/
