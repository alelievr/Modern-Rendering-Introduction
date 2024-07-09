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

// TODO: gif of flipping matrix because this page will be boring to read.

In this course, we'll be using the row major matrices as it is the default in DirectX and HLSL.

## Translation

## Rotation

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
