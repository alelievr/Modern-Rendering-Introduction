---
title: "Ray Tracing"
order: 30
author: Antoine Lelievre
category: RenderPipeline 
layout: post
---

Ray tracing is a technique that involves computing the intersection between a ray and a geometric primitive, such as a triangle or a sphere. This method is widely used in various rendering algorithms and provides a straightforward way to simulate how light interacts with objects in a scene.

In fact ray tracing is often the first step when learning to make 3D renders because you can easily achieve compelling results with a few lines of code. This simplicity comes at with a very high performance cost, which makes it hard to use in real-time applications.

A ray is defined by two vectors: a **position**, which represents its starting point, and a **direction**, which determines the path it follows. These rays can be used to trace visibility, reflections, shadows, and other effects by intersecting with objects in the scene.

When a ray intersects with a surface, we can extract useful information from the hit point. This includes the **3D model** that was hit, the **specific triangle** within the model’s mesh that was intersected, the normal of the surface, it's color, etc.

## Path Tracing

[Path tracing](https://en.wikipedia.org/wiki/Path_tracing) is the name of a [Light Transport](https://en.wikipedia.org/wiki/Light_transport_theory) algorithm. It uses the principle of ray tracing to simulate how light rays interact with a 3D scene.

The idea behind the algorithm is simple: we start by shooting rays from the camera or the light sources. These rays will intersect the objects in the scene. At each intersection, we evaluate how light interacts with the surface, and we accumulate the lighting information to simulate light bouncing between objects.

There are several types of path tracers. For this course, we're particularly interested in "backward" path tracing. It's called backward because the rays start from the camera and then bounce off the objects in the scene before hitting a light. This is the reverse of what happens in reality.

### PBRT 4th edition

In this course the reference renderer we're making will mostly follow the guidelines of [Physically Based Rendering V4](https://pbr-book.org/4ed/contents) with some simplifications. We're also doing it fully on the GPU from the start whereas the book only talk about GPU implantation at the end. This book is an amazing resource on path tracing and design, I heavily recommend reading it, or at least the chapters that you're interested in.

### Approximations

Here, we are already making an approximation by assuming that light can be represented as a "photon packet" traveling in a straight line through space and interacting with any surface it encounters. We already know that this is physically inaccurate because light exhibits both particle and wave behavior.  

With ray tracing, it is impossible to represent interference patterns or diffraction grating, but we assume this is acceptable for our use case. (If you're interested in this topic, you can take a look at [A Generalized Ray Formulation for Wave-Optics Rendering](https://ssteinberg.xyz/2023rtplt/2023_rtplt.pdf)).

Some path tracers attempt to achieve a closer match to reality by tracing multiple rays per photon packet, with each ray representing a single wavelength of light. Typically, at least three rays are used for the wavelengths corresponding to red, green, and blue. This technique is called **spectral path tracing**. It allows for accurate modeling of phenomena such as diffraction in prisms, iridescence, and fluorescence.

However, for our course, we will assume that these effects are not required. As you can imagine, spectral path tracing is computationally expensive, as it requires adjusting not only all lighting equations to work with wavelengths but also all material descriptions.

## Implementation

Let's start with the minimal compute shader kernel to compute an image:

```c
RWTexture2D<float4> _Output;

[numthreads(8, 8, 1)]
void main(uint3 id : SV_GroupThreadID)
{
    uint2 positionSS = id.xy;
    float4 color = 0;

    _Output[positionSS] = color;
}
```

In this code, we have the screen space pixel position denoted by `positionSS`. To determine the ray direction from this position, I'll jump ahead a bit because we need some transformations to compute the ray direction. You can take a look at the transformation pipeline described in [Matrices and Transformations](MatricesAndTransformations.md) for more information.

### Calculating the ray direction from the camera

The transformation pipeline tells us exactly what we need to do to move from the **Screen Space** we have right now to the world space where our rays can intersect with other objects of the scene.

So, starting from screen space, we can move to NDC by remapping the position between [-1, +1] and set z to 1, because we want to cast our rays toward the +Z direction. Additionally we can move directly to the HCLIP space if we set w to 1 as well.

```c
uint2 positionSS = id.xy;
float2 outputSize;
_Output.GetDimensions(outputSize.x, outputSize.y); // Get the dimension of the output to remap the screen position.
float3 positionNDC = float3((positionSS / outputSize) * 2 - 1, 1);
float4 positionHCLIP = float4(positionNDC, 1);
```

From the position in homogeneous clip space, we multiply by the inverse view projection matrix to move to world space, and after that normalizing the vector to ensure it's length is 1 which necessary for a direction.

```c
    float4 world = mul(float4(positionNDC, 1), inverseViewProjectionMatrix);
    float3 directionWS normalize(world.xyz);
```

We can put these sequence transformations in dedicated functions with to help us move from one space tro another when necessary, you can take a look at those functions in [Common.hlsl](https://github.com/alelievr/Modern-Rendering-Introduction/blob/master/ModernRenderer/assets/shaders/Common.hlsl).

### Objects intersections

Now that we have the ray direction and it's origin (it's the camera position), we can calculate intersections with other objects in the scene. Using the intersections formulas we saw in previous chapters, we can test our ray tracer by hardcoding a few objects in hlsl. Each object is assigned a different color and we select the closest hit distance in case of multiple intersections. If nothing is intersected, we just return black color.

![](../assets/Recordings/Hardcoded%20Scene%2000.png)

The result might not be impressive right now but there is no point in doing crazy stuff with primitives as our goal will be to render polygonal meshes with the path tracer in the next chapters.

You can check out the intersection functions in the [GeometryUtils.hlsl](https://github.com/alelievr/Modern-Rendering-Introduction/blob/master/ModernRenderer/assets/shaders/GeometryUtils.hlsl) source file.

## Conclusion

Ray tracing is a key technique for simulating light interactions in 3D rendering. Although it’s relatively simple to implement, its high computational cost makes it difficult to use in real-time applications. Path tracing builds on ray tracing by simulating light transport more accurately, but it comes with added complexity as well as performance challenges.

In this course, we've covered the basics of ray tracing and path tracing, including the necessary transformations for calculating ray directions and basic intersection methods for geometric primitives. While the current implementation is simple, it sets the foundation for more complex scenes using polygonal meshes in the future.

## References

- 📄 [Path tracing - Wikipedia](https://en.wikipedia.org/wiki/Path_tracing)
- 📄 [Physically Based Rendering: From Theory to Implementation (4th Edition)](https://pbr-book.org/4ed/contents)
- 📄 [2023 Path Tracing Paper - S. Steinberg (PDF)](https://ssteinberg.xyz/2023rtplt/2023_rtplt.pdf)
- 🎥 [How does Ray Tracing Work in Video Games and Movies? - Branch Education](https://www.youtube.com/watch?v=iOlehM5kNSk)
