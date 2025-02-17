---
title: "Sphere Intersection"
order: 60
author: Antoine Lelievre
category: Math3D 
layout: post
---

# Sphere - Ray Intersection

The intersection between a ray and a sphere can lead to no intersection, one intersection or two if the ray pass through the sphere.

Intersecting a sphere for the rendering in path tracing is not necessarily useful in itself (even though right now we can use it to populate some objects in our scene for the path tracer) but it'll also be useful later on to accelerate some algorithm. For example a sphere can be used to represent a proxy that encapsulate a complex 3D object, intersecting the sphere first allow to know quickly if the ray is likely to intersect the 3D model. This is known as a [bounding sphere](https://en.wikipedia.org/wiki/Bounding_sphere).

## Sphere definition

Defining a sphere in 3D space only requires two variables: a position and a radius. For the sake of simplicity in the following equations, the center of the sphere will be denoted $c$ and the radius $r$.

You might already know the formula of as sphere at the origin written as $x^2 + y^2 + z^2 = r^2$, this is the scalar form of the equation. For the sake of simplicity, we'll be using the vector notation which replaces the 3 unknown in the equation by one 3D vector that we call $x$ which gives $||x||^2 = r^2$.

The general formulation of the sphere can then be written as the following formula:

$$|| x - c ||^2 = r^2$$

Similarly to the line, solving this equation with an arbitrary point, gives the distance from this point to the surface of the sphere.

## Solving the equation

Following the same logic as the plane, we can substitute the unknown point $x$ in the sphere equation by the equation of the line. This substitution is the common technique to solve intersections, so we'll be using it for all our intersections.

$$|| (l_0 + ld) - c ||^2 = r^2$$

We want to know the value of d, so we can compute the intersection position on the sphere. First, we can expand the formula by rewriting the left side without the square.

$$(ld + (l_0 - c)) \cdot (ld + (l_0 - c)) = r^2$$

Next, rearrange the left side following the [Binomial theorem](https://en.wikipedia.org/wiki/Binomial_theorem) and move the $r^2$ on the left side as well.

$$(ld) \cdot (ld) + 2((ld) \cdot (l_0 - c)) + (l_0 - c) \cdot (l_0 - c) - r^2 = 0$$

Then we can factorize with $d$, which gives this [quadratic formula](https://en.wikipedia.org/wiki/Quadratic_formula):

$$d^2(l \cdot l) + 2d(l \cdot (l_0 - c)) + (l_0 - c) \cdot (l_0 - c) - r^2 = 0$$

A quadratic formula has two solutions that will represent the two distances where the line intersects the sphere. By convention we make sure that the vector representing the direction of the line is always normalized (it's length is 1) which allows us to write these two simplified formulas:

$$\nabla =[l \cdot (l_0 - c )]^{2}-(|| l_0 -c || ^{2}-r^{2})$$
$$d_0 =-[l \cdot (l_0 - c )] + {\sqrt {\nabla }}$$
$$d_1 =-[l \cdot (l_0 - c )] - {\sqrt {\nabla }}$$

## Result

Now that we have the two distances, we need to check if those distances form points that belongs to our vector (our vector is a subset of the line because it's infinite in only a single direction). Fortunately for us, we only need to check if the distance is negative to know this.

As you may have noticed, this is formula is a lot more complicated than the ray / plane intersection, in addition to this formula, we can do a cheaper check before evaluating the formula, this check consist in evaluating if the line intersects the sphere, this won't give us any information about the hit position in case both intersects but it provides a cheap early out solution, also in case of the bounding sphere check I mentionned above, we actually don't need the intersection distances, so it's cheaper to only evaluate this:

$$d = (l_0 - c) \cdot l $$

## References

- 📄 [Line–sphere intersection - Wikipedia](https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection)
