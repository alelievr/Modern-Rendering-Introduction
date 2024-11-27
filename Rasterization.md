# Rasterization

Rasterization is probably one of the most known and used part of the GPU, this is an essential step when rendering polygonal meshes to the screen. Generally when we talk about rasterization on the GPU we mean to talk about the process of transforming arbitrary geometry into pixels on screen, while this is definition is not wrong, it lacks precision as there are underlying steps that are important to have a good idea of what rasterization is.

> Note that for the sake of clarity I'll only talk about how the rasterizer work in a Discrete GPU. If you're interested in learning how rasterization work with different architectures like TBRD check out [GPU architecture types explained
](https://www.rastergrid.com/blog/gpu-tech/2021/07/gpu-architecture-types-explained/).

Let's take a look at a diagram representing all the steps happening during rasterization of a very simple shader. Note that the order of some steps can change depending on the graphics API or GPU but the overall layout remains similar.

![](Media/Images/SimpleRasterization.png)

We'll try to explain each step of the diagram in the following sections, but before, something important to keep in mind is that the number of steps in the rasterization pipeline can change depending on the feature used inside the fragment shader. The image above describe the simplest case of a shader that only writes color data and doens't use [discard](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-discard) instructions.

For reference, here's what the pipeline could look like if we take in account depth write and discard operations:

![](Media/Images/Rasterization.png)

As you can see it's quite a bit more complex and it doesn't include all the cases, we could add MSAA, conservative depth, coverage, etc. 

## From Linear To Discrete

The process of rasterization in itself is simple, it consist in converting a series of 2D shapes into pixels. In our case these shapes represent the transformed data from the mesh shaders and the final pixels represent the output image that we want to render. You can see this process as overlaying a grid of the same size as the output image on top of the geometry and then calculating the color of the triangle only if it intersect with the center of each cell.

// TODO: image

In a sense you can see this process as turning the input geometry which has infinite resolution (linear) into a finite resolution matching the output image. This process is called [Discretization](https://en.wikipedia.org/wiki/Discretization) and introducing the infamous aliasing that you've probably seen countless times in video games or graphic apps. We'll see that in more details in the following chapter when dealing with textures.

## Primitive Assembly

This is the first step of the rasterization process. Technically it can be considered outside because it's only part of the setup before the actual rasterization starts.

The Primitive Assembler takes data from the mesh and turn it into a list of Primitives for the rasterizer to process. Most of the time these primitives are triangle but the rasterizer support other type of primitive like lines and points.

To form these primitives, the assembler reads the index buffer linking vertices together. It's possible to specify how the GPU should read this index buffer to form the triangle by specifying the type of [topologies](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-primitive-topologies), that way you can form triangle strips or fans.

## Viewport Clipping & Scissors Test

## Face Culling

This step removes any non-visible triangles, it helps improving the performance as the culled triangles will not produce any work for the fragment shader. This step can be controlled by the culling mode of the rasterizer. It has 3 different modes:

- **Cull Off / None**

    This option disables the face culling so that all triangles that are in the view will go trough the next stages of rasterization.

- **Cull Back**

    This is the most common cull mode, it cull the "back" side of all the visible triangles.    

- **Cull Front**

    Used for special effects, culls the "front" side of the visible triangles. 

To determine which side the triangle is facing, we use the winding order or winding direction.
It indicates the rotation direction in which the vertices are arranged to form the triangle. Usually this order is automatically chosen by the 3D modeling software exporting the mesh. There is a rasterizer state that you can configure to adapt to the winding order of the meshes, you can either set it to **Clockwise** or **Counter Clockwise**.

![](Media/Recordings/TriangleIntersection%2004%20Face%20Culling.gif)

## Coarse/Tile Rasterization

This step consist into rasterizing the input geometry at a low resolution to check if the triangle overlaps a tile (8x8 pixels for example). If the triangle overlaps a tile, then it can proceed to the next stages.

## Hierarchical-Z Test

When **Depth Testing** is enabled in the rasterization config, the GPU can perform an early depth test on the tiles from the coarse rasterization. This is also an optimization that allows to early out the rasterization process in case the whole tile of pixels fail the depth test.

To know the result of the depth test for the whole tile without computing the depth of every pixels in the tile, the GPU uses a conservative approximation of the depth at each corner of the tile. For this reason Hierarchical-Z Test cannot be enabled when the fragment shader uses pixel discard instructions or modifies the depth of the object (see 2nd diagram at the top).

## Fine Rasterization

For each tile that passed the tests above, we can do the fine rasterization pass which correspond to the output resolution.

## Stencil Test

Stencil Testing is another important feature of the GPU

pre-fragment work, post fragment (alpha clip)
ALso have hierarchical-stencil optimization

## Depth Test

## Depth Write

## Fill Mode

## Fragment Invocation

### Vertex Interpolation

### Pixel discard

### Quads & Helper Pixels

## Blending & Output Merger

## Conservative Rasterization

## Multisampling (MSAA)

## Variable Rate Shading

## Misc

Talk about Occlusion Queries, depth bounds, z-clip, etc.

## References

https://en.wikipedia.org/wiki/Rasterisation

https://www.rastergrid.com/blog/gpu-tech/2021/07/gpu-architecture-types-explained/

https://learn.microsoft.com/en-us/windows/win32/direct3d12/conservative-rasterization

https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_AMD_rasterization_order.html

https://fgiesen.wordpress.com/2011/07/05/a-trip-through-the-graphics-pipeline-2011-part-5/
https://fgiesen.wordpress.com/2011/07/06/a-trip-through-the-graphics-pipeline-2011-part-6/
https://fgiesen.wordpress.com/2011/07/08/a-trip-through-the-graphics-pipeline-2011-part-7/

https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-rules#triangle-rasterization-rules-without-multisampling

https://www.khronos.org/opengl/wiki/Face_Culling
