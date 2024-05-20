# Plane - Ray Intersection

## Definition

We already know how to describe a ray (3D origin and direction vectors) but how is a plane defined? There are several ways to describe a plane mathematically and through code. The definition we're going to use for intersection is the algebraic plane formula: $\mathbf{({\color{aquamarine}p} - {\color{orange}p_0}) \cdot {\color{lime}n}}$ where ${\color{aquamarine}\mathbf{p}}$ is a point in space, ${\color{orange}\mathbf{p_0}}$ is a point on the plane and ${\color{lime}\mathbf{n}}$ is the normal of the plane. Note that a plane can also be describe with only it's normal and a value representing how far it is located along the normal line: ${\mathbf{\color{aquamarine}p} \cdot {\color{lime}n} + h}$.

By definition a plane encompasses all the points ${\color{aquamarine}p}$ that satisfy this equation ${ (\mathbf{{\color{aquamarine}p} - {\color{orange}p_{0}}) \cdot {\color{lime}n} = 0}}$, in other world you can see the plane as an infinite set of points p. 

If the plane crosses the origin, the equation can be simplified by removing the ${\color{orange}\mathbf{p_0}}$ term, this simplification is often done in calculation because we can easily subtract the origin of our plane to the arbitrary point we're evaluating.

The distance between an arbitrary point p to the plane crossing the origin is given by this simple formula: $\mathbf{p \cdot n}$

## Formula

$${\mathbf{ {p} = {l_{0}} + {l} \ d}}$$

$${\mathbf{ ({l} \cdot {n} )\ d+({l_{0}} -{p_{0}} )\cdot {n} =0.}}$$

## References

https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection