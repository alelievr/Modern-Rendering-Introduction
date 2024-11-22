# Rasterization

Rasterization is probably one of the most known and used part of the GPU, this is an essential step when rendering polygonal meshes to the screen. Generally when we talk about rasterization on the GPU we mean to talk about the process of transforming arbitrary geometry into pixels on screen, while this is definition is not wrong, it lacks precision as there are underlying steps that are important to have a good idea of what rasterization is.

> Note that for the sake of clarity I'll only talk about how the rasterizer work in a Discrete GPU. If you're interested in learning how rasterization work with different architectures like TBRD check out [GPU architecture types explained
](https://www.rastergrid.com/blog/gpu-tech/2021/07/gpu-architecture-types-explained/).

First, let's take a look at a diagram representing all the steps happening during rasterization (and a bit more). Note that the order of some steps can change depending on the graphics API or GPU but the overall layout remains similar.

// TODO: diagram

## From Linear To Discrete

The process of rasterization in itself is simple, it consist in converting a series of 2D shapes into pixels. In our case these shapes represent the transformed data from the mesh shaders and the final pixels represent the output image that we want to render. You can see this process as overlaying a grid of the same size as the output image on top of the geometry and then calculating the color of the triangle only if it intersect with the center of each cell.

// TODO: image

## Primitive Assembler

## Face Culling

Cull Mode & Winding order

## Coarse/Tile Rasterization

## Hierarchical-Z Test

## Fine Rasterization

## Stencil Test

pre-fragment work, post fragment (alpha clip)
ALso have hierarchical-stencil optimization

## Depth Test

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

https://fgiesen.wordpress.com/2011/07/08/a-trip-through-the-graphics-pipeline-2011-part-7/

