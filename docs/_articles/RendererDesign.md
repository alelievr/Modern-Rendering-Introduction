---
title: "Renderer Design"
order: 20
author: Antoine Lelievre
category: Fundamentals 
layout: post
---

The goal of a renderer is to take a 3D scene as input and output a 2D image representing a view of this 3D scene from a certain angle. Generally, renderers are integrated with other pieces of software, like an editor, to create and modify the 3D scene. They can also be part of a more complex system, like a game engine with many components running in parallel (physics simulation, audio, gameplay, rendering, etc.).

The renderer you'll learn to create in this course is what we call a *real-time renderer*, which means it needs to generate an image every frame, typically at least 60 times per second. That's a relatively short amount of time, which gets even shorter with increasing refresh rates:

| Refresh rate | Time in milliseconds |
|--------------|----------------------|
| 30           | 33.33                |
| 60           | 16.66                |
| 120          | 8.33                 |
| 144          | 6.94                 |
| 240          | 4.16                 |

To achieve these frame rates, we leverage the GPU ([Graphics Processing Unit](https://en.wikipedia.org/wiki/Graphics_processing_unit)), which contains dedicated hardware designed to accelerate 3D rendering.

## Frame Layout

Let's take a look at what happens during a single frame. Since we "only" have to render a 3D scene, things are relatively simple:

![](../assets/Recordings/RendererDesign%2001.png)

This is a very simplified view of a frame, but basically, the CPU handles updating the scene—for example, a moving camera or newly created objects. Then, the CPU prepares all the data needed to render the frame on the GPU. This could involve uploading textures to certain objects, preparing buffers to send camera information, etc. Finally, the CPU tells the GPU what to do in order to render the scene. This is typically done through a list of commands that the GPU will process. Once the GPU finishes rendering, the frame is complete.

## Double Buffering and Triple Buffering

To get a more complete picture of what happens, let's add the screen to our diagram. The GPU stores the image visible on the screen in its memory, and the screen's refresh rate controls how often this image is accessed for display. This specific memory is called the **Front Buffer**. We have to be particularly careful when writing directly to this front buffer because the screen is actively reading from it, which can cause [tearing issues](https://en.wikipedia.org/wiki/Screen_tearing) if you start writing to it before the screen finishes reading.

Fortunately, there's a simple solution to this problem: instead of writing directly to the front buffer, we write to another buffer called the **Back Buffer**, and when it's ready, we simply tell the screen to read from this buffer instead. The operation of swapping these two buffers is extremely fast compared to writing a whole image of a few megabytes, which helps avoid tearing (as long as the swap doesn't happen while the screen is reading the buffer).

After the swap, our **Back Buffer** becomes the **Front Buffer**, and vice versa. This operation is commonly called a **Flip**.

![](../assets/Recordings/RendererDesign%2000.gif)

The combination of the **Back Buffer** and **Front Buffer** is called a **swapchain**. It is an object responsible for handling these special buffers. Swapchains also have specific constraints compared to other buffers or textures you can allocate on the GPU, which we'll explore later.

Using a swapchain with 2 buffers is called **double buffering**. Logically, **triple buffering** refers to a swapchain with 3 buffers. The third buffer is essentially a second **Back Buffer**. Triple buffering is used to reduce the latency between what the application wants to draw and what is displayed on the screen.

You may have noticed in the animation above that the CPU waits for the screen to finish displaying its current frame before preparing the next one. This delay introduces latency, and triple buffering is a way to avoid it.

The idea is to decouple the screen's refresh rate from the application (CPU) by eliminating the wait and adding an intermediate buffer. With 2 **Back Buffers**, on each frame, we alternate which back buffer we write to. When the screen is ready to receive the new frame, it reads whichever **Back Buffer** is ready. This allows the application to update a **Back Buffer** immediately instead of waiting for it to become available, which helps reduce latency.

![](../assets/Recordings/RendererDesign%2002.gif)

For this course, we're actually going to use **double buffering**. Not only is it simpler to implement, but more importantly, most GPU drivers already handle **triple buffering** internally. They achieve this by releasing the screen wait earlier in the application to access these intermediate images.

If we were to implement triple buffering ourselves, we could end up with quadruple buffering instead, which would actually increase latency rather than reduce it.

## Reducing the wait on GPU

You might have noticed in the animations above that the CPU is waiting for the GPU to complete its work before starting the next frame. This is not very efficient, as the CPU could already be preparing the next frame so that the GPU can start immediately once it finishes the previous one. The key to achieving good performance in a rendering engine is to keep the GPU consistently busy with a steady workload, which can be quite challenging.

To address this, we can move the "Update Objects" part to execute before the GPU wait, since this part of the application doesn't access GPU resources.

For the second block, "Prepare GPU Data", we need to be more careful. If we move the wait after this block, we could end up writing to memory that the GPU is currently reading or hasn't read yet, which would break rendering. To fix this, we can apply the same concept as double buffering in the swapchain: any GPU resource that needs to be updated by the CPU will be doubled, and for each frame, we specify which buffer the GPU should read from.

In other words, we transform every resource uploaded by the CPU into an array of 2 resources. Then, when using these resources in the "Draw Command" block, we use either index 0 or 1 depending on the frame. This way, we can safely upload GPU resources without conflicting with the current GPU execution.

Similarly, the last block, "Draw Command", is also a kind of buffer with specific usage, so we can apply double buffering here too. This means we can move the GPU wait to the end of the following frame. If the GPU has already finished the previous frame, there is no wait.

## Frame Pacing

**Frame pacing** refers to the consistency and timing of frames rendered by an application. When the time it takes to compute a frame is inconsistent, some frames may not be updated during the display refresh (simply because there was no new image pushed to the swapchain), leading to a laggy visual experience, even if the FPS count is high.

To maintain proper frame pacing, one simple approach is to limit the frame rate to a value lower than the application's maximum FPS. This allows more time for processing slower frames. However, the best solution is to optimize your application to minimize heavy computations before rendering. Alternatively, you can offload some of the processing to a separate thread to improve performance and consistency.

These concepts are important when designing a game engine, as there are many other systems that need to run before rendering and can impact frame pacing. If you're interested in this topic, check out the excellent book [Game Engine Architecture](https://www.gameenginebook.com/index.html).

## Input latency

In real-time rendering, it's important to respond quickly to user or player inputs, such as camera movements or object interactions. **Input latency** refers to the time it takes for an application to process these inputs and display the results on screen. For instance, in shooting games, input latency is often measured by the duration between the moment you pull the trigger and when the muzzle flash first appears on your screen.

To accurately calculate input latency, it's important to consider all the steps involved, including:

- Time between your input and the game update loop
- Time for the application to process the changes based on the inputs
- Time for the application to issue draw commands to the GPU
- Time for the GPU to render the frame
- Time between the end of the GPU rendering and the next flip

There are also external factors, like the polling rate of the mouse/keyboard and the latency of the screen, but we often exclude them from calculations as they depend on the setup.

Some applications require extreme care in handling input latency, such as competitive games that must ensure consistent and low latency. That's why it's important to make the application run as fast as possible, using all the tools available, such as multi-threading and asynchronous GPU utilization.

## Frame Generation

**Frame Generation** is a technique where the GPU generates intermediate frames between two existing images to increase the framerate. This technique has become extremely popular recently due to high-refresh-rate screens becoming more affordable, as well as being more efficient than rendering an extra frame from the engine.

Frame Generation is almost independent of the renderer and hooks directly into the application's swapchain. This allows the application to continue running at a certain frequency (let's say 60 Hz), while the frame generation algorithm presents frames to the screen at a higher refresh rate (120 Hz or more, depending on the number of generated frames).

Generating new frames is not without issues. One of the biggest drawbacks is that it adds latency, as two whole frames are required to generate the one in between. At best, half of the display refresh rate is added as extra latency (often more in practice). Frame Generation algorithms also introduce new graphical artifacts such as ghosting, blurring, or interpolation issues (visible only on generated frames). These artifacts tend to be less noticeable at higher refresh rates, as the interpolated images are usually closer together.

## Conclusion

Understanding the architecture of a renderer is essential for building efficient and performant systems, particularly when aiming for real-time rendering. The interaction between the CPU and GPU must be carefully orchestrated to ensure that both components are working optimally without unnecessary idling. Techniques such as double buffering and resource management strategies help mitigate latency and maximize throughput, allowing the renderer to generate frames at high refresh rates smoothly.

As we have seen, by utilizing concepts like swapchains, double buffering, and offloading tasks across CPU and GPU, we can maintain a steady pipeline that keeps the hardware busy while avoiding issues like tearing or excessive latency. This architecture serves as the foundation upon which more advanced rendering techniques and optimizations can be built, which will be explored in the following sections of this course.

## References

- 📄 [Multiple buffering - Wikipedia](https://en.wikipedia.org/wiki/Multiple_buffering)
- 📄 [Fixing Time.deltaTime in Unity 2020.2 for Smoother Gameplay - Unity Blog](https://unity.com/blog/engine-platform/fixing-time-deltatime-in-unity-2020-2-for-smoother-gameplay)
- 🎥 [Lossless Scaling: Frame Generation For Every Game - But How Good Is it? - Digital Foundry](https://www.youtube.com/watch?v=69k7ZXLK1to)