---
title: "Triangle Intersection"
order: 70
author: Antoine Lelievre
category: Math3D 
layout: post
---

Intersecting a line and a triangle is the most complex and most useful function we'll see in this course. Most of the 3D models we'll render are made of thousands to millions of triangles so it's very important to have a fast triangle intersection algorithm.

To achieve this, we're going to use the [Möller–Trumbore intersection algorithm](https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm), this algorithm not only provides a relatively cheap way to compute the intersection position but also compute the barycentric coordinates of the triangle which will be useful when we'll talk about texturing. But first, let's see how we could compute an intersection between a line and a triangle intuitively.

## Naive Solution

The simplest solution that arise from the question how to calculate the intersection point between a triangle and a line is to first compute the intersection on an infinite plane (formed by the 3 vertices of the triangle) and the line. We already know how to calculate this from the previous chapter and the next step is equally straightforward and consist into checking that the intersection point found on the plane lies inside the triangle.

While this solution is the most logical, it's also not very performant. When doing path tracing, we're going to compute millions of line-triangle intersection per frame so we need to tackle optimization for this algorithm from the start.

Additionally, this approach doesn't compute the barycentric coordinates that we'll need in the future either, so we'd need to re-calculate them anyway costing even more performance along the line.

If you're interested in seeing how this solution is implemented in great detail, I recommend checking out [Ray-Triangle Intersection: Geometric Solution](https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution.html).

## Barycentric Coordinates

In this chapter we're particularly interested in the specific case of barycentric coordinates inside a triangle. These coordinates are composed of 3 values (or a single 3 component vector) often called $u$, $v$ and $w$, these values represent the position of a point inside the triangle.

To visualize what it means, let's add a point $P$ on the same plane than the triangle ABC, we'll also make sure that the position of P is within the triangle. Then we draw a segment from each vertex of the triangle towards the point $P$.

![](/assets/Recordings/TriangleIntersection%2005%20Sub-Triangles.gif)

Drawing these lines forms 3 new triangles: $\triangle PAB$, $\triangle PCA$, and $\triangle PBC$ and each one of this triangle correspond to a single barycentric coordinate. What can be counter intuitive here is that the barycentric coordinate of a vertex is represented by it's opposing triangle. In other words in this example, the barycentric coordinate of $B$ is represented by the triangle $\triangle PCA$.

