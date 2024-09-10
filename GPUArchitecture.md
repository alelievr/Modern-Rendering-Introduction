# GPU Architecture

Modern GPUs are massively parallel and allow to run tens of thousands of threads at the same time, so it is crucial to keep in mind that we need to dispatch a large amount of tasks each time we want to execute something to avoid wasting GPU. Luckily the workload of rendering an image can be easily divided by each texel of the resulting image, for reference in a full HD image, there is a bit more than 2 million texels (1920 x 1080) so it'll be plenty of work to fill the GPU.

## Dedicated Hardware

In a GPU, there are some part of the execution that are directly mapped to physical hardware engraved in the silicium. These specific parts are designed to accelerate particular operations performed during the rendering and are often called "Fixed Hardware functions" as opposed to programmable functions we all know. On of the most widely known is the hardware to [rasterize](https://en.wikipedia.org/wiki/Rasterisation) geometry, it's task is to map geometry to a pixel grid and schedule jobs for the execution of fragment shaders.

More recently GPU added support for hardware ray-tracing, it means that there is dedicated hardware especially designed to make ray-triangle intersection and ray-traversal operations faster. This dedicated hardware need to have a certain specific memory layout to be used, so hardware specific functions often comes with a new set of types and [intrinsics](https://en.wikipedia.org/wiki/Intrinsic_function) to manipulate them.

## Command Buffers

The current rendering API are command based, which means that they use a pattern where the CPU is recording all the operation that the GPU needs to execute consecutively and then this command is passed to the GPU for execution. This command list is called a Command Buffer. Due to this command buffer pattern, the execution of shaders on the GPU is mostly serial (following the command buffer order) with little overlap between the different shaders executed. (note that this overlap is not necessarily wanted as running a lot of different shader programs on the GPU means that the GPU needs to switch context a lot and if there is not enough work dispatched per shader, it will result in most of the GPU doing nothing as a single SIMD block can only execute a single shader at a time. TODO: check this information).

## APIs

The API to use the GPU is still evolving fast a new features are becoming unlocked as new hardware and drivers are released, a lot of the design choices on the rendering architecture come from these limitations.

For example one of the most limiting things on the GPU right now is that it's impossible to run a GPU program from the GPU. It's always the CPU that triggers work on the GPU, but recently a new API is being designed to allow dispatching work from the GPU: the [Work Graphs](https://devblogs.microsoft.com/directx/d3d12-work-graphs/).

It's still very new and not much implementation exists yet so I'm not going to use it in this course. What we'll try to do is to maximize the use of the GPU for every part of the renderer so when the Work Graph API becomes more widely implemented, we can transition to it pretty easily.

## Resources

All the GPU resources that require memory to be allocated are managed from the CPU, additionally it's impossible to allocate more memory during the execution of a program on the GPU, so it's essential to have allocated every bit of memory needed for the program to run beforehand.

the GPU natively support several types of resources. The most common one is the Texture, it stores a 2D, 3D or Cube image and can be both read and written from the GPU. This type of resource also comes with a fixed function hardware called Sample that allows to perform cheap bilinear interpolation when reading memory from it. Note that the memory layout of the textures on the GPU are entirely determined by it's architecture and it is often completely hidden from the users through access functions to load and store functions to a texture.

The GPU also support several kinds of buffers, this is the same an array on the CPU, you can put any kind of structure in it. This kind of resource usually don't have any specific hardware to accelerate the operations you do with it.

We'll see in more details the kind of resources we'll use when starting to actually render some images.

## Conclusion

## References

https://gpuopen.com/wp-content/uploads/2021/01/AMD_Graphics_pipeline_GIC2020.pdf

https://devblogs.microsoft.com/directx/d3d12-work-graphs/

https://devblogs.microsoft.com/directx/dev-preview-of-new-directx-12-features/
