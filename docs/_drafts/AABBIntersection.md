---
title: "AABB Intersection"
order: 60
author: Antoine Lelievre
category: Math3D 
layout: post
---

An Axis-Aligned Bounding Box (AABB) is a type of [bounding volume](https://en.wikipedia.org/wiki/Bounding_volume) characterized by a box that has no rotation relative to the origin of the coordinate space. This means that each face of the box is aligned with one of the primary axes of the space (x, y, and z). In the image below, you can observe that each edge of the AABB is parallel to one of the basis vectors of the coordinate system.

![AABB Diagram](../assets/Recordings/AABBIntersection%2000.png)

AABBs are commonly utilized to optimize the performance of algorithms by serving as a proxy for more complex objects. The principle is to first intersect the simpler AABB, which is a quick operation, and then to intersect the more complex object contained within the AABB. This is the idea behind acceleration structures, where a hierarchy of bounding volumes is used to quickly determine the final intersection between many objects.

## Ray-AABB Intersection

To intersect a ray with an AABB, we can take a simple approach using slabs, a slab is defined by the region created in between 2 parallel planes. Then we compute the intersection points between this slab and the ray, like in this animation of a 2D AABB being intersected by a ray:

![Ray-AABB Intersection Animation](../assets/Recordings/AABBIntersection%2001.gif)

To determine if a ray intersects an AABB, we calculate the intervals between the intersection points of the ray with each slabs that define the AABB. We then check these intervals for each dimension (x, y, and z) of the AABB. An intersection is confirmed if the intervals are valid across all dimensions, meaning the closest intersection point is closer than the farthest intersection point.

This method is detailed in the algorithm from [Real-Time Collision Detection](https://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf) by Christer Ericson.

```c
// Intersects a ray R(t) = p + t*d against an Axis-Aligned Bounding Box (AABB).
// If there's an intersection, returns intersection distance tmin and point q.
int IntersectRayAABB(
    float3 p,            // Ray origin
    float3 d,            // Ray direction
    float3 aabbMin,      // Minimum corner of the AABB
    float3 aabbMax,      // Maximum corner of the AABB
    out float tmin,      // Intersection distance
    out float3 q         // Intersection point
)
{
    tmin = 0.0f;             // Start with the minimum distance (can be -FLT_MAX for entire ray)
    float tmax = FLT_MAX;    // Maximum allowable distance for the ray (segment length or ∞)

    // Iterate over each axis (x, y, z)
    for (int i = 0; i < 3; i++)
    {
        // If the ray is parallel to the slab (AABB plane pair)
        if (abs(d[i]) < EPSILON)
        {
            // If the origin is outside the slab, there's no intersection
            if (p[i] < aabbMin[i] || p[i] > aabbMax[i])
                return 0;
        }
        else
        {
            // Compute the intersection t-values for the near and far planes of the slab
            float ood = 1.0f / d[i];
            float t1 = (aabbMin[i] - p[i]) * ood;
            float t2 = (aabbMax[i] - p[i]) * ood;

            // Ensure t1 is the intersection with the near plane, and t2 with the far plane
            if (t1 > t2)
            {
                float temp = t1; // Swap t1 and t2
                t1 = t2;
                t2 = temp;
            }

            // Update tmin and tmax to compute the intersection interval
            tmin = max(tmin, t1);
            tmax = min(tmax, t2);

            // If the interval becomes invalid, there is no intersection
            if (tmin > tmax)
                return 0;
        }
    }

    // If we reach here, the ray intersects the AABB on all 3 axes
    q = p + d * tmin; // Compute the intersection point
    return 1;
}
```

## Ray - AABB Test

When working with Axis-Aligned Bounding Boxes (AABBs) in graphics, there are scenarios where simply determining whether a ray intersects the AABB is sufficient, without requiring the exact intersection point. In such cases, we can adopt a more efficient approach that minimizes unnecessary computations.

To achieve this, instead of treating the problem as a ray-AABB intersection, we use a finite **line segment**. This is practical because, in most cases, we already have defined constraints for the ray's minimum and maximum hit distances. These constraints are commonly derived from the properties of the camera (e.g., near and far clipping planes in a 3D viewing frustum). Using these bounds, we can construct a finite segment that represents the portion of the ray that lies within the camera's viewing range.


```c
// Tests if a segment specified by points p0 and p1 intersects an Axis-Aligned Bounding Box (AABB).
bool TestSegmentAABB(
    float3 p0,              // Start point of the segment
    float3 p1,              // End point of the segment
    float3 aabbMin,         // Minimum corner of the AABB
    float3 aabbMax          // Maximum corner of the AABB
)
{
    // Compute box center-point and half-length extents
    float3 c = (aabbMin + aabbMax) * 0.5f; // Box center
    float3 e = aabbMax - c;                // Box half-extent

    // Segment midpoint and halflength vector
    float3 m = (p0 + p1) * 0.5f;           // Segment midpoint
    float3 d = p1 - m;                     // Segment halflength vector
    m = m - c;                             // Translate box and segment to the origin

    // Test world coordinate axes as separating axes
    float adx = abs(d.x);
    if (abs(m.x) > e.x + adx) return false;
    float ady = abs(d.y);
    if (abs(m.y) > e.y + ady) return false;
    float adz = abs(d.z);
    if (abs(m.z) > e.z + adz) return false;

    // Add a small epsilon to counteract potential arithmetic errors when the segment is
    // near-parallel to one of the coordinate axes
    adx += EPSILON;
    ady += EPSILON;
    adz += EPSILON;

    // Test cross products of segment direction vector with coordinate axes
    if (abs(m.y * d.z - m.z * d.y) > e.y * adz + e.z * ady) return false; // Cross with X-axis
    if (abs(m.z * d.x - m.x * d.z) > e.x * adz + e.z * adx) return false; // Cross with Y-axis
    if (abs(m.x * d.y - m.y * d.x) > e.x * ady + e.y * adx) return false; // Cross with Z-axis

    // No separating axis found; segment overlaps the AABB
    return true;
}
```

## AABB Distance

To calculate the distance to an AABB, you can use symetry, the point in space 

```c
float sdBox(
    float3 p, // The point in space to test (relative to the box center).
    float3 b  // The half extents of the box (i.e., the dimensions from the center to the box surface).
)
{
    float3 q = abs(p) - b;

    // Calculate the signed distance:
    // - The length of the positive offset (outside distance)
    // - Add the inside (negative) distance for points inside the box
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}
```

## OBB

Oriented Bounding Boxes (OBBs) generalize AABBs by removing the restriction that the bounding box must align with the coordinate axes. Instead, an OBB can be arbitrarily rotated in 3D space, allowing it to better conform to the shape of the object it encloses.

An OBB is typically represented by:
- **A position vector**: The center of the box in 3D space.
- **A basis of three unit vectors**: These form an orthonormal set that defines the orientation of the box. Each vector represents one of the box's principal axes.
- **Scales (or half-extents)**: Each dimension is associated with a scale value that determines the size of the box along the corresponding basis vector.

This structure allows an OBB to enclose objects in a rotation-independent manner and to fit tightly around objects regardless of their orientation in the scene. This property is beneficial for culling as it means that the efficiency of the culling improves and we'll have less objects to render

### Ray - OBB Intersection

The intersection against an OBB is surprisingly easy as we already hae the algorithm to intersect an AABB, we just need to add the rotation part. We can simply do that by rotating the ray with the inverse rotation matrix of the OBB. As you've seen in the [Matrix Inverse](MatricesAndTransformations.md#Matrix-Inverse) section, inverting a rotation matrix is easy, we can just take it's transpose and do a multiply on the ray.

## Conclusion

## References

- 📄 [Bounding volume - Wikipedia](https://en.wikipedia.org/wiki/Bounding_volume)
- 📄 [Distance functions - Inigo Quilez](https://iquilezles.org/articles/distfunctions/)
- 📄 [Real-Time Collision Detection (Christer Ericson) - PDF](https://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf) (5.3.3 Intersecting Ray or Segment Against Box - page 179)
