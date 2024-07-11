# Meshes

A mesh is an object that contains a collection of data describing a set of polygonal surfaces. More specifically in this course we'll refer as Mesh a list of data representing a list of triangles forming a 3D object.

This list of data contains information about the vertices and connectivity information (which vertices are connected to each other). Since we're talking about triangle meshes here, the vertices will be group by 3 to form the 3 summits of a triangle.

## Vertices

A vertex describes the information that each point in the 3D mesh contains, usually the vertex contains at least the position in space but it can contain anything else and this data can be accessed by shaders during the rendering.

## Primitives

The 3D pipeline APIs provide different types of primitives like point, lines, triangles list, triangle strip, triangle fan, etc. The most commonly used is the triangle list, while it's not the most efficient in term of bandwidth as you need to store a lot more indices compared to a triangle strip, it is the most flexible as it allows to represent models that are made from multiple parts not connected to each others

## Rendering

Since meshes are used during the rendering, it's essential to make sure that they contain all the information required for both the lighting and material evaluation of the object.

As we're following PBR guidelines, most of our objects will use textures and we need a way to apply those textures on 3D meshes. The most common way to apply textures is called [UV mapping](https://en.wikipedia.org/wiki/UV_mapping), this techniques relies on the fact that we can unwrap the 3D mesh onto a 2D surface which represent a texture. In practice, complex algorithms can perform the UV unwrapping after the mesh has been authored, this algorithm then assign a 2 component vector to each vertex of the mesh called 'UV', this UV data is a coordinate from 0 to 1 that indicates the position in the texture to apply at this vertex position. We can also visualize the UV using the red and green color channels:

![](Media/Recordings/Meshes%2004.gif)

Using the same method, we can display the vertex position of the mesh, except that this time the values are not normalized so we just display them as color without any modification.

![](Media/Recordings/Meshes%2003.png)

Almost all the lighting algorithms will require the [surface normal](https://en.wikipedia.org/wiki/Normal_(geometry)), this information is usually authored when the 3D model is created because it influences a lot its rendering. We can visualize the normals of a mesh by simply drawing an arrow pointing in the direction of the normal vector from each vertex like so:

![](Media/Recordings/Meshes%2000.gif)

Notice that the color of the arrow here reprensents the value of the normal vector. The normal vector is always normalized (it is a direction only) so it's values are between -1 and 1, the color of the normal can be interpreted as color by simply remapping the -1 to 1 values to 0 to 1 (color = $normal * 0.5 + 0.5$) these values can then be mapped directly to RGB colors. Nowadays models have quite a lot of vertices so it's not practical to represent them with little arrows so instead we use the color of the normal directly on the model:

<table>
  <tr><th>Suzanne</th><th>Lucy</th></tr>
  <tr>
    <td><img src="Media/Recordings/Meshes%2001.png"  alt="2D" width = 100% height = 100% ></td>
    <td><img src="Media/Recordings/Meshes%2002.png"  alt="3D" width = 100% height = 100% ></td>
  </tr>
</table>

The normal values are playing a big part in the details of the lighting of a surface, right now we have only one normal value per vertex which is often not enough to render high quality models. Instead of just adding more vertex to store more normals, we can add use a normal map. The normal map consist in high frequency normals being stored in a 2D texture, when the model is rendered, the UVs are used to know which texel to sample from the normal map. As you can see, we get back a lot of details from applying a normal map to the object:

![](Media/Recordings/Meshes%2006.gif)

The normal values you see above are in object space, that's the same space as the vertex positions are in, it means that all the vertex data is relative to the object basis (the origin of this space is the object pivot or center most of the time). While it's possible to store the normals in object space in the normal map texture, it's not practical, for this reason almost every normal map texture is stored in tangent space nowadays. The tangent space is constructed from the basis formed using the vertex normal, tangent and bi-tangent.

We already have the normal vector in the mesh data, what about the two others? like the normal, tangent data can be embed in the 3D model when it's created but sometimes it's missing so we have to re-compute it.

There are several algorithm that exists to compute the tangent from the normal, but they all use UVs to determine the direction of the tangent. Let's see what the simplest algorithm looks like:

```c


```

TODO: show how to compute tangent and bitangent from UV and vertex position (naive method).

TODO: visualize the tangent space basis at a vertex on a mesh and how it allows to store data independent from the curvature of a surface

Tangent space offers several advantages compared to object space to store normal values:

- We need to store only 2 values instead of 3 because the normal cannot point to below the surface
- textures can be animated using UVs, for example scrolling the UVs to make a flow effect is not possible with object space normals as at some point the normals would point below the surface causing the lighting to break.
- You can reuse normal maps on other objects that have the same UV layout. This is very interesting when combined with planar mapping because it become possible to interpolate between different normal maps.

## 3D model formats

There are a lot of 3D model formats that can be imported to polygonal meshes, the most popular is FBX. It doesn't really matter as long as we can import correct the mesh from a DCC (Digital Content Creation) tool. Usually, post import steps are performed to compress the data or generate missing data from the mesh that we need for the rendering (tangent for example) or rescale the model to adapt to the engine convention ( + z up / y up modes).

On of the simplest 3D model format is the [obj](https://en.wikipedia.org/wiki/Wavefront_.obj_file) file format. With this text file you can really see the how a mesh is formed structurally with the list of vertex positions and the faces that connects the vertices using integers that represent the indices of the vertex. In this case the 'f' instruction represent the connectivity information between the vertices that we can then interpret to form triangles ready to be rendered.

Lately, more complex formats emerged from the need to share more information between different DCC softwares. A good example of this is the [USD](https://openusd.org/release/index.html) file format which can store a complete 3D scene with lights, animations, 3D models, textures, materials, etc.

## References

https://en.wikipedia.org/wiki/UV_mapping

https://en.wikipedia.org/wiki/Normal_(geometry)

https://en.wikipedia.org/wiki/Polygon_mesh

https://en.wikipedia.org/wiki/Wavefront_.obj_file

https://openusd.org/release/index.html

https://casual-effects.com/data/

https://en.wikipedia.org/wiki/List_of_common_3D_test_models