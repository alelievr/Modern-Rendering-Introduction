# Meshes

A mesh is an object that contains a collection of data describing a set of polygonal surfaces. More specifically in this course we'll refer as Mesh a list of data representing a list of triangles forming a 3D object.

This list of data contains information about the vertices and connectivity information (which vertices are connected to each other). Since we're talking about triangle meshes here, the vertices will be group by 3 to form the 3 summits of a triangle.

## Vertices

A vertex describes the information that each point in the 3D mesh contains, usually the vertex contains at least the position in space but it can contain anything else and this data can be accessed by shaders during the rendering.

## Primitives

The 3D pipeline APIs provide different types of primitives like point, lines, triangles list, triangle strip, triangle fan, etc. The most commonly used is the triangle list, while it's not the most efficient in term of bandwidth as you need to store a lot more indices compared to a triangle strip, it is the most flexible as it allows to represent models that are made from multiple parts not connected to each others

## Rendering

Since meshes are used during the rendering, it's essential to make sure that they contain all the information required for both the lighting and material evaluation of the object. In addition to the position, meshes often comes with normals, UVs

![](Media/Recordings/Meshes%2000.gif)

## 3D model formats

There are a lot of 3D model formats that can be imported to polygonal meshes, the most popular is FBX. It doesn't really matter as long as we can import correct the mesh from a DCC tool. We usually need to perform post import steps to compress the data or generate missing data from the mesh that we need for the rendering (tangent for example) or rescale the model to adapt to the engine convention ( + z up / y up modes)

## References

https://en.wikipedia.org/wiki/Polygon_mesh
