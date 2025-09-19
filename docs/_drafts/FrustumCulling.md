---
title: "Frustum Culling"
order: 20
author: Antoine Lelievre
category: RenderPipeline 
layout: post
---

Frustum culling is an optimization technique that consist into excluding objects that are outside of the field of view of the camera. This is one of the mandatory techniques that you need to implement if you want to render a complex scene. It is so important that tou can consider most of the 3D scene impossible to render if you don't implement it. This is why Frustum Culling is going to be the first step of our render pipeline.

The idea is simple, first we need a frustum, we can build this out of the view projection matrix of the camera. This is essentially the truncated pyramid shape that you've seen in the Camera chapter:

![](../assets/Recordings/Camera%2001.png)

## Frustum - Bounds Testing

The frustum of the camera can be tested against many kind of bounding volumes. In this chapter we'll go over the most common ones.

### Frustum - AABB

If you remember the chapter on [Axis Aligned Oriented Box](../_articles/AABBIntersection.md) You should already know what is an AABB and how to compute its intersection against a ray. Here we're not interested in computing an intersection between the Frustum and AABB but just to know if the two objects overlaps, fortunately, this is much simpler to do.

Let's start with the naive approach. The first step is to decompose the frustum in a set of 6 planes facing inwards (the plane normals point towards the center of the frustum)

![](../assets/Recordings/Frustum%20Culling%2000%20-%20Frustum%20Planes.gif)

## References

https://iquilezles.org/articles/frustumcorrect/