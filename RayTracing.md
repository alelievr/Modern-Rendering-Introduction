# Ray Tracing

Ray tracing is is a technique that consist in computing the intersection between a ray and another primitive. This technique is used in a lot of different algorithm in rendering and provide a simple and intuitive approach to rendering.

A ray is composed of two vectors: one position and one direction.

Each intersection with a ray allows to retrieve information such as the hit position, which 3D models was hit by the ray, which triangle was hit inside the mesh. From this intersection, we can then get the mesh surface data and interpolate it.

## Path Tracing

[Path tracing](https://en.wikipedia.org/wiki/Path_tracing) is the name of a [Light Transport](https://en.wikipedia.org/wiki/Light_transport_theory) algorithm, it uses the principle of ray-tracing to simulate how light rays interacts with a 3D scene.

The idea of the algorithm is pretty simple: we're going to start shooting rays from the camera, these rays will intersect the objects in the scene, at each intersection we'll evaluate how light interacts with the surface and we'll accumulate the lighting information to simulate light bouncing between objects.

There are several types of path tracer that exist, for this section, we're particularly interested in "backward" path tracing. It's called backward because we generate rays from the camera and then trace back to a surface before evaluating the lighting.

## PBRT 4th edition

In this course the reference renderer we're making will mostly follow the guidelines of [Physically Based Rendering V4](https://pbr-book.org/4ed/contents) with some simplifications. We're also doing it fully on the GPU from the start whereas the book only talk about GPU implantation at the end.

## Approximations

Here we are already starting to approximate by assuming that the light can be represented by a photo going straight in space and interacting with any surface that it comes in contact. We already know that this is physically wrong because we know from observation that light behaves both as a particle and a wave. So with ray-tracing it's impossible to represent interference patterns or diffraction grating which we assume is okay for our use case (If you're interested in this, you can take a look at [A Generalized Ray Formulation For Wave-Optics Rendering](https://ssteinberg.xyz/2023rtplt/2023_rtplt.pdf)).

Some path tracers are trying to do a closer match to reality by tracing multiple rays per photon, each ray representing a single wavelength of light (usually we do at least 3 ray for the wavelength corresponding to red, green and blue), this is called spectral path tracing. It allows to accurately model diffraction in prisms, iridescence, fluorescence, etc. We're also going to assume that these are not needed for our course because as you can imagine this is pretty expensive to compute and we'd need to adjust not only all the lighting equations to work with wavelength but also all our materials.

## Bevy Implementation

For bevy, we're going to add a new plugin that will handle the rendering of our path tracer. We'll start by simple code that traces an unlit sphere:

## References

https://en.wikipedia.org/wiki/Path_tracing

https://pbr-book.org/4ed/contents

https://ssteinberg.xyz/2023rtplt/2023_rtplt.pdf
