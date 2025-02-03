# GPU Architecture

Modern GPUs are massively parallel and allow to run tens of thousands of threads at the same time, so it is crucial to keep in mind that we need to dispatch a large amount of tasks each time we want to execute something to avoid wasting GPU. Luckily the workload of rendering an image can be easily divided by each texel of the resulting image, for reference in a full HD image, there is a bit more than 2 million texels (1920 x 1080) so it'll be plenty of work to fill the GPU.

## Dedicated Hardware

In a GPU, there are some part of the execution that are directly mapped to physical hardware engraved in the silicium. These specific parts are designed to accelerate particular operations performed during the rendering and are often called "Fixed Hardware functions" as opposed to programmable functions we all know. On of the most widely known is the hardware to [rasterize](https://en.wikipedia.org/wiki/Rasterisation) geometry, it's task is to map geometry to a pixel grid and schedule jobs for the execution of fragment shaders.

More recently GPU added support for hardware ray-tracing, it means that there is dedicated hardware especially designed to make ray-triangle intersection and ray-traversal operations faster. This dedicated hardware need to have a certain specific memory layout to be used, so hardware specific functions often comes with a new set of types and [intrinsics](https://en.wikipedia.org/wiki/Intrinsic_function) to manipulate them.

### GPU specific code

It's expected that because the GPU have a unique architecture, the code is designed around that and take advantages of this design to improve performance. To achieve good performances, it's important to know how the hardware works. While it's true that every GPU is different, there are a lot of things in common, so optimizing an algorithm for a certain GPU will (most of the time) translate to other GPUs.

If you're interested into the inner workings of the GPU, AMD publishes a lot of documentation on the subject that present architecture, instruction sets and features in great detail. You can check it at [GPU Open Presentations](https://gpuopen.com/learn/presentations/).

I'm going to talk about the most important parts that you need to know for programming on the GPU, this is not a comprehensive list as the GPU is comprised of a lot of different parts though. Also I'm going to use the terminology of AMD GPUs, mostly because they publish more documentation and they are more "Open" than others but similar concepts exists on NVIDIA/Intel just with different names.

### Wave (AMD) or Warp (NVIDIA)

A wave is a group of threads running in parallel [SIMD](https://fr.wikipedia.org/wiki/Single_instruction_multiple_data) fashion, most of the GPUs can run either 32 or 64 threads in parallel. This is the smallest unit of execution that exists on a GPU, which means that dispatching less than 32 threads will result in unused hardware and thus reduce the performance.

Let's take a look at a simple example: rendering a cube will cause 8 vertices (one per corner) to be processed by a shader, which translate to 8 threads. If we assume that the GPU have a wave size of 64, then only 1/8th of the wave is used to process the vertices (only 12.5% of the wave is used). We could add 7 other cubes (as long as they are in the same draw call) and it would take the same amount of time to process it's vertices.

When a large number of threads is dispatched to the GPU, these threads are grouped in waves. Multiple waves can be executed in parallel as long as they are running the same shader code. For example, the current generation of AMD card can have up to 20 waves running in parallel. It's worth noting that every wave running in parallel are sharing the same resources ([ALU](https://en.wikipedia.org/wiki/Arithmetic_logic_unit), Register allocation, [Branch](https://en.wikipedia.org/wiki/Branch_(computer_science)) Processor, etc.). While each wave shares the same shader program, each of them have a separate program counter (PC) so each wave can execute any part of the shader program without needing to be in lockstep with other waves.

### Registers

Like on a CPU, registers are used to store states and data throughout the execution of a shader. The GPU however have two different types of register that have specific usages.

Scalar Registers (or SGPR), this is a register that stores a value shared across all the threads of a wave. They are very convenient because we only need to store a single value and it can be used by all threads of the wave. Usually when we read data from shader constants, this is the type of register that is used, because the GPU knows that a constant will have the same value for all threads.

Vector Registers (or VGPR), this is the most common type of register in a shader, it stores a unique value per thread. Which means that in a wave of 64 threads, there are 64 VGPRs holding potentially different values. It is the most common type of vector because it propagates during the execution of the shader as an operation with a VGPR and a SGPR can only result in a new VGPR.

### Occupancy

This is probably the most important concept to grasp when writing shaders. Occupancy defines how many waves can run in parallel when the GPU executes your shader. We say that occupancy is at 100% when the maximum of wave is running in parallel, if we take the example from the chapter above that would be 20 waves.

Occupancy is limited by 3 main factors:

- VGPR count
- SGPR count
- Local Data Share (LDS) size

To understand why the number of registers used in your shader limits how many of them can run in parallel you need to know that all the states of executions of your shader are stored in registers. This is a very big difference compared to CPU programming where most of the state is stored into the stack. In fact there is no stack at all on the GPU, so every operation that need to store intermediate data will consume a register. Of course registers are reused a lot between computation throughout the shader but there is always a part of your program that uses more registers than others. During compilation, the compiler encodes the max number of registers needed to execute the entirety of the program. This is this number that limits the occupancy, because there is a fixed number of VGPRs and SGPRs that need to be shared between many waves. We refer to this number as the "VGPR pressure". The same limits applies for the SGPRs.

Similarly, the LDS also have a limited size, when you declare more LDS in a compute shader, it'll take more space and ultimately the GPU will have to reduce the number of wave in parallel because there is not enough LDS to run them all.

Note that LDS, VGPRs and SGPRs are all allocated before the shader starts it's execution, which means that from start to finish, the maximum amount of registers and LDS is allocated. This is the case even if some part of your shader have a lower register usage than others. Apple's [M3 GPUs](https://developer.apple.com/videos/play/tech-talks/111375/) have a different approach that allows dynamic register allocation which improves the parallelism of complex shaders, but this design is still new and haven't been used on any other GPU yet. Still, it's a proof that GPU design changes and with that the rules on how to optimize, maybe sometimes in the future we'll not be talking about occupancy anymore :).

For more information, you can read this document: https://gpuopen.com/learn/occupancy-explained/

## Command Buffers

The current rendering API are command based, which means that they use a pattern where the CPU is recording all the operation that the GPU needs to execute consecutively and then this command is passed to the GPU for execution. This command list is called a Command Buffer. Due to this command buffer pattern, the execution of shaders on the GPU is mostly serial (following the command buffer order) with little overlap between the different shaders executed. The overlap between commands is limited due to the number of "Contexts" that stores information about a particular program, to give you an idea, your GPU probably between 7 and 20 contexts. So it can only run that amount of different tasks/shaders at the same time, this is where you see the big difference with the CPU that easily handles thousands of different tasks.

Command buffers can be dispatched to different "queues" on the GPU, each queue have a specification and allow to optimize the GPU usage. In this course we'll mostly use the "Graphics" or "Direct" queue as it allows to do everything.

The other queues are specialized for asynchronous computation and memory copy operations. If you're interested in learning more about these command queues, check out the [DirectX 12 documentation](https://learn.microsoft.com/en-us/windows/win32/direct3d12/executing-and-synchronizing-command-lists).

## APIs

The APIs to access the GPU are evolving fast and new features are added as new hardware and drivers are released (mesh shaders, hardware ray-tracing, work graphs, etc.). A lot of the design choices on the rendering architecture come from working with the limitations of current APIs. It's even more true when working with multiple graphics APIs which is often needed for multi-platform support, most of the time, the design that works everywhere is chosen by aligning on the lowest common denominator.

In this course we'll not talk about a particular graphics API in details but the we'll be following the conventions of DirectX12 which is the most advanced graphics API in term of feature set to this day. Similar concept also exists on Vulkan and Metal, they just often use different names.

If you want to learn low-level APIs like Vulkan or DirectX12 in details, you can follow dedicated tutorials like [Vulkan Tutorial](https://vulkan-tutorial.com/) or [Learning DirectX12](https://www.3dgep.com/learning-directx-12-1/).

One of the most limiting things on the GPU right now is that it's impossible to run a GPU program from the GPU. It's always the CPU that triggers work on the GPU, but recently a new API is being designed to allow dispatching work from the GPU: the [Work Graphs](https://devblogs.microsoft.com/directx/d3d12-work-graphs/).

It's still very new and not much implementation exists yet so I'm not going to use it in this course. What we'll try to do is to maximize the use of the GPU for every part of the renderer so when the Work Graph API becomes more widely implemented, we can transition to it easily.

## Resources

All the GPU resources that require memory to be allocated are managed from the CPU, additionally it's impossible to allocate more memory during the execution of a program on the GPU, so it's essential to have allocated every bit of memory needed for the program to run beforehand.

the GPU natively support several types of resources. The most common one is the Texture, it stores a 2D, 3D or Cube image and can be both read and written from the GPU. This type of resource also comes with a fixed function hardware called Sample that allows to perform cheap bilinear interpolation when reading memory from it. Note that the memory layout of the textures on the GPU are entirely determined by it's architecture and it is often completely hidden from the users through access functions to load and store functions to a texture.

The GPU also support several kinds of buffers, this is the same an array on the CPU, you can put any kind of structure in it. This kind of resource usually don't have any specific hardware to accelerate the operations you do with it.

We'll see in more details the kind of resources we'll use when starting to actually render some images.

## Conclusion

In summary, understanding GPU architecture is important to achieve good rendering performances. GPUs, with their massively parallel design, need to be fed with massive workloads that can be distributed across thousands of threads. This makes it essential to structure your algorithms in a way that maximize the GPU usage to avoid leaving hardware underutilized. Understanding concepts such as waves, registers, and occupancy are important to optimize your shaders.

As we’ve discussed, the GPU’s fixed-function hardware, such as rasterizers and ray-tracing units, accelerates specific tasks and shapes the way we approach GPU programming. Knowing how to leverage these capabilities, allows us to design systems that fully exploit the GPU’s strengths.

## References

- 📄 [AMD Graphics Pipeline (GIC 2020) - PDF](https://gpuopen.com/wp-content/uploads/2021/01/AMD_Graphics_pipeline_GIC2020.pdf)
- 📄 [D3D12 Work Graphs - DirectX Developer Blog](https://devblogs.microsoft.com/directx/d3d12-work-graphs/)
- 📄 [Dev Preview of New DirectX 12 Features - DirectX Developer Blog](https://devblogs.microsoft.com/directx/dev-preview-of-new-directx-12-features/)
- 📄 [Occupancy Explained - GPUOpen](https://gpuopen.com/learn/occupancy-explained/)
- 📄 [Understanding GPU Context Rolls - GPUOpen](https://gpuopen.com/learn/understanding-gpu-context-rolls/)
- 📄 [SIMD in the GPU World - RasterGrid](https://www.rastergrid.com/blog/gpu-tech/2022/02/simd-in-the-gpu-world/)
- 🎥 [How do Graphics Cards Work?  Exploring GPU Architecture - Branch Education](https://youtu.be/h9Z4oGN89MU?si=a_kCXj6zf2sw-f98)
