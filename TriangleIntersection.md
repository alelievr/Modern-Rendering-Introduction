# Triangle intersection

Intersecting a line and a triangle is the most complex and most useful function we'll see in this course. Most of the 3D models we'll render are made of thousands to millions of triangles so it's very important to have a fast triangle intersection algorithm.

To achieve this, we're going to use the [Möller–Trumbore intersection algorithm](https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm), this algorithm not only provides a relatively cheap way to compute the intersection position but also compute the barycentric coordinates of the triangle which will be useful when we'll talk about texturing. But first, let's see how we could compute an intersection between a line and a triangle intuitively.

## Naive Solution

The simplest solution that arise from the question how to calculate the intersection point between a triangle and a line is to first compute the intersection on an infinite plane (formed by the 3 vertices of the triangle) and the line. We already know how to calculate this from the previous chapiter and the next step is equally straightforward and consist into check that the intersection point found on the plane lies inside the triangle.

While this solution is the most logical, it's also not very performant. When doing path tracing, we're going to compute millions of line-triangle intersection per frame so we need to tackle optimization for this algorithm from the start.

Additionally, this approach doesn't compute the barycentric coordinates that we'll need in the future either, so we'd need to re-calculate them anyway costing even more performance along the line.

If you're interested in seeing how this solution is implemented in great detail, I recommend checking out [Ray-Triangle Intersection: Geometric Solution](https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution.html).

## Barycentric Coordinates

In this chapiter we're particularly interested in the specific case of barycentric coordinates inside a triangle. These coordinates are represented with 3 values (or a single 3 component vector) often called $u$, $v$ and $w$, these values represent the position of a point inside the triangle.

In essence, computing the barycentric coordinates is as simple as calculating the area of each sub-triangles using the [triangle area formula](https://en.wikipedia.org/wiki/Area_of_a_triangle) which is why it's often more intuitive to think about barycentric coordinates as the 3 normalized areas of the 3 triangles formed by subdividing the triangle around an arbitrary point.

![](media/Recordings/TriangleIntersection%2000.gif)

We can observe a few tings by looking at this animation:
- first the values of $u$, $v$ and $w$ never go below 0 or above 1, this is a consequence of P staying inside the triangle.
- summing the areas of the 3 small triangle always give the area of the main triangle $ABC$
- When the point P overlaps with a vertex of the triangle, only one of the barycentric values is equal to 1 and the two other are equal to 0.
- When the point P is on an edge of the triangle, a single barycentric value is 0.

Summing the area of the 3 colored triangles always gives you the area of the main triangle, this is a very important property as it allows to use the barycentric coordinate to interpolate data between the vertices of a triangle while always making sure that the quantities are preserved.

For simplicity, we always consider that the main triangle have an area of 1, so we can directly use the barycentric coordinates to do the interpolation instead of having to scale the values with the actual area of the triangle.

$$u + v + w = 1$$

Which means that we can determine the value of a single variable if we know the two others: $w = 1 - u - v$, etc.

The fact that the barycentric coordinates are always normalized is very important when doing interpolation inside a triangle, more specifically it ensures that all the interpolated values stays within the bounds of the inputs. In other extrapolation is not permitted as long as the barycentric coordinates represent a point inside the triangle (we'll see that it's not always the case in the chapiter about rasterization, which can lead to funny problems). 

![](Media/Recordings/TriangleIntersection%2001.gif)

In this animation, the point P represent the interpolated value of the 3 vertices of the triangle. As you can see, the value of P blend nicely between the 3 vertices no matter it's position. This interpolated value is simply obtained by multiplying the barycentric coordinates by the value of the vertex such as:

$$P = A * u + B * v + C * w$$

## Möller–Trumbore Intersection Algorithm

We already know tha barycentric coordinates are very important for interpolation of values on the surface of a triangle, so it's a given that we'll need to calculate them in addition to the intersection position.

This algorithm proposes to formulate the intersection of the triangle in a different manner by directly solving for the barycentric coordinates of the point inside the triangle and then checks if the barycentric coordinates are inside the triangle.

### Checking if the line is parallel to the triangle plane

Starting form our triangle, the first step is to formulate the plane passing through all the vertices of our triangle, for this, we'll use a different formula compared to what we used in the [Plane Intersection](PlaneIntersection.md) chapiter: our plane will be defined by two coplanar vectors in space created by linking the vertices of our triangle (i.e. subtracting the vertices position will create a vector representing the length and direction needed to move from one point to another in the triangle).

$$\vec{AB} = B - A$$
$$\vec{AC} = C - A$$

From there, we could compute the normal of the plane with a cross product and then use it to check if our line intersects the plane but the original algorithm doesn't use it in further calculations. Instead it computes the cross product of the line direction and the edge $\vec{AC}$ which creates a vector $\vec{e}$ that will be perpendicular to the edge $\vec{AB}$ if the input line is parallel to the plane. This is greatly explained in the [Cross Product lesson by 3Blue1Brown](https://www.3blue1brown.com/lessons/cross-products), if you're uncertain about your understanding of cross products and determinants are I recommend you to see this lesson before continuing.



// TODO: visualize the cross and dot product between the triangle edges and the line direction
// This result is also the determinant of the triangle

### Linear Equations Formulation

### Cramer's rule

// TODO: visualize the determinant in 3D (volume of the Parallelepiped)

### Optimizations in the code

## Back-face and Front-face

// TODO: gif of two rotating triangle: backface, and cull off

## References

https://www.graphics.cornell.edu/pubs/1997/MT97.pdf

https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm

https://en.wikipedia.org/wiki/Barycentric_coordinate_system

https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates.html

https://en.wikipedia.org/wiki/Area_of_a_triangle

https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection.html

https://www.youtube.com/watch?v=fK1RPmF_zjQ
