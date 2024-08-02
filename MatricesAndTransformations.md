# Matrices and Transformations

In computer graphics, matrices are a very useful tool, they are represented by a table of values or a vector of vector. I recommend reading [Matrix Multiplication from 3Blue1Brown](https://www.3blue1brown.com/lessons/matrix-multiplication) before tackling this chapiter to understand well the effect of combining matrices together and performing several transformation at the same time. They are extensively used to facilitate the computation of objects position, rotation and scale in the 3D world.

We'll mostly use 2x2, 3x3 and 4x4 square matrices in this chapiter, they are the most used type of matrices in computer graphics.

> Note that sometimes we also use non-square matrices but it's often an optimization to avoid storing all the components.

## Matrix Order and Transpose

The first thing to determine is how to write our matrices. There are two conventions for writing matrices: row major and column major.
To visualise the difference, you can imagine a list of numbers representing all the components of the matrix, for example let's take 9 values to create a 3x3 matrix:

$$[a_0, a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8]$$

To construct a matrix with the column major, we take every number in the array from left to right and start putting them in the matrix from the top left corner, filling each column of the matrix from top to bottom and then switching to the folloing column.

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

Let's start by describing what kind of rotation our matrix will perform. The rotation matrix rotate a position around an origin in 3 axises. In fact the common rotation matrix we can observe in most game engine is in fact a combination of 3 separate rotation matrices multiplied by each others.



## Scale

## Camera Matrices

## Matrix Inverse

## Transformation Pipeline

### Object Space

### World Space

### View Space

### HCLIP space

### NDC Space

### Screen Space

## References

https://en.wikipedia.org/wiki/Matrix_(mathematics)

https://antongerdelan.net/opengl/raycasting.html

http://davidlively.com/programming/graphics/opengl-matrices/row-major-vs-column-major/

https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/geometry/row-major-vs-column-major-vector.html

https://en.wikipedia.org/wiki/Transpose

https://en.wikipedia.org/wiki/Rotation_matrix
