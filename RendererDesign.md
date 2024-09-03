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

## Double Buffering vs Triple Buffering

You might have noticed in the image above that the CPU is waiting for the GPU to complete it's work before starting the next frame. This is not very efficient as our CPU could already be preparing the next frame, so that when the GPU finishes the first, it can start right away. The key to achieve good performance in a rendering engine is to keep the GPU busy with a good workload at all time, which can be pretty challenging.

## Latency Management
