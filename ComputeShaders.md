# Compute Shaders

A shader is a very lightweight program that runs on the GPU, it is designed to be spawned quickly from CPU commands such as dozens or hundreds of different shaders runs every frame. Shader are designed to work exclusively with the resources of GPU, so in term of feature set it's a lot more restricted than CPU programs (for example you cannot allocate memory on the GPU, you cannot access files on the disk, etc.).

## Different types of shaders

The GPU supports multiple kind of shaders, each of them have access to different hardware features and are integrated the different pipelines. In this chapiter, we're interested in the compute pipeline, it is the simplest of all pipelines because it has only a single step.

### Compute Pipeline

The compute pipeline on the GPU only allows one type of shader to be executed: the compute shader.This is the simplest type of shader, it's multi-purpose and doesn't have access to any fixed hardware function, it's very good to do general purpose computation on the GPU ([GPGPU](https://fr.wikipedia.org/wiki/General-purpose_processing_on_graphics_processing_units)). Compute shaders are dispatched directly using a number of threads, it is the equivalent of having a job system handling all the boilerplate of starting jobs and distributing work between the workers.

[![](Media/Images/compute-pipeline.png)](https://learn.microsoft.com/en-us/windows/win32/direct3d12/pipelines-and-shaders-with-directx-12)

## Languages

There are several languages that you can use to write GPU programs, the most popular are GLSL and HLSL. Both languages have very good support for a lot of features and extensive tooling for cross-platform compilation, HLSL has the upper hand regarding to language specifications as it continues to evolve quickly driven by Microsoft for DirectX advances. For this course, we'll use the GLSL language as it's the one natively supported in [Bevy](https://bevyengine.org/). TODO: check if can use HLSL instead.

## Syntax

The GLSL syntax is very similar to C with the the twist that there are no pointers, function overload is possible as well as a few other extra functionality that makes common operations easier.

This simple example describes a function that clears an 4 channel image to 0. The entry point of the compute shader (main in this case) is called a [kernel](https://en.wikipedia.org/wiki/Compute_kernel).

The kernel have access to all the resources that were bound by the CPU to this compute shader as well as some automatically generated variables. like `gl_GlobalInvocationID` that indicates the "position" of the current kernel

```glsl
gimage2D output;

// Where is group size?
void main()
{
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    imageStore(output, pos, vec4(0.0, 0.0, 0.0, 0.0));
}
```

// TODO: explanation of group and threads and LDS

To learn more about the syntax of compute shader in GLSL, you can read the official documentation: https://www.khronos.org/opengl/wiki/Compute_Shader.

## References

https://fr.wikipedia.org/wiki/General-purpose_processing_on_graphics_processing_units
