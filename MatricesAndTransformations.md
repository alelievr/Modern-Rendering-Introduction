# Matrices and Transformations

In computer graphics, matrices are a very useful tool, they are represented by a table of values or a vector of vector. I recommend reading [Matrix Multiplication from 3Blue1Brown](https://www.3blue1brown.com/lessons/matrix-multiplication) before tackling this chapter to understand well the effect of combining matrices together and performing several transformation at the same time. They are extensively used to facilitate the computation of objects position, rotation and scale in the 3D world.

We'll mostly use 2x2, 3x3 and 4x4 square matrices in this chapter, they are the most used type of matrices in computer graphics.

> Note that sometimes we also use non-square matrices but it's often an optimization to avoid storing all the components.

## Matrix Order and Transpose

The first thing to determine is how to write our matrices. There are two conventions for writing matrices: row major and column major.
To visualise the difference, you can imagine a list of numbers representing all the components of the matrix, for example let's take 9 values to create a 3x3 matrix:

$$[a_0, a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8]$$

To construct a matrix with the column major, we take every number in the array from left to right and start putting them in the matrix from the top left corner, filling each column of the matrix from top to bottom and then switching to the following column.

$$\begin{bmatrix}
a_0 & a_3 & a_6\\
a_1 & a_4 & a_7\\
a_2 & a_5 & a_8
\end{bmatrix}$$

Similarly, to create a row major matrix, we take each number in the array from left to write and fill each row of the matrix from left to write starting from the top left corner.

$$\begin{bmatrix}
a_0 & a_1 & a_2\\
a_3 & a_4 & a_5\\
a_6 & a_7 & a_8
\end{bmatrix}$$

You've probably noticed that the numbers on the diagonal (top left corner to bottom right) are unchanged when we use the two different matrix formats. This is because the row major matrix is the transpose of the column major matrix (and vice versa). Indeed transposing a matrix allows to move back and forth between column and row major formats, you can see this operation as the matrix rotating around it's diagonal.

![](Media/Recordings/Matrix%2000%20Transpose.gif)

In this course, we'll be using row major matrices which is the default in DirectX. This means that we'll be using the pre-multiplication order where the value to transform is placed on the left side of the operand.

$$result = vector * matrix$$

## Translation

We already know how to represent a translation using a 3D vector, so let's see how to encode it into a 4x4 matrix so that it gives the same result as if we used a vector to translate the object.

We can write our position vector like so: $v = [x, y, z]$ The first visible problem is that there is only 3 components in this vector which mean that we cannot multiply it directly with our 4x4 matrix, we need to add 1 value. This value will be called $w$.

To build our translation matrix, let's start from the identity matrix which once multiplied by our vectors, leave it unchanged:

$$\begin{bmatrix}
1 & 0 & 0 & 0 \\
0 & 1 & 0 & 0 \\
0 & 0 & 1 & 0 \\
0 & 0 & 0 & 1
\end{bmatrix}$$

For our translation matrix to work we want to add 3 different values to our vector, each corresponding to a translation on one axis. In other words we simply want that our matrix multiplication operation simplifies itself to a simple vector addition. If we look closely to the matrix multiplcation, we can find such values on the bottom row of the matrix, adding the X, Y, and Z components of a translation vector will have the same effect as adding the two vectors when the matrix multiplication is performed:

$$
\begin{bmatrix} v_x & v_y & v_z & v_w \end{bmatrix}
*
\begin{bmatrix}
1 & 0 & 0 & 0 \\
0 & 1 & 0 & 0 \\
0 & 0 & 1 & 0 \\
t_x & t_y & t_z & 1
\end{bmatrix}
=
\begin{bmatrix} v_x * 1 + v_w * t_x & v_y * 1 + v_w * t_y & v_z * 1 + v_w * t_z & v_w \end{bmatrix}
$$

If you observe closely the result, we see that the $w$ component of $v$ is a factor of the translation of the matrix. Simply setting is to 1 in the vector before performing the multiplication will make sure that we translate our position vector by exactly the amount inside the translation matrix. Note that we can ignore the $w$ component after performing the multiplication because we only need the 3 dimensions for our resulting position.

![](Media/Recordings/Matrix%2001.gif)

## Rotation

The rotation is a bit more complex but it uses the same principle as we've seen in the translation matrix which is simplifying the matrix multiplication operation by keeping some '0' inside to control the operation we want to perform.

Let's start by describing what kind of rotation our matrix will perform. The rotation matrix rotate a position around the origin in 3 axises. The rotation matrix is in fact a combination of 3 separate rotation matrices multiplied by each others, one per axis.

To understand how it works, let's see how to rotate an object around a single axis. The object in the example is going to be a point represented by a 3D vector and the axis will be the red X axis. The purple arrow was added to help visualize the motion.

![](Media/Recordings/Matrix%2003%20Rotation-Point.gif)

The first thing to notive is that the X coordinate isn't affected by a rotation on the X axis, this is similar for the rotation on Y and Z axies and gives us a hit about the matrix we need to do this rotation: some part of the matrix will have the property of the identity matrix so that when the position is multiplied by the matrix, it's X axis is unchanged.

Let's write again our position vector $v$ multiplied by an identity matrix while highlighting which values we know are already correct:

$$\begin{bmatrix} v_x & v_y & v_z & v_w \end{bmatrix}
*
\begin{bmatrix}
\colorbox{green}1 & 0 & 0 & \colorbox{green}0 \\
\colorbox{green}0 & 1 & 0 & \colorbox{green}0 \\
\colorbox{green}0 & 0 & 1 & \colorbox{green}0 \\
\colorbox{green}0 & \colorbox{green}0 & \colorbox{green}0 & \colorbox{green}1
\end{bmatrix}$$

We know that the column on the left is good because its the value of $v_x$, the bottom row is also correct because it's multiplied by $v_w$ which we don't care in the result as our vector has only 3 components, and the column on the right is also multiplied by $v_w$. Though if we leave the vector like so, a value from $v_w$ could seep into the result and make it incorrect, that's why we always set the 4th component of the vector to 0 when rotating a position.

All that's left to do is rotating the $Y$ and $Z$ coordinates of the position, this can simply be done using trigonometric function sin and cos. Rotating a position around an axis is the same as calculating it's new coordinate after moving the point of x degrees on a circle, see how the green point orbits around the X axis forming a circle, the new position of this point on the circle is what we're looking for. We can easily solve this problem in 2D as we already know that the value of the $x$ coordinate.

A 2D rotation is described by these formulas:

$$rotated_x = x * \cos(\theta) - y * \sin(\theta)$$
$$rotated_y = x * \sin(\theta) + y * \cos(\theta)$$

As you can see there are some similarities with matrix multiplication in this formula, in fact these formulas can be written as a single matrix that gets multiplied by a 2D vector:

$$
{\begin{bmatrix}
\cos \theta & -\sin \theta \\
\sin \theta &\cos \theta \\
\end{bmatrix}}
$$

 This means that it's easy for us to take integrate this matrix into our existing identity matrix to rotate Y and Z components:

$$
\begin{bmatrix}
1 & 0 & 0 & 0 \\
0 & \cos \theta & -\sin \theta & 0 \\
0 & \sin \theta & \cos \theta & 0 \\
0 & 0 & 0 & 1
\end{bmatrix}
$$

And this is the final form of the rotation matrix on the $X$ axis, if we apply the same logic to the $Y$ and $Z$ axises, we end up with 3 matrices describing the rotation of any angle on the 3 axises of the standard basis. In 3D we often multiply these 3 matrices together to be able to rotate a position in any direction.

![](Media/Recordings/Matrix%2002%20Rotation.gif)

> Note that to rotate a direction, we usually convert the matrix to a 3x3 by removing the bottom row and right column. This is because 4x4 matrices usually encodes multiple transformation at the same time (rotation, translation, scale) and we don't want any translation when rotating a direction (remember a direction is a vector that always start from the origin).

If you've payed close attention to the values inside the rotation matrix, you may have noticed that in every axis specific rotation matrices, there is always a row with a value equal to one of the basis vector:

$$
\begin{bmatrix}
\colorbox{red}1 & \colorbox{red}0 & \colorbox{red}0 \\
0 &\cos \theta & -\sin \theta \\
0 & \sin \theta & \cos \theta \\
\end{bmatrix}

\begin{bmatrix}
\cos \theta & 0 & \sin \theta \\
\colorbox{green}0 & \colorbox{green}1 & \colorbox{green}0 \\
-\sin \theta & 0 & \cos \theta \\
\end{bmatrix}

\begin{bmatrix}
\cos \theta & -\sin \theta & 0 \\
\sin \theta & \cos \theta & 0 \\
\colorbox{blue}0 & \colorbox{blue}0 & \colorbox{blue}1 \\
\end{bmatrix}
$$

This is an interesting property that allows you to retrieve the basis of an object from it's rotation matrix, all you have to do is to create vectors from these rows. This is commonly referred to the right, up, and forward directions of the object.

## Scale

The scale matrix is a lot simpler than the two previous ones, starting again from the identity matrix, we only need to find the matrix that can multiply a vector by a number. But remember, multiplying by the identity matrix is similar to multiply by 1. So we just need to multiply our identity matrix by a number to get our scaling matrix:

$$
\begin{bmatrix}
s & 0 & 0 & 0 \\
0 & s & 0 & 0 \\
0 & 0 & s & 0 \\
0 & 0 & 0 & 1
\end{bmatrix}
$$

![](Media/Recordings/Matrix%2003%20Scale.gif)

This is called uniform scaling, because the scaling of the object doesn't affect it's overall shape. There is also non-uniform scaling that allows stretching more in certain axises than others:

$$
\begin{bmatrix}
s_x & 0 & 0 & 0 \\
0 & s_y & 0 & 0 \\
0 & 0 & s_z & 0 \\
0 & 0 & 0 & 1
\end{bmatrix}
$$

![](Media/Recordings/Matrix%2003%20Non%20Uniform%20Scale.gif)

## Projection Matrices

Projection matrix allows to perform a [3D Projection](https://en.wikipedia.org/wiki/3D_projection) transforming how the scene is viewed. There are numerous types of projections but in realtime rendering we mainly use perspective and orthographic projections so let's see those two:

### Perspective

Like the name implies, this projection gives perspective to the scene, this is the most familiar type of projection because we're used to see the world with perspective, that's what allows us to evaluate distances.

The perspective projection depends on the field of view, near and far plane, exactly like our simple camera from the previous chapter. The wider the field of view, the more objects you can see, but at the cost of deformation, it's a bit like looking at a curved mirror, it can allow you to view more than a flat mirror but with a distorted image. These 3 parameters form a frustum shape, if a point is within this space then it's visible.

To build the perspective matrix, it's actually important to know what we want as a result after multiplying our positions with this matrix so let's talk a bit about HCLIP space first.

Homogeneous Clip space (or HCLIP for short) is represented by a 4 component vector where the $x$, $y$ and $z$ component values represent a position in this space and $w$ is the clipping value. This is called clip space because every coordinate outside the $[-w, +w]$ range is outside of the frustum and thus not visible by the camera.

Having the CLip space boundary defined by a single value means that it can be represented as a cube centered on the origin. In fact the projection matrix actually transforms the frustum shape into a box with $2*w$ as edge size.

![](Media/Recordings/Matrix%2004%20Frustum%20To%20Box.gif)

Unlike the translation, rotation or scaling matrix, this transformation actually warps space itself so everything that was in the frustum shape will end-up inside the box. This is the cause of the deformation caused by the perspective (objects appear smaller as they get farther and skewed when they get close to the border of the frustum).

You can see this in action with this animation. Look at how the objects are morphing while following the deformation of the perspective transform. In the bottom left corner, is a view filmed with a camera without perspective, it allows to get a better look at the deformation occurring here.

![](Media/Recordings/Matrix%2005%20Perspective%20Deformation.gif)

So how do we calculate this $w$ value? Let's think about the simplest case where the frustum view have a FoV of 90 degrees and the aspect ratio is squared (vertical FoV = horizontal FoV). To transform this shape into a box what we can do is set the value of $w$ to be equal to the value of $z$. when the $x$ and $y$ coordinates gets divided by $z$ it "flattens" the frustum into a box.

You might have noticed a small issue with the calculation above: the z coordinate always end up being 1 (z / z = 1), so we're actually loosing the depth information. Which means that right now we're transforming the frustum into a plane and not a box. We need to keep this z value "intact" for further calculation.

To do that, we can remap the $z$ value between 0 and $+w$ so that when $z$ is equal to the near plane, $z / w = 0$ and when $z$ is equal to the far plane, $z / w = 1$. We can do that by using the near and far plane values with this formula where $z$ is the input coordinate of the point, $f$ is the far plane and $n$ is the near plane.

$$z' = \frac{z * f - f * n}{f - n}$$

Which we can decompose into two parts:

$$z' = z*\frac{f}{f - n} - \frac{f * n}{f - n}$$

And then we can introduce these two parts into the projection matrix in the column where $z$ is computed:

$$
\begin{bmatrix}
1 & 0 & 0 & 0 \\
0 & 1 & 0 & 0 \\
0 & 0 & \frac{f}{f - n} & 1 \\
0 & 0 & -\frac{f * n}{f - n} & 0
\end{bmatrix}
$$

> Notice the 1 in the column used to compute the $w$ component of the vector, it indicates that we set the value of the to the component $z$ of the input vector.

Now that we have a working matrix for a fixed angle, we need to make it work with any FoV between $]0, 180[$ degrees. There is actually a very simple way to handle that, we already know that the z coordinate is not affected by perspective, only the X and Y are, so the two cells of the matrix that we'll change are the two $1$ in the diagonal of the matrix. If  you remember the previous chapter, these numbers are analog to the scale matrix, in fact this scaling the space on the X and Y axises is the simplest solution to handle an arbitrary FoV. With no scale (a value of 1), the FoV is 90 degrees. The smaller the FoV is, the lower the scaling value needs to be to stretch the space accordingly, and vice versa. at FoV 180 degree, the scaling value reaches infinity, that's why this value is excluded from the range of possible FoVs, similarly at 0 FoV, nothing is visible so it's excluded as well.

![](Media/Recordings/Matrix%2006%20FoV.gif)

We can calculate the scaling factor needed with trigonometry rules by forming a square triangle between the camera origin and the far plane.

What we want to know is the length of the side of the triangle, we already know the two points and the angle, so we can easily get this with $arctan(fov / 2)$, note that the fov needs to be converted in radians for the arctan which can be done by multiplying by $\frac{pi}{180}$.

> Note that if the camera screen is not square (which is often the case), there are two different field of views: one vertical and one horizontal. In this course when field of view is mentioned, we are talking about the vertical field of view.

which gives us this final projection matrix:

$$
\begin{bmatrix}
\arctan(\frac{fov}{2}) * a & 0 & 0 & 0 \\
0 & \arctan(\frac{fov}{2}) & 0 & 0 \\
0 & 0 & \frac{f}{f - n} & 1 \\
0 & 0 & -\frac{f * n}{f - n} & 0
\end{bmatrix}
$$

Where $a$ is the aspect ratio of the screen (width divided by the height) and $fov$ is the field of view in degrees

### Orthographic

An orthographic projection doesn't have any perspective which makes it less intuitive than the perspective projection. In this projection, all the objects you see on screen keep the same size regardless of the distance. It can be useful to have this kind of projection to get a sense of the scale of certain objects or simply to create a different art style.

The orthographic camera is simpler to control, the field of view is replaced by a single size value that controls how large the view is or how zoomed in the view is. We still need to keep the aspect ratio to match the output resolution and there is the same near and far plane as the perspective camera with a slight difference: both near and far plane can have negative values without breaking the camera.

Multiplying a position by this matrix should output a value in HCLIP space, except that this time the we only need to scale the coordinates by a certain factor per axis, because all the objects are already in a box compared to a frustum for the perspective.

Here's the orthographic matrix:

$$
\begin{bmatrix}
\frac{2}{sizeX * a} & 0 & 0 & 0 \\
0 & \frac{2}{sizeY} & 0 & 0 \\
0 & 0 & \frac{-2}{f - n} & 0 \\
0 & 0 & -\frac{f + n}{f - n} & 0
\end{bmatrix}
$$

## Matrix Inverse

The matrix inverse is a really nice operation, you can see this operation as the equivalent of the [reciprocal](https://en.wikipedia.org/wiki/Multiplicative_inverse) operation. In 3D it is heavily used to calculate the reverse of a transformation or undo a transformation. You'll see it uses in the transformation pipeline, this is basically what allows us to go back in the transformation chain.

## Transformation Pipeline

The transformation pipeline in 3D is a series of transformation, usually applied by matrix multiplications that has the purpose to transform a 3D object into 2D surface on a screen.

![](Media/Recordings/Matrix%20Transformation%20Pipeline.png)

## Camera Relative Rendering

Another important concept to be aware of is what camera relative rendering is as we'll be using this throughout all the course. Dealing with large worlds where the coordinates of objects becomes big values causes precision issues due to the nature of the floating point. These issues can visually cause objects to jitter, camera to be unstable or shaking, etc. To get rid of these issues, instead of having a fixed world space origin at (0, 0, 0) in the world, we say that the origin is the camera. Because the camera is never very far away from what it sees, it's safe to assume that there will be no precision issue in the computations of the shader.

This has 3 main implications:
- The position of the camera is always 0.
- For each vertex on the GPU we need to subtract the world position of the camera before doing any calculation.
- To get the world space position, we need to add the world position of the camera.

To clarify in which space we are, we call these two **Camera Relative World Space** and **Absolute World Space**.

## Matrix alternatives

There are alternative solutions to matrices for performing transforms, amongst the most common ones, we find quaternions, dual quaternions, and Geometric Algebra.

### Quaternions & Dual Quaternions

Quaternions are use exclusively to describe a rotation, they are a bit harder to visualize than euler angles mostly because of the 4 components but they are extremely efficient at storing rotation values, performing interpolation on rotation and avoiding gimbal lock issues.

Dual quaternions adds the translation on top of the rotation part, they are often used in animated meshes to improve the quality of the animation.

If you want to learn more on this topic, check out [Math in Game Development Summit: A Visual Guide to Quaternions and Dual Quaternions
](https://www.youtube.com/watch?v=en2QcehKJd8).

### Geometric Algebra

Geometric Algebra offer a complete alternative to matrix transformations. The goal of this frameworks is to provide a more intuitive approach to representing geometric entities like points, lines, planes, and rotations, which results in cleaner and more compact mathematical expressions. Unlike matrices, which can obscure the underlying geometric meaning, GA and PGA allow direct manipulation of objects and transformations within a unified algebraic system, enabling more straightforward handling of rotations, translations, scales, etc.

However, the vast majority of realtime applications are still using matrices. Historically matrices were first to provide an unified transformation framework, now the number of tutorial and resources available to learn matrices is huge and easy to access. Similarly almost every 3D application and library only exposes matrices as API which forces developers to use them by default. Matrices are also baked into shading languages providing native support for matrix operations and specific optimizations which makes them easier to use. GA and PGA tend to have a steeper learning curve for developers because of those reasons.

For more information about, you can check out [LookMaNoMatrices](https://enkimute.github.io/LookMaNoMatrices/).

## References

https://en.wikipedia.org/wiki/Matrix_(mathematics)

https://antongerdelan.net/opengl/raycasting.html

http://davidlively.com/programming/graphics/opengl-matrices/row-major-vs-column-major/

https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/geometry/row-major-vs-column-major-vector.html

https://en.wikipedia.org/wiki/Transpose

https://en.wikipedia.org/wiki/Rotation_matrix

https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/projection-matrix-introduction.html

https://en.wikipedia.org/wiki/Multiplicative_inverse

https://learn.microsoft.com/en-us/windows/win32/dxtecharts/the-direct3d-transformation-pipeline

https://learn.microsoft.com/en-us/windows/win32/direct3d9/viewports-and-clipping
