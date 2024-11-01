# Ray Tracing

Ray tracing is is a technique that consist in computing the intersection between a ray and another primitive. This technique is used in a lot of different algorithm in rendering and provide a simple and intuitive approach to rendering.

A ray is composed of two vectors: one position and one direction.

Each intersection with a ray allows to retrieve information such as the hit position, which 3D models was hit by the ray, which triangle was hit inside the mesh. From this intersection, we can then get the mesh surface data and interpolate it.

## Path Tracing

[Path tracing](https://en.wikipedia.org/wiki/Path_tracing) is the name of a [Light Transport](https://en.wikipedia.org/wiki/Light_transport_theory) algorithm, it uses the principle of ray-tracing to simulate how light rays interacts with a 3D scene.

The idea of the algorithm is simple: we're going to start shooting rays from the camera or the light sources, these rays will intersect the objects in the scene, at each intersection we'll evaluate how light interacts with the surface and we'll accumulate the lighting information to simulate light bouncing between objects.

There are several types of path tracer that exist, for this course, we're particularly interested in "backward" path tracing. It's called backward because the rays starts from the camera and then bounces of the objects of the scene before hitting a light, this is the reverse of what happens in reality.

## PBRT 4th edition

In this course the reference renderer we're making will mostly follow the guidelines of [Physically Based Rendering V4](https://pbr-book.org/4ed/contents) with some simplifications. We're also doing it fully on the GPU from the start whereas the book only talk about GPU implantation at the end. This book is an amazing resource on path tracing and design, I heavily recommend reading it, or at least the chapters that you're interested in.

## Approximations

Here we are already starting to approximate by assuming that the light can be represented by a photo going straight in space and interacting with any surface that it comes in contact. We already know that this is physically wrong because we know from observation that light behaves both as a particle and a wave. So with ray-tracing it's impossible to represent interference patterns or diffraction grating which we assume is okay for our use case (If you're interested in this, you can take a look at [A Generalized Ray Formulation For Wave-Optics Rendering](https://ssteinberg.xyz/2023rtplt/2023_rtplt.pdf)).

Some path tracers are trying to do a closer match to reality by tracing multiple rays per photon, each ray representing a single wavelength of light (usually they do at least 3 ray for the wavelength corresponding to red, green and blue), this is called spectral path tracing. It allows to accurately model diffraction in prisms, iridescence, fluorescence, etc. We're also going to assume that these are not needed for our course because as you can imagine this is pretty expensive to compute and we'd need to adjust not only all the lighting equations to work with wavelength but also all our material descriptions.

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

In this code we have the screen space pixel position denoted by `positionSS`. To determine the ray direction from this position, I'm going to jump ahead a bit here because we need some transformations to be able to know the exact direction the rays need to go. To cast our rays By taking a look at the transformation pipeline described in [Matrices And Transformations](MatricesAndTransformations.md).

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

Now that we have the ray direction and it's origin (it's the camera position), we can calculate intersections with other objects in the scene. Using the intersections formulas we saw in previous chapiters, we can test our ray tracer by hardcoding a few objects in hlsl. Each object is assigned a different color and we select the closest hit distance in case of multiple intersections. If nothing is intersected, we just return black color.

![](Media/Recordings/Hardcoded%20Scene%2000.png)

The result might not be impressive right now but there is no point in doing crazy stuff with primitives as our goal will be to render polygonal meshes with the path tracer in the next chapiters.

You can check out the intersection functions in the [GeometryUtils.hlsl](https://github.com/alelievr/Modern-Rendering-Introduction/blob/master/ModernRenderer/assets/shaders/GeometryUtils.hlsl) source file.

## References

https://en.wikipedia.org/wiki/Path_tracing

https://pbr-book.org/4ed/contents

https://ssteinberg.xyz/2023rtplt/2023_rtplt.pdf
