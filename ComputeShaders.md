# Compute Shaders

A shader is a very lightweight program that runs on the GPU, it is designed to be spawned quickly from CPU commands such as dozens or hundreds of different shaders runs every frame. Shader are designed to work exclusively with the resources of GPU, so in term of feature set it's a lot more restricted than CPU programs (for example you cannot allocate memory on the GPU).

## Different types of shaders

The GPU supports multiple kind of shaders, each of them have access to different hardware features and are integrated in the graphics pipeline of the GPU.

Several pipeline exist in the GPU, each pipeline has it's type of shaders that you can execute inside, in this chapiter, we're focusing on the simplest pipeline: the compute pipeline.

### Compute Pipeline

The compute pipeline on the GPU only allows one type of shader to be executed: the compute shader.This is the simplest type of shader, it's multi-purpose and doesn't have access to any fixed hardware function, it's very good to do general purpose computation on the GPU ([GPGPU](https://fr.wikipedia.org/wiki/General-purpose_processing_on_graphics_processing_units)). Compute shaders are dispatched directly using a number of threads, it is the equivalent of having a job system handling all the boilerplate of starting jobs and distributing work between the workers.

## Languages

GLSL / HLSL

## References

https://fr.wikipedia.org/wiki/General-purpose_processing_on_graphics_processing_units
