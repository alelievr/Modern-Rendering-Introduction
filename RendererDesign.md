# Renderer Design

The goal of a renderer is to take a 3D scene as input and output a 2D image representing a view of this 3D scene from a certain angle. Generally, renderers are integrated with other piece of software like an editor to create and modify the 3D scene. They can also be part of a more complex system like a game engine with many components running in parallel (physic simulation, audio, gameplay, rendering, etc.). 

The renderer you'll learn to create in this course is what we call a realtime renderer, which means that it needs to generate an image every frame, typically 60 times per seconds. That's a relatively short amount of time which gets smaller and smaller with increasing refresh rates:

Refresh rate | Time in milliseconds
--- | ---
30 | 33.33
60 | 16.66
120 | 8.33
144 | 6.94
240 | 4.16

To achieve these framerates, we leverage the ([Graphics Processing Unit](https://en.wikipedia.org/wiki/Graphics_processing_unit)) which contains dedicated hardware intended to make the task of 3D rendering faster.

## Frame Layout

Let's take a look at what happens during a single frame, because we "only" have the rendering of a 3D scene to do, things are rather simple:

![](Media/Recordings/RendererDesign%2001.png)

This is a very simplified view of a frame but basically we have the CPU handling the update of the scene, for example this could be a camera that moves, objects that gets created, etc. then the CPU prepares all the data needed to render the frame on the GPU, this could be uploading textures to certain objects, preparing buffers to send camera information, etc. And finally the CPU tells what the GPU needs to do in order to render the scene, this is typically executed through a list of commands that the GPU will process. Finally we just have to wait for the GPU to finish rendering and the frame is done.

## Double Buffering and Triple Buffering

To have a more complete picture of what happens, let's add the screen in our diagram. The GPU stores the image visible on screen in it's memory and the screen refresh rate controls how often this image is accessed to be displayed. This particular memory is called a **Front Buffer**, we have to be particularly careful when writing directly to this front buffer because the screen is actively reading it, which can cause [tearing issues](https://en.wikipedia.org/wiki/Screen_tearing) if you start writing to this buffer before the screen finishes to read it. Fortunately for us, there is a very simple solution to this problem: instead of writing directly to this buffer, we write to another one which we call the **Back Buffer** and when it's ready we just tell the screen to read from this buffer instead of the front one. The operation of swapping those two buffers is extremely fast compared to writing a whole image of a few Megabytes which allows to avoid tearing (provided that you don't swap it when the screen is in the middle of reading the buffer). After the swap, our **Back Buffer** becomes the **Front Buffer** and vice versa.

![](Media/Recordings/RendererDesign%2000.gif)

The combination of **Back Buffer** and **Front Buffer** is called a swapchain, it is an object that is responsible of handling these special buffers. They also hold specific constraints compared to other buffers / textures you can allocate on the GPU that we'll see in the future.

Using a swapchain with 2 buffers, is double buffer buffer, so logically, triple buffering is a swapchain with 3 buffers. The third buffer is actually a 2nd **Back Buffer**. Triple buffering is used to reduce the latency between what the application want to draw and what is on the screen. You might have noticed in the animation above that the CPU is waiting for the screen to finish displaying it's current frame before preparing the next one. This delay introduces latency and triple buffering is a way to avoid that.

The idea is to cut the dependency between the screen refresh rate and the application (CPU) by removing the wait for the screen and adding an intermediate buffer. With 2 **Back Buffers**, at each frame, we swap which back buffer we write to and when the screen is ready to recieve the new frame, it just reads whichever **Back Buffer** is ready. That way, the application can safely update the **Back Buffer** instead of just waiting for it to get old which reduces the latency.

![](Media/Recordings/RendererDesign%2002.gif)

For this course, we're actually going to use double buffering. Not only it is simpler to implement but more importantly, most of the GPU drivers are already handling triple-buffering. They make it work by releasing the screen wait earlier in the application to get these intermediate images. If we were to implement triple-buffering in our application we could end up with quadruple buffering instead and we'd actually gain latency instead of reducing it.

## Reducing the wait on GPU

You might have noticed in the animations above that the CPU is waiting for the GPU to complete it's work before starting the next frame. This is not very efficient as our CPU could already be preparing the next frame, so that when the GPU finishes the first, it can start right away. The key to achieve good performance in a rendering engine is to keep the GPU busy with a good workload at all time, which can be pretty challenging.

To achieve that, we can already move the "Update Objects" part to be executed before the GPU wait because this part of the application doesn't access GPU resources.

For the second block "Prepare GPU Data", we need to be more careful, if we move the wait after this block, we could end-up writing to memory that the GPU is reading or haven't read yet which would break the rendering. To fix this, we can borrow the same concept as the double buffering of the swapchain: any GPU resources that we need to update from the CPU will be doubled and we'll specify at each frame which buffer the GPU reads.

In other words, the idea is to transform every resources uploaded by the CPU into an array of 2 resources, then when we use these resources in the "Draw Command" block, we use either 0 or 1 as index depending on the frame. That way, we can upload any GPU resources without it having to conflict with the current execution.

Similarly, the last block "Draw Command" is also a kind of buffer with specific usage, so we can apply double buffering here too, meaning that we can move the GPU wait to the end of the following frame, if the GPU already has finished the previous frame, then there is no wait.

## Conclusion

Understanding the architecture of a renderer is essential for building efficient and performant systems, particularly when aiming for real-time rendering. The interaction between the CPU and GPU must be carefully orchestrated to ensure that both components are working optimally without unnecessary idling. Techniques such as double buffering and resource management strategies help mitigate latency and maximize throughput, allowing the renderer to generate frames at high refresh rates smoothly.

As we have seen, by utilizing concepts like swapchains, double buffering, and offloading tasks across CPU and GPU, we can maintain a steady pipeline that keeps the hardware busy while avoiding issues like tearing or excessive latency. This architecture serves as the foundation upon which more advanced rendering techniques and optimizations can be built, which will be explored in the following sections of this course.
