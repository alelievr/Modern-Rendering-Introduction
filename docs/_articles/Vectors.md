---
title: "Vectors"
order: 20
author: Antoine Lelievre
category: Math3D 
layout: post
---

# Vectors

In 3D we use vectors to store information about a particular object in space. Vectors are composed of several components, each of these component hold a single value. The number of components in a vector is identical to the number of dimensions we're working with, so in 3D we'll use 3 component vectors and in 2D 2 component vectors.

The most intuitive information we can put in a vector is position, we simply write down the coordinate in space of the position following the order of the axis X, Y and Z.

$$a=(2, 4, -3), b=(-1, 0, 4),...$$

// TODO: reduce point count and add the same color as the one in the image below

![](Media/Recordings/Vectors%2000.png)

Another way of thinking about these coordinates is how they represent an arrow in space starting from the origin (0, 0, 0) and going towards the point, in this view the tip of the arrows ends exactly on the coordinate of the vector. This representation of vectors is useful to help visualize what happens when we transform a vector (for example adding two vectors).

This arrow based representation of vectors especially make sense when we represent directions using vectors. A direction is a normalized vector which means that it's length is always 1, this property makes some calculation a lot easier as we'll see in the next chapters. Another way of thinking about direction vectors is that the tip of the arrow is always on the surface of a sphere (or a circle in 2D).

![](Media/Recordings/Vectors%2001.gif)

There are 3 special direction vectors in euclidian space that forms the basis of the space. These are the direction vectors aligned with the main axises of the space:
$$\hat{i} = (1, 0, 0)$$
$$\hat{j} = (0, 1, 0)$$
$$\hat{k} = (0, 0, 1)$$

By combining these 3 vectors and a scale, you can reach every point in space. This is the definition of the vector basis. In this example, the basis is in world space, but each different space have it's own basis so when we'll look at the object space, you'll see that it also have it's own basis.

Note that vectors are not limited to storing position and direction data, they can contain literally anything and they are sometimes used to store things that are a bit less intuitive like colors or rotations.

## Complementary references

I highly recommend watching this excellent series of video to build up your intuition about vectors and other mathematical tools related to linear algebra: [Essence of linear algebra](https://www.youtube.com/watch?v=fNk_zzaMoSs&list=PLZHQObOWTQDPD3MizzM2xVFitgF8hE_ab). There is also a text version along with some good interactive exercises if you prefer reading: https://www.3blue1brown.com/topics/linear-algebra

If you would like something more mathemacical oriented, then I recommend reading [immersivemath](https://immersivemath.com/ila/index.html), it has amazing interactive illustrations and goes more deeply into the mathematical equations of linear algebra.

There is also a very complete book about mathematics in computer graphics: [Mathematics for 3D Game Programming and Computer Graphics](https://canvas.projekti.info/ebooks/Mathematics%20for%203D%20Game%20Programming%20and%20Computer%20Graphics,%20Third%20Edition.pdf), it's quite old but the mathematical foundations behind graphics rendering hasn't changed so it's still a very good reference.

## References

- 🎥 [Essence of Linear Algebra - YouTube](https://www.youtube.com/watch?v=fNk_zzaMoSs&list=PLZHQObOWTQDPD3MizzM2xVFitgF8hE_ab)
- 📄 [Linear Algebra - 3Blue1Brown](https://www.3blue1brown.com/topics/linear-algebra)
- 📄 [Interactive Linear Algebra - ImmersiveMath](https://immersivemath.com/ila/index.html)
