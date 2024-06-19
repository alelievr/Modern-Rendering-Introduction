# Plane - Ray Intersection

## Plane Definition

We already know how to describe a ray (3D origin and direction vectors) but how is a plane defined? There are several ways to describe a plane mathematically and through code. The definition we're going to use for intersection is the algebraic plane formula: $\mathbf{({\color{aquamarine}p} - {\color{orange}p_0}) \cdot {\color{lime}n}}$ where ${\color{aquamarine}\mathbf{p}}$ is a point in space, ${\color{orange}\mathbf{p_0}}$ is a point on the plane and ${\color{lime}\mathbf{n}}$ is the normal of the plane. Note that a plane can also be describe with only it's normal and a value representing how far it is located along the normal line: ${\mathbf{\color{aquamarine}p} \cdot {\color{lime}n} + h}$.

By definition a plane encompasses all the points ${\mathbf{\color{aquamarine}p}}$ that satisfy this equation ${ (\mathbf{{\color{aquamarine}p} - {\color{orange}p_{0}}) \cdot {\color{lime}n} = 0}}$, in other world you can see the plane as an infinite set of points p.

If the plane crosses the origin, the equation can be simplified by removing the ${\color{orange}\mathbf{p_0}}$ term, this simplification is often done in calculation because we can easily subtract the origin of our plane to the arbitrary point we're evaluating.

One interesting thing to notice is that when solving the plane equation with an arbitrary point, the result is a single value representing the distance between the plane and the point. In itself this equation can be called a Signed Distance Field function because it gives us the distance to the line for any point in space. We'll probably see that in a future chapiter.

## Line / Vector Definition

This is the formula of a vector starting from point $\mathbf{l_{0}}$ and pointing in the direction of $\mathbf{l}$, d is the distance variable that allows to represent any point along the line forming this formula:

$${\mathbf{ {p} = {l_{0}} + {l} \ d}}$$

Note that this is both the formula for a line and a vector, in our case we're interesting in solving the intersection between a vector and another object, which means that we must discard any intersection if the object is "behind" the vector like so:

![](Media/Recordings/Plane%2001.png)

This is equivalent to say that we'll discard any solution where the $\mathbf{d}$ variable is negative (going in the inverse direction compared to the vector $\mathbf{l}$)

## Solving the equation

First, we need to make the vector-plane equation, this is pretty easy since we already have both equations, we just need to combine them following a certain logic: We know that the point we're looking for is both on the plane and on the line defined by both equations, this is the definition of an intersection after all. Then it means that we can substitute the point ${\mathbf{\color{aquamarine}p}}$ in the plane equation by a point defined with the line equation, which gives:

$${\mathbf{ (({l_{0}} + {l} \ d) - {\color{orange}p_{0}} )\cdot {\color{lime}n} = 0}}$$

We're interested in finding $\mathbf{d}$ which is the distance between the start of the vector $l_{0}$ and the intersection point.

To do this, we can expand the formula:

$${\mathbf{ ({l} \ d + {l_{0}} - {\color{orange}p_{0}})\cdot {\color{lime}n} = 0}}$$

$${\mathbf{ ({l} \ d) \cdot {\color{lime}n} + ({l_{0}} - {\color{orange}p_{0}}) \cdot {\color{lime}n} = 0}}$$

Extracting the variable $d$ outside of the dot product

$${\mathbf{ ({l} \cdot {\color{lime}n} )\ d+({l_{0}} -{\color{orange}p_{0}} )\cdot {\color{lime}n} = 0}}$$

Then we simply have to isolate d

$${\mathbf{ d = -{(l_{0} - {\color{orange}p_{0}}) \cdot {\color{lime}n}  \over {l} \cdot {\color{lime}n} }}}$$

And we can remove the negative sign in front of the result by swapping ${\mathbf{\color{orange}p_{0}}}$ and $\mathbf{l_0}$. Swapping those two variables is equivalent to negating the vector in the parenthesis.

$${\mathbf{ d = {({\color{orange}p_{0}} - l_{0}) \cdot {\color{lime}n}  \over {l} \cdot {\color{lime}n} }}}$$

Note that we also need to handle the case where both the line and vector are parallel, in this case there is no intersection. Luckily we can know that in advance by first computing the dot product of $\mathbf{l}$ and $\mathbf{n}$ which will give 0 if both vectors are parallel. Look how this visualization of the dot product reacts whe the vector $l$ overlaps with the line $P$.

![](Media/Recordings/Plane%2002.gif)

## Result

After solving the equation, computing the intersection point is trivial since we know the value of d, we just need to solve the line equation with this value.

We often need more information than the intersection point when rendering an object, so it's not rare for the intersection computation to only be the beginning. For the plane, we don't really need to compute any additional information because the plane is identical in any point (there is no curvature, normal is already given by it's formula, etc.).

## References

https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection

https://iquilezles.org/articles/distfunctions/
