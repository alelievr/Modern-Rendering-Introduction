# Matrices and Transformations

In computer graphics, matrices are a very useful tool, they are represented by a table of values or a vector of vector. I recommend reading [Matrix Multiplication from 3Blue1Brown](https://www.3blue1brown.com/lessons/matrix-multiplication) before tackling this chapiter to understand well the effect of combining matrices together and performing several transformation at the same time. They are extensively used to facilitate the computation of objects position, rotation and scale in the 3D world.

We'll mostly use 2x2, 3x3 and 4x4 square matrices in this chapiter, they are the most used type of matrices in computer graphics.

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

In this course, we'll be using row major matrices which widely used in HLSL and DirectX which means that we'll be using the pre-multiplication order where the value to transform is planed on the left side of the operand.

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

TODO

## Matrix Inverse

The matrix inverse is a really nice operation, you can see this operation as the equivalent of the [reciprocal](https://en.wikipedia.org/wiki/Multiplicative_inverse) operation. In 3D it is heavily used to calculate the reverse of a transformation or undo a transformation. You'll see it uses in the transformation pipeline, this is basically what allows us to go back in the transformation chain.

## Transformation Pipeline

The transformation pipeline in 3D is a series of transformation, usually applied by matrix multiplications that has the purpose to transform a 3D object into 2D surface on a screen.

### Object Space

### World Space

### View Space

### HCLIP space

### NDC Space

### Screen Space

## Matrix alternatives

TODO: talk a bit about:
https://enkimute.github.io/LookMaNoMatrices/

## References

https://en.wikipedia.org/wiki/Matrix_(mathematics)

https://antongerdelan.net/opengl/raycasting.html

http://davidlively.com/programming/graphics/opengl-matrices/row-major-vs-column-major/

https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/geometry/row-major-vs-column-major-vector.html

https://en.wikipedia.org/wiki/Transpose

https://en.wikipedia.org/wiki/Rotation_matrix

https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/projection-matrix-introduction.html

https://en.wikipedia.org/wiki/Multiplicative_inverse
