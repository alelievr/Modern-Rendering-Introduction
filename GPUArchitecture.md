# GPU Architecture

Modern GPUs are massively parallel and allow to run tens of thousands of threads at the same time, so it is crucial to keep in mind that we need to dispatch a large amount of tasks each time we want to execute something. Luckily the workload of rendering an image can be easily divided by each texel of the resulting image, for reference in a full HD image, there is a bit more than 2 million texels (1920 x 1080) so it'll be plenty of work to fill the GPU.

## Dedicated Hardware

ROP, rasterizer, ray-tracing, etc.

## Command Buffers

The current rendering API are command based, which means that they use a pattern where the CPU is recording all the operation that the GPU needs to execute consecutively and then this command is passed to the GPU for execution. This command list is called a Command Buffer. Due to this command buffer pattern, the execution of shaders on the GPU is mostly serial (following the command buffer order) with little overlap between the different shaders executed. (note that this overlap is not necessarily wanted as running a lot of different shader programs on the GPU means that the GPU needs to switch context a lot and if there is not enough work dispatched per shader, it will result in most of the GPU doing nothing as a single SIMD block can only execute a single shader at a time. TODO: check this information).

## APIs

The API to use the GPU is still evolving fast a new features are becoming unlocked as new hardware and drivers are released, a lot of the design choices on the rendering architecture come from these limitations.

For example one of the most limiting things on the GPU right now is that it's impossible to run a GPU program from the GPU. It's always the CPU that triggers work on the GPU, but recently a new API is being designed to allow dispatching work from the GPU: the [Work Graphs](https://devblogs.microsoft.com/directx/d3d12-work-graphs/).

It's still very new and not much implementation exists yet so I'm not going to use it in this course. What we'll try to do is to maximize the use of the GPU for every part of the renderer so when the Work Graph API becomes more widely implemented, we can transition to it pretty easily.

## Resources

TODO Texture, Structured Buffers, Constant Buffers, etc.

## References

https://gpuopen.com/wp-content/uploads/2021/01/AMD_Graphics_pipeline_GIC2020.pdf

https://devblogs.microsoft.com/directx/d3d12-work-graphs/

https://devblogs.microsoft.com/directx/dev-preview-of-new-directx-12-features/