In essence, computing the barycentric coordinates is as simple as calculating the area of the 3 sub-triangles formed by the new point inside the triangle (see figure below) using the [triangle area formula](https://en.wikipedia.org/wiki/Area_of_a_triangle). It's often more intuitive to think about barycentric coordinates as the 3 normalized areas of the 3 triangles formed by subdividing the triangle around an arbitrary point.

![](/assets/Recordings/TriangleIntersection%2000.gif)

We can observe a few tings by looking at this animation:
- first the values of $u$, $v$ and $w$ never go below 0 or above 1, this is a consequence of P staying inside the triangle. If P is outside of the triangle, then the barycentric coordinates are outside of the $(0, 1)$ range
- summing the areas of the 3 colored triangles always give the area of the main triangle $ABC$
- When the point P overlaps with a vertex of the triangle, only one of the barycentric values is equal to 1 and the two other are equal to 0.
- When the point P is on an edge of the triangle, a single barycentric value is 0.

The fact that the sum of the area of the sub-triangles is equal to the area of the main triangle is a very important property as it allows to use the barycentric coordinate to interpolate data between the vertices of a triangle while always making sure that the quantities are preserved.

For simplicity, we always consider that the main triangle have an area of 1, so we can directly use the barycentric coordinates to do the interpolation instead of having to scale the values with the actual area of the triangle.

$$u + v + w = 1$$

Which means that we can determine the value of a single variable if we know the two others: $w = 1 - u - v$, etc.

The fact that the barycentric coordinates are always normalized is very important when doing interpolation inside a triangle, more specifically it ensures that all the interpolated values stays within the bounds of the inputs. In other word, extrapolation is not permitted as long as the barycentric coordinates represent a point inside the triangle (we'll see that it's not always the case in the chapter about rasterization, which can lead to funny problems).

![](/assets/Recordings/TriangleIntersection%2001.gif)

In this animation, the point P represent the interpolated value of the 3 vertices of the triangle. As you can see, the value of P blend nicely between the 3 vertices no matter it's position. This interpolated value is simply obtained by multiplying the barycentric coordinates by the value of the vertex such as:

$$P = A * u + B * v + C * w$$

## Fast Intersection Algorithm

We already know that barycentric coordinates are very important for interpolation of values on the surface of a triangle, so it's a given that we'll need to calculate them in addition to the intersection position.

This algorithm proposes to formulate the intersection of the triangle in a different manner by directly solving for the barycentric coordinates of the point inside the triangle and then checks if the barycentric coordinates are inside the triangle.

<!-- ### Checking if the line is parallel to the triangle plane

Starting form our triangle, the first step is to formulate the plane passing through all the vertices of our triangle, for this, we'll use a different formula compared to what we used in the [Plane Intersection](PlaneIntersection.md) chapter: our plane will be defined by two coplanar vectors in space created by linking the vertices of our triangle (i.e. subtracting the vertices position will create a vector representing the length and direction needed to move from one point to another in the triangle). -->

Let's consider a triangle ABC and a line called $l$ cross it, the line origin is denoted by a point called $P$, the line direction is called $\vec{L}$ and it's length is 1.

![](/assets/Recordings/TriangleIntersection%2003%20Triangle.png)

We can use these 4 points to form vectors by simply substracting the position of those points, let's create 3 additional vectors pointing towards the 3 vertices of the triangle from $P$.

$$\vec{PA} = A - P$$
$$\vec{PB} = B - P$$
$$\vec{PC} = C - P$$

We now have 4 vectors (including the line direction) and we'll use the [Triple Product](https://en.wikipedia.org/wiki/Triple_product) to calculate the volume of 3 parallelepiped formed by those 4 vectors, let's see what it looks like with the first parallelepiped formed with the 3 vectors $\vec{L}$, $\vec{PA}$ and $\vec{PB}$.

> Note that if you're unsure how the Triple Product relates to the volume of the parallelepiped formed by 3 vectors, you can check out this great lesson on [The Determinant by 3Blue1Brown](https://www.3blue1brown.com/lessons/determinant).

![](/assets/Recordings/TriangleIntersection%2003%20Parallelepiped.gif)

We can observe a couple of things from this schema, but the most important one is that one of the face of the parallelepiped (the one with the vectors $\vec{PA}$ and $\vec{PB}$) overlaps with the triangle edge perfectly and that the volume of the parallelepiped crosses the surface of the triangle, in other words, the parallelepiped covers a part of the area of the triangle. The result of the triple product is the volume of the parallelepiped shown in transparent white, but if you remember correctly this calculation can also give a negative number as result, which indicates that the space was flipped during computation. Let's see what happens when we move the point P towards the edge of the triangle

![](/assets/Recordings/TriangleIntersection%2003%20Determinants.gif)

As soon as the line crosses the edge of the triangle and doesn't intersect it anymore, we observe that the parallelepiped flips on itself, hence causing the determinant to be negative.

We can use this cheap calculation (only a cross and dot product) to check on which "side" is the line from the point of view of the triangle. and then by doing that for all 3 sides of the triangle, we can ensure that the line indeed crosses the triangle at a point.

At this stage of the algorithm with only have volume of 3 parallelepipeds, but we're interseted in the barycentric coordinates instead so let's finish the calculation. Fortunately we already know that the barycentric corrdinates are related to the area of the sub-triangles formed by the intersection point and the triangle vertices. Since we already have the volume of the 3 parallelepipeds we can do a similar thing by normalizing their volumes, hence giving an identical result to the normalized area of the sub-triangles.

![](/assets/Recordings/TriangleIntersection%2003%20Volume%20Barycentric.gif)

Once we have the 3 barycentric coordinates $u$, $v$ and $w$ we can compute the position of the intersection by interpolation like mentionned above.

### Optimizations in the code

Let's take a look at the original HLSL code of the algorithm:


```c
float ScalarTriple(float3 a, float3 b, float3 c)
{
    return dot(cross(a, b), c);
}

bool RayTriangleIntersect(
    float3 lineOrigin, float3 lineDirection,
    float3 a, float3 b, float3 c,
    out float u, out float v, out float w)
{
    // Create vectors from p to triangle vertices
    float3 pa = a - lineOrigin;
    float3 pb = b - lineOrigin;
    float3 pc = c - lineOrigin;
    
    // Test if the line is inside the edges bc, ca and ab. Done by testing
    // that the signed parallelepiped volumes, computed using scalar triple
    // products, are all positive
    u = ScalarTriple(lineDirection, pc, pb);
    if (u < 0.0f) return false;
    v = ScalarTriple(lineDirection, pa, pc);
    if (v < 0.0f) return false;
    w = ScalarTriple(lineDirection, pb, pa);
    if (w < 0.0f) return false;

    // Compute the barycentric coordinates (u, v, w) by normalizing the parallelepiped volumes.
    float denom = 1.0f / (u + v + w);
    u *= denom;
    v *= denom;
    w *= denom;
    return true;
}
```

In it's current state, we use the 3 triple products to compute the tethrahedral volumes but we can do better. For example, we can share the cross product calculations by rewriting the math using the properties of the triple product. First, let's write all the operation performed:
$$u = pb \cdot (lineDirection \times pc)$$
$$v = pc \cdot (lineDirection \times pa)$$
$$w = pa \cdot (lineDirection \times pb)$$

In the second line, we can change the order of the operands to perform the cross product between the line direction and pc, exactly the first line which allows us to share the calculation.

$$pb \cdot (lineDirection \times pa) = - pa \cdot (lineDirection \times pc)$$

We can rewrite the core loop of the algorithm like so:

```c
float3 m = cross(lineDirection, pc);
u = dot(pb, m); // ScalarTriple(lineDirection, pc, pb);
if (u < 0.0f) return false;
v = -dot(pa, m); // ScalarTriple(lineDirection, pa, pc);
if (v < 0.0f) return false;
w = ScalarTriple(lineDirection, pb, pa);
if (w < 0.0f) return false;
```

With these optimization, this intersection is quite fast. In fact it's often faster by a small margin than the famous [Möller–Trumbore intersection algorithm](https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm). Of course, the results depends on the hardware you test on but in most case, this one should be faster.

## Back-face and Front-face

When we model 3D geometry using triangles, we often use triangles that have a single visible side. This is an optimization so that triangles that are facing "away" from the camera are ignored, we'll see this in more details when talking about rasterization. The important point here is that a triangle has a front and back face.

Depending on the configuration of the 3D model we want to render, it's interesting to be able to render both front and back faces of the triangle. Right now our algorithm is not capable of doing that as it only renders it's front face.

To fix this, we need to change how we check the signs of the parallelepiped volumes: instead of just checking that they are all above 0, we can check that all u, v and w have the same sign like so:

```c
bool SameSign(float a, float b)
{
    return (a < 0 && b < 0) || (a > 0 && b > 0);
}

...

float3 m = cross(lineDirection, pc);
u = dot(pb, m); // ScalarTriple(lineDirection, pc, pb);
v = -dot(pa, m); // ScalarTriple(lineDirection, pa, pc);
if (!SameSign(u, v)) return false;
w = ScalarTriple(lineDirection, pb, pa);
if (!SameSign(u, w)) return false;
```

In this 3D scene you can see the front and back faces of the triangle, the green represent the front face and red the back face. Notice how the two triangles on the right pop in and out as they rotate, this is because the shader applied to them was configured to cull or discard one of the faces. The effect is also interesting on a sphere where we see the inside of the sphere when front faces are culled. In this scene the white cylinder crosses both sphere at the exact same locations.

![](/assets/Recordings/TriangleIntersection%2004%20Face%20Culling.gif)

## References

- 📄 [Barycentric coordinate system - Wikipedia](https://en.wikipedia.org/wiki/Barycentric_coordinate_system)
- 📄 [Area of a triangle - Wikipedia](https://en.wikipedia.org/wiki/Area_of_a_triangle)
- 📄 [Barycentric Coordinates - Scratchapixel](https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates.html)
- 📄 [Real-Time Collision Detection (Christer Ericson) - PDF](https://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf) (5.3.4 Intersecting Line Against Triangle - page 184)
- 📄 [Möller–Trumbore Ray-Triangle Intersection - Scratchapixel](https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection.html)
- 📄 [Möller–Trumbore Intersection Algorithm - Wikipedia](https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm)
