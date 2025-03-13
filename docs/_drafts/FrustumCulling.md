---
title: "Frustum Culling"
order: 20
author: Antoine Lelievre
category: RenderPipeline 
layout: post
---

Frustum culling is an optimization technique that consist into excluding objects that are outside of the field of view of the camera. This is one of the mandatory techniques that you need to implement if you want to render a complex scene.

The idea is simple, first we need a frustum, we can build this out of the view projection matrix of the camera. This is essentially the truncated pyramid shape that you've seen in the Camera chapter

![](../assets/Recordings/Camera%2001.png)

## Frustum - Bounds Testing

The frustum of the camera can be tested against many kind of bounding volumes

## References

https://iquilezles.org/articles/frustumcorrect/