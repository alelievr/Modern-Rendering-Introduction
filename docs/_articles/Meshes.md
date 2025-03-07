---
title: "Meshes"
order: 10
author: Antoine Lelievre
category: Authoring 
layout: post
---

A mesh is an object that contains a collection of data describing a set of polygonal surfaces. More specifically in this course we'll refer as Mesh a list of data representing a list of triangles forming a 3D object.

This list of data contains information about the vertices and connectivity information (which vertices are connected to each other). Since we're talking about triangle meshes here, the vertices will be group by 3 to form the 3 summits of a triangle.

## Vertices

A vertex describes the information that each point in the 3D mesh contains, usually the vertex contains at least the position in space but it can contain anything else and this data can be accessed by shaders during the rendering.

## Primitives

The 3D pipeline APIs provide different types of primitives like point, lines, triangles list, triangle strip, triangle fan, etc. The most commonly used is the triangle list, it uses a list of index which represents each triangle individually, each index in the list points to a single vertex, thus 3 indices forms a triangle. In the triangle list format, each triangle is completely separated, so you need 3 indices for each triangle in the mesh, other format such as triangle strip and fan are more compact in term of indices usage but it comes with the constrain that each triangle is directly connected to the previous one which is not often possible depending on the mesh represented.

The mesh data format is very good at representing the connectivity between vertices and offers a good ratio of flexibility and performances which makes it the preferred method of encoding 3D objects for realtime rendering as long as they have a finite surface.

## Rendering

Since meshes are used during the rendering, it's essential to make sure that they contain all the information required for both the lighting and material evaluation of the object.

### Texture Coordinates (UV)

As we're following PBR guidelines, most of our objects will use textures and we need a way to apply those textures on 3D meshes. The most common way to apply textures is called [UV mapping](https://en.wikipedia.org/wiki/UV_mapping), this techniques relies on the fact that we can unwrap the 3D mesh onto a 2D surface which represent a texture. In practice, complex algorithms can perform the UV unwrapping after the mesh has been authored, this algorithm then assign a 2 component vector to each vertex of the mesh called 'UV', this UV data is a coordinate from 0 to 1 that indicates the position in the texture to apply at this vertex position. We can also visualize the UV using the red and green color channels:

![](../assets/Recordings/Meshes%2004.gif)

Something important to be aware with UV mapping is that there are also conventions to respect which affects how the calculations depending on UVs are calculated. In this course we'll be following the [DirectX texture coordinate](https://learn.microsoft.com/en-us/windows/win32/direct3d9/texture-coordinates) convention which means that the UVs start in the top left corner of the texture.

### Position

Using the same method, we can display the vertex position of the mesh, except that this time the values are not normalized so we just display them as color without any modification.

![](../assets/Recordings/Meshes%2003.png)

### Normal

