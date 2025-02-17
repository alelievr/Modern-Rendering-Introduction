---
title: "AABB Intersection"
order: 60
author: Antoine Lelievre
category: Math3D 
layout: post
---

An Axis Aligned Bounding Box is a [bounding volume](https://en.wikipedia.org/wiki/Bounding_volume) represented by a box that has no rotation relative to the origin of the space. In the image below, you can see that each line of the boxes are parallel to one of the basis vectors.

![](/assets/Recordings/AABBIntersection%2000.png)

These bounding volumes are often used to accelerate other algorithm by serving as proxy to a more complex object. The idea behind this is to intersect the bounding volume which is a fast operation to perform and then intersect what's inside the bounding volume which is often more costly to evaluate. This is the basic idea behind an acceleration structure, there is a hierarchy of bounding volumes that are fast to intersect.

## Ray - AABB Intersection

To intersect a ray with an AABB, we can take a simple approach using slabs, a slab is defined by the region created in between 2 parallel planes.

![](/assets/Recordings/AABBIntersection%2001.gif)

## Conclusion

## References

- 📄 [Bounding volume - Wikipedia](https://en.wikipedia.org/wiki/Bounding_volume)
- 📄 [Real-Time Collision Detection (Christer Ericson) - PDF](https://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf) (5.3.3 Intersecting Ray or Segment Against Box - page 179)
