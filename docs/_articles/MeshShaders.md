---
title: "Mesh Shaders"
order: 50
author: Antoine Lelievre
category: RenderPipeline 
layout: post
---

# Mesh Shaders

Mesh shaders are a modern approach to writing shaders that handle both vertex transformation and primitive assembly. When implemented correctly, they can be faster than legacy vertex shaders and provide significantly more flexibility in terms of data layout for meshes. However, they are more complex to set up than vertex shaders and there are fewer examples or tutorials that explain how to get started with mesh shaders.

## Clusters & Meshlets

The first step in understanding mesh shaders is to explain the operations performed on mesh data before sending it to the mesh shader. To achieve optimal performance, we cannot simply pass the entire mesh as input to the mesh shader. Instead, the mesh must be divided into smaller parts called **meshlets**. Each meshlet is a group of triangles forming a small patch on the mesh's surface. In the image below, each colored patch represents a single meshlet:

![](../assets/Images/Stanford%20Bunny.png)

You might notice that most of the generated meshlets are roughly the same size. This is the result of the meshlet generation algorithm, which aims to minimize the bounding volume of each meshlet while maximizing [vertex reuse](https://interplayoflight.wordpress.com/2021/11/14/shaded-vertex-reuse-on-modern-gpus/) within each meshlet. Minimizing the bounding volume is particularly important when implementing a meshlet culling algorithm, which we will discuss in a future chapter.

For this course, we will generate meshlets using [meshoptimizer](https://github.com/zeux/meshoptimizer?tab=readme-ov-file#mesh-shading), which efficiently produces GPU-ready meshlet data.

**Clusters** are a concept similar to meshlets in that they also represent portions of a mesh. However, clusters are primarily used for spatial partitioning to accelerate algorithms like ray tracing. Unlike meshlets, clusters can be utilized in various algorithms and are not constrained by mesh shader requirements.

## The Graphics Pipeline

The diagram below, taken from the DirectX 12 documentation, illustrates the sequence of shader stages executed by the GPU when rendering an object. The blue nodes represent **fixed hardware functions**—these are physically embedded into the GPU silicon and cannot be programmed, which is why they are referred to as "fixed hardware functions." Typically, these functions serve as intermediaries between different shader stages, passing and converting data from one stage to another.

[![](../assets/Images/MeshShaderPipeline.png)](https://devblogs.microsoft.com/directx/dev-preview-of-new-directx-12-features/)

In this guide, we will focus on the **Mesh Shader Pipeline**, which is the new preferred method for rendering objects in modern graphics APIs. This pipeline is easier to understand because it has fewer stages and fewer fixed hardware functions.  

If you want to learn more about the stages in the legacy graphics pipeline, refer to the [DirectX 11 documentation](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-graphics-pipeline).

## Mesh Shaders

Mesh shaders are very similar to compute shaders, with a few extra features that specialize them for processing geometry. The most important thing to note is that the mesh shader output is directly connected to the rasterizer. This means that, in addition to performing vertex transformation operations, the mesh shader assembles the geometry that will feed the rasterizer. If you look at the pipeline diagram above, the mesh shader actually replaces the **Input Assembly (IA)** and **Vertex Shader (VS)** stages.

For reference, this is what a mesh shader might look like when working with meshlet data:

```c
struct MeshToFragment {
    float4 positionCS : SV_Position;
};

struct PrimitiveAttribute {
    uint primitiveID : SV_PrimitiveID;
    bool primitiveCulled : SV_CullPrimitive;
};

[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void mesh(
    uint threadID : SV_GroupThreadID,
    uint groupID : SV_GroupID,
    out indices uint3 triangles[MAX_OUTPUT_PRIMITIVES],
    out primitives PrimitiveAttribute sharedPrimitives[MAX_OUTPUT_PRIMITIVES],
    out vertices MeshToFragment vertices[MAX_OUTPUT_VERTICES]
) {
    // Fetch meshlet data
    Meshlet meshlet = meshlets[groupID];

    // Inform the GPU about the number of vertices and triangles the mesh shader will output
    SetMeshOutputCounts(meshlet.vertexCount, meshlet.triangleCount);

    // Process primitives
    uint primitiveId = threadID;
    if (primitiveId < meshlet.triangleCount) {
        triangles[primitiveId] = LoadPrimitive(meshlet.triangleOffset, primitiveId);

        PrimitiveAttribute attribute;
        attribute.primitiveID = primitiveId;
        attribute.primitiveCulled = false;
        sharedPrimitives[primitiveId] = attribute;
    }
    
    // Process vertices
    uint vertexID = threadID;
    if (vertexID < meshlet.vertexCount) {
        MeshToFragment vout;
        vout.positionCS = LoadAndTransformVertex(meshlet, vertexID).positionCS;
        vertices[vertexID] = vout;
    }
}
```

Let's analyze each section to understand how it works.

### Inputs

Mesh shaders offer a flexible approach to processing mesh data similar to compute shaders, allowing developers to optimize data layouts for improved performance.

Unlike traditional pipelines that rely on fixed-function input assemblers and index buffers, mesh shaders enable applications to directly manage how vertex and primitive data are fetched and processed. This flexibility facilitates various optimizations, such as data compression, quantization, or custom memory layouts optimized for specific algorithms.

Typically, each mesh shader workgroup processes a single meshlet, in this course we use a group size of 128 threads, so that's 128 threads per meshlet. This thread group size ensures that each thread processes one triangle.

### Output Primitives

After calling [SetMeshOutputCounts](https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html#setmeshoutputcounts), the next step is to output mesh data for the rasterizer. We begin by processing the primitives. There are two primary types of output data for primitives, denoted by the `out indices` and `out primitives` parameters in the kernel function.

First, the **primitive indices** define the triangles sent to the rasterizer. Since a meshlet has a maximum of 256 triangles, these indices are stored in an 8-bit format relative to the start of the meshlet.

Second, we output per-primitive data that does not get interpolated by the rasterizer. This typically includes `SV_PrimitiveID`, which must be explicitly passed by the mesh shader, as it is not automatically generated. Additionally, `SV_CullPrimitive` enables per-triangle culling, a feature we will explore in later chapters when discussing triangle culling techniques and their benefits.

### Output Vertices

Next, we output the vertices, denoted by the `out vertices` parameter in the kernel function. This process involves loading vertex data from buffers, transforming the vertex positions into clip space, and preparing additional vertex attributes for the fragment shader.

Every piece of data passed in `MeshToFragment` will be interpolated by the rasterizer before being accessed in the fragment shader. This struct is used to store information such as texture coordinates, normals, and tangents.

### Limitations

In DirectX 12, the maximum geometry output per workgroup is:
- **256 vertices**
- **256 triangles**

## Amplification / Task Shaders

The amplification shader is an optional stage in the mesh shader pipeline that runs before the mesh shader. The same `DispatchMesh()` function is used to invoke either the amplification shader followed by the mesh shader or just the mesh shader directly if no amplification shader is present.

The amplification shader is similar to a compute shader but serves a single purpose: dispatching work for the mesh shader. It can either generate additional work, such as performing tessellation, or reduce workload by culling mesh shaders that would not be visible (meshlet culling).

This process is controlled by an intrinsic called [`DispatchMesh`](https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html#dispatchmesh-intrinsic). Each thread of the amplification shader can call this intrinsic to invoke multiple mesh shader workgroups. If you only need one amplification shader workgroup to spawn a single mesh shader workgroup (to process one meshlet), then only one thread of the amplification shader workgroup should call `DispatchMesh` with a value of 1.

### Sending Payload from Amplification to Mesh Shaders

You can pass custom data from the amplification shader to the mesh shader using a payload struct. This is done by declaring an array of structs in `groupshared` memory, allowing each workgroup of the mesh shader to access the data. The array must be large enough to accommodate all group invocations of the mesh shader. The payload array is then passed as an argument to the `DispatchMesh` intrinsic, as shown in the following meshlet culling example:

```c
struct TaskCullingPayload
{
    uint meshletIndex[TASK_THREAD_GROUP_SIZE];
};

groupshared TaskCullingPayload cullingPayload;

[NumThreads(TASK_THREAD_GROUP_SIZE, 1, 1)]
void task(uint threadID : SV_DispatchThreadID, uint groupThreadId : SV_GroupThreadID)
{
    uint visbleMeshletCount = 0;
    bool visible = true;

    // perform meshlet culling
    ...

    if (visible)
        cullingPayload[WavePrefixCountBits(visible)] = threadID;

    DispatchMesh(visbleMeshletCount, 1, 1, cullingPayload);
}

[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void mesh(uint groupID : SV_GroupID, in payload TaskCullingPayload cullingPayload)
{
    uint meshletIndex = cullingPayload.meshletIndex[groupID];
}
```

## Fragment Shaders

While the fragment shader is not exclusive to the mesh shading pipeline (it is also part of the legacy vertex shader path), mesh shaders have a special connection to fragment shaders. They allow driving properties that were previously auto-generated, such as `SV_PrimitiveID`. Additionally, mesh shaders enable the use of an 8-bit index buffer instead of 16-bit or 32-bit, which increases rasterization bandwidth efficiency.

Mesh shaders also support per-triangle culling via `SV_CullPrimitive`, preventing unnecessary triangles from reaching the rasterizer. This additional flexibility helps optimize rendering performance by reducing workload on fixed-pipeline functions, which operate at a fixed rate and can become a bottleneck when the rest of the pipeline is highly optimized.

It's important to note that `SV_PrimitiveID` is not automatically generated when using mesh shaders. Instead, the mesh shader must explicitly pass a custom primitive ID value to the fragment shader through this semantic.

## Implementation

Let's examine an actual implementation of mesh shaders used for rendering a visibility buffer. You can find the source code for this implementation in the [VisibilityPass.hlsl](https://github.com/alelievr/Modern-Rendering-Introduction/blob/master/ModernRenderer/assets/shaders/VisibilityPass.hlsl) file.

### From Mesh to Meshlet

The first step is to convert an arbitrary mesh loaded from a 3D object file into meshlets. We do this immediately after importing the mesh using `meshoptimizer` functions.

First, we use `meshopt_buildMeshletsBound` to determine the number of meshlets the mesh will generate and allocate the buffers accordingly.

Then, we use `meshopt_buildMeshlets` to generate the three meshlet data buffers: the meshlet list, meshlet vertex indices, and meshlet triangles.

Each meshlet in the meshlet list consists of the following structure:

```c
// From meshoptimizer.h
struct meshopt_Meshlet
{
    /* Offsets within meshlet_vertices and meshlet_triangles arrays containing meshlet data */
    unsigned int vertex_offset;
    unsigned int triangle_offset;

    /* Number of vertices and triangles used in the meshlet; data is stored in a consecutive range defined by offset and count */
    unsigned int vertex_count;
    unsigned int triangle_count;
};
```

Lastly, for each meshlet in the list, we call `meshopt_computeMeshletBounds` to build the list of meshlet bounds for culling. Additionally, we use `meshopt_optimizeMeshlet` to reorder the vertices within a single meshlet to optimize locality. If you're interested in learning more about this optimization, check out the [Meshlet Triangle Locality Matters](https://zeux.io/2024/04/09/meshlet-triangle-locality/) post.

### Buffers in HLSL

On the shader side, we end up with five buffers containing the necessary data for meshlet processing:

- A vertex buffer that contains vertex data such as position, normal, texture coordinates, etc.
- A meshlet list describing all the meshlets in a single mesh, using the `meshopt_Meshlet` struct as the layout.
- A meshlet indices buffer. This index buffer is used as an indirection to access vertices, allowing a single vertex to be used in different meshlets.
- A meshlet triangle buffer. Each triangle in the meshlet is represented by three 8-bit values in this buffer, corresponding to relative indices in the meshlet index buffer (relative to the start of the meshlet).
- A meshlet bounds buffer, storing values used for meshlet culling.

### Putting It All Together

When a mesh shader is dispatched from the CPU, we specify the number of meshlets that compose the mesh we want to draw. Because the mesh shader is similar to a compute shader, this number represents the number of workgroups dispatched to the mesh shader. We can retrieve the ID of each workgroup using the `SV_GroupID` semantic in the shader's parameters:

```c
void mesh(uint groupID : SV_GroupID)
```

We use this `groupID` to fetch the current meshlet to process from the meshlet list and directly call `SetMeshOutputCounts` to set the number of output vertices and triangles:

```c
uint meshletIndex = groupID;
meshopt_Meshlet meshlet = meshlets[meshletIndex];
SetMeshOutputCounts(meshlet.vertex_count, meshlet.triangle_count);
```

Next, we process the triangles of the meshlet. For this renderer, I use meshlets with up to 128 triangles and a maximum of 128 vertices. This allows the triangle index to fit within 7 bits for the visibility buffer.

Each thread processes a single triangle and outputs a `uint3` primitive index to the rasterizer by loading it from the meshlet triangle buffer:

```c
uint3 LoadPrimitive(uint offset, uint primitiveId)
{
    return uint3(
        meshletTriangles[offset + primitiveId * 3 + 0],
        meshletTriangles[offset + primitiveId * 3 + 1],
        meshletTriangles[offset + primitiveId * 3 + 2]
    );
}

...

uint primitiveId = threadID;
triangles[primitiveId] = LoadPrimitive(meshlet.triangleOffset, primitiveId);
```

Along with the triangle indices, we prepare the primitive data for the fragment shader of the visibility pass. I chose to encode the visibility data into the `SV_PrimitiveID` semantic:

```c
struct VisibilityPrimitiveAttribute
{
    uint packedVisibilityData : SV_PrimitiveID;
    bool primitiveCulled : SV_CullPrimitive;
};

...

VisibilityPrimitiveAttribute attribute;
attribute.packedVisibilityData = EncodeVisibility(groupID, primitiveId);
attribute.primitiveCulled = false;
sharedPrimitives[primitiveId] = attribute;
```

Finally, we process the vertices. To do this, we load the vertex index from the meshlet indices, retrieve all vertex attributes, and transform them using the instance data (object-to-world matrix):

```c
TransformedVertex LoadVertexAttributes(uint meshletIndex, uint vertexIndex, uint instanceID)
{
    TransformedVertex vout;
    
    // Fetch mesh data from buffers
    VertexData vertex = vertexBuffer.Load(vertexIndex);
    InstanceData instance = LoadInstance(instanceID);
    
    vout.positionOS = vertex.positionOS;
    
    // Apply camera-relative rendering
    vertex.positionOS = GetCameraRelativePosition(vertex.positionOS);
    float3 positionWS = TransformObjectToWorld(vertex.positionOS, instance.objectToWorld);
    
    vout.positionCS = TransformCameraRelativeWorldToHClip(positionWS);
    vout.positionWS = positionWS;
    vout.uv = vertex.uv;
    vout.normal = TransformObjectToWorldNormal(vertex.normal);

    return vout;
}

...

uint vertexIndex = meshletIndices[meshlet.vertexOffset + threadID];
TransformedVertex vertex = LoadVertexAttributes(groupID, vertexIndex, instanceID);
VisibilityMeshToFragment vout;

vout.positionCS = vertex.positionCS;

vertices[threadID] = vout;
```

You can find the full source code in [VisibilityPass.hlsl](https://github.com/alelievr/Modern-Rendering-Introduction/blob/master/ModernRenderer/assets/shaders/VisibilityPass.hlsl) and [MeshUtils.hlsl](https://github.com/alelievr/Modern-Rendering-Introduction/blob/master/ModernRenderer/assets/shaders/MeshUtils.hlsl).

## Conclusion

## References

- 📄 [From Vertex Shader to Mesh Shader - Mesh Shaders on AMD RDNA™ Graphics Cards](https://gpuopen.com/learn/mesh_shaders/mesh_shaders-from_vertex_shader_to_mesh_shader/)
- 📄 [Mesh Shaders - DirectX-Specs](https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html)
- 📄 [Introduction to Turing Mesh Shaders - NVIDIA Technical Blog](https://developer.nvidia.com/blog/introduction-turing-mesh-shaders/)
- 📄 [Optimizing the Graphics Pipeline with Compute - GDC 2016](https://ubm-twvideo01.s3.amazonaws.com/o1/vault/gdc2016/Presentations/Wihlidal_Graham_OptimizingTheGraphics.pdf)
- 📄 [meshoptimizer - Github](https://github.com/zeux/meshoptimizer?tab=readme-ov-file#mesh-shading)
- 🎥 [Mesh Shaders - The Future of Rendering](https://www.youtube.com/watch?v=3EMdMD1PsgY)