Almost all the lighting algorithms will require the [surface normal](https://en.wikipedia.org/wiki/Normal_(geometry)), this information is usually authored when the 3D model is created because it influences a lot its rendering. We can visualize the normals of a mesh by simply drawing an arrow pointing in the direction of the normal vector from each vertex like so:

![](../assets/Recordings/Meshes%2000.gif)

Notice that the color of the arrow here reprensents the value of the normal vector. The normal vector is always normalized (it is a direction only) so it's values are between -1 and 1, the color of the normal can be interpreted as color by simply remapping the -1 to 1 values to 0 to 1 (color = $normal * 0.5 + 0.5$) these values can then be mapped directly to RGB colors. Nowadays models have quite a lot of vertices so it's not practical to represent them with little arrows so instead we use the color of the normal directly on the model:

<table>
  <tr><th>Suzanne</th><th>Lucy</th></tr>
  <tr>
    <td><img src="../assets/Recordings/Meshes%2001.png"  alt="2D" width = 100% height = 100% ></td>
    <td><img src="../assets/Recordings/Meshes%2002.png"  alt="3D" width = 100% height = 100% ></td>
  </tr>
</table>

The normal values are playing a big part in the details of the lighting of a surface, right now we have only one normal value per vertex which is often not enough to render high quality models. Instead of just adding more vertex to store more normals, we can add use a normal map. The normal map consist in high frequency normals being stored in a 2D texture, when the model is rendered, the UVs are used to know which texel to sample from the normal map. As you can see, we get back a lot of details from applying a normal map to the object:

![](../assets/Recordings/Meshes%2006.gif)

### Tangent & Bitangent

The normal values you see above are in object space, that's the same space as the vertex positions are in, it means that all the vertex data is relative to the object basis (the origin of this space is the object pivot or center most of the time). While it's possible to store the normals in object space in the normal map texture, it's not practical, for this reason almost every normal map texture is stored in tangent space nowadays. The tangent space is constructed from the basis formed using the vertex normal, tangent and bi-tangent.

Tangent space offers several advantages compared to object space to store normal values:

- We need to store only 2 values instead of 3 because the normal cannot point to below the surface
- textures can be animated using UVs, for example scrolling the UVs to make a flow effect is not possible with object space normals as at some point the normals would point below the surface causing the lighting to break.
- You can reuse normal maps on other objects that have the same UV layout. This is very interesting when combined with planar mapping because it become possible to interpolate between different normal maps.

We already have the normal vector in the mesh data, what about the two others? like the normal, tangent data can be embed in the 3D model when it's created but sometimes it's missing so we have to re-compute it.

There are several algorithm that exists to compute the tangent from the normal, one of the simplest algorithm uses the UVs and vertex positions of the triangles inside the mesh to calculate the tangent. Let's see how we could determine the direction of the tangent vector from these values.

First, we know that the tangent, by definition, lies on the surface of the triangle. So its vector can be written as a combination of 2 edges of the triangle (because 2 edges of the triangle form a 2D basis on a plane). In other words we only need to find 2 values to multiple 2 edges of the triangle to form the tangent.

Having access to the UVs of the mesh gives us a good indication on which direction the tangent should be pointing. If we consider that the UVs will be used to sample a normal map encoded in tangent space, then the basis created by the normal, tangent and bitangent vectors must be in the same direction than the UVs to avoid any rotation in the normal data of the texture.

Indeed, to calculate the tangent we can directly use the delta of the 2 UV values as the scaling factors for our two triangle edges. For the bitangent we do exactly the same

Let's consider the surface of a low poly sphere and display the vertex normal (blue), tangent (red) and bitangent (green) as arrows for each vertex:

![](../assets/Recordings/Meshes%2007.png)

It may not be obvious from this image but the direction of the tangent and bitangent are directly aligned with the UVs, to better visualize that, we can display the X and Y components of the UVs separately as grayscale:

<table>
  <tr><th>X</th><th>Y</th></tr>
  <tr>
    <td><img src="../assets/Recordings/Meshes%2009.png"  alt="3D" width = 100% height = 100% ></td>
    <td><img src="../assets/Recordings/Meshes%2008.png"  alt="2D" width = 100% height = 100% ></td>
  </tr>
</table>

As you can see the X UV corrdinates are going from left to right on the sphere and the small red arrows are pointing in this direction.

At this step, our algorithm correctly calculates the tangent and bitangent on flat-shaded meshes, but what happens if the normal of the triangle is not perpendicular to it's surface?

You may have noticed on the animations above that the normal is shared between several faces, this allows to leverage the vertex interpolation during rendering to allow the mesh to look "smooth" when computing the lighting. In the case of the sphere we can clearly see that the normals are not perpendicular to the surface of the triangles otherwise we'd need multiple normals per vertex to account for all the triangle angles. This scenario reveals a flow in our algorithm: the tangent and bitangents are aligned with the triangle surface which means that they are not perpendicular to the normal. Since we need these 3 vectors to form a [standard basis](https://en.wikipedia.org/wiki/Standard_basis) we need to fix that using the [Gram-Schmidt orthogonalization](https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process).

For reference, here's the HLSL code used to generate normals from the data of a triangle inside a mesh:

```c
// This algorithm operates on a triangle as input. 
// a, b, and c represents the vertex positions of the triangle.
// uvA, uvB, and uvC represents the uv values for each vertex respectively.
// normalA is the normal of the vertex A

// Compute vectors for two edges of the triangle from the common vertex 'a'
float3 ab = b - a;
float3 ac = c - a;
// Compute the difference of UVs associated with the edges of the triangle.
float2 uvAB = uvB - uvA;
float2 uvAC = uvC - uvA;

// Finally compute the tangent
tangent = (ac * uvAB.y - ab * uvAC.y);

// At this point the tangent is not necessarily perpendicular to the normal so let's fix that
tangent -= normalA * dot(normalA, tangent);
tangent = normalize(tangent);

// Finally compute the bitangent with a cross product
bitangent = cross(normal, tangent);
```

> Note that this simple algorithm fails when the mesh data is bad or degenerated, most of the 3D engines are now using a more robust method using mikktspace from Morten S. Mikkelsen. If you're interested, you can learn more about this technique here: http://www.mikktspace.com.

### Other data

As you noticed, the mesh data format is very flexible and allows storing basically anything per vertex as long as it's not too big (there is a limit on how many per vertex attributes you can have and each attribute is limited to 4 32 bit components). It's not uncommon to see other data like colors being encoded per vertex, this can be an alternative to textures. Animated meshes also embed more data like the weight and indices of each bone of the rig. Some data is also specific to certain type of assets like trees which can store the pivot of the branch for procedural wind animation, etc.

## 3D model formats

There are a lot of 3D model formats that can be imported to polygonal meshes, the most popular is [FBX](https://en.wikipedia.org/wiki/FBX). It doesn't really matter as long as we can import correct the mesh from a DCC (Digital Content Creation) tool. Usually, post import steps are performed to compress the data or generate missing data from the mesh that we need for the rendering (tangent for example) or rescale the model to adapt to the engine convention ( + z up / y up modes).

One of the simplest 3D model format is the [obj](https://en.wikipedia.org/wiki/Wavefront_.obj_file) file format. With this text file you can really see the how a mesh is formed structurally with the list of vertex positions and the faces that connects the vertices using integers that represent the indices of the vertex. In this case the 'f' instruction represent the connectivity information between the vertices that we can then interpret to form triangles ready to be rendered.

Lately, more complex formats emerged from the need to share more information between different DCC softwares. A good example of this is the [USD](https://openusd.org/release/index.html) file format which can store a complete 3D scene with lights, animations, 3D models, textures, materials, etc.

## Conclusion

Meshes are the foundation of 3D modeling and rendering, offering a versatile structure for representing polygonal surfaces, primarily through triangles. By organizing vertices and connectivity information, meshes enable the creation of 3D surfaces that are essential for the rasterizer to function. Various data attributes, such as texture coordinates, normals, tangents, and bitangents, can be stored on a per-vertex basis and are crucial for evaluating light and materials applied to the mesh. The flexibility of mesh formats allows them to store additional data, supporting a wide range of use cases, from static models to complex procedural animations. This adaptability, coupled with advances in file formats like USD, continues to drive mesh technology as an integral part of 3D graphics and real-time rendering.

## References

- 📄 [UV mapping - Wikipedia](https://en.wikipedia.org/wiki/UV_mapping)
- 📄 [Normal (geometry) - Wikipedia](https://en.wikipedia.org/wiki/Normal_(geometry))
- 📄 [Polygon mesh - Wikipedia](https://en.wikipedia.org/wiki/Polygon_mesh)
- 📄 [Wavefront .obj file - Wikipedia](https://en.wikipedia.org/wiki/Wavefront_.obj_file)
- 📄 [OpenUSD Release - OpenUSD](https://openusd.org/release/index.html)
- 📄 [Casual Effects - Data](https://casual-effects.com/data/)
- 📄 [List of common 3D test models - Wikipedia](https://en.wikipedia.org/wiki/List_of_common_3D_test_models)
- 📄 [List of common 3D test models - Github](https://github.com/alecjacobson/common-3d-test-models)
- 📄 [glTF Sample Assets - Khronos Group](https://github.com/KhronosGroup/glTF-Sample-Assets)
- 📄 [Texture Coordinates - Direct3D 9 - Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/direct3d9/texture-coordinates)
- 📄 [FBX - Wikipedia](https://en.wikipedia.org/wiki/FBX)
