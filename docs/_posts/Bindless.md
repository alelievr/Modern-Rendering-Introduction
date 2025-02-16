# Bindless Bindings

Bindless rendering is an approach that simplifies resource management between the CPU and shaders. To fully understand the benefits of bindless rendering, we first need to explore the concept of bindings.

## Bindings

A shader often needs to read from or write to an external resource such as a texture, a buffer, or a sampler. At the time the shader is executed, all the required resources must be bound (pointing to valid resources created by the CPU). This binding operation is performed on the CPU and requires precise management of which shaders need which resources, thus increasing the CPU cost of the render loop. 

In modern graphics APIs, this cost can be mitigated by preparing objects that contain lists of bindings ahead of time. However, this still involves handling potentially thousands of bindings in a scene with numerous materials.

Let's look at a simple example to illustrate this. Here, we have a shader that reads a texture:

```hlsl
Texture2D<float4> albedoTexture : register(t0, space0);
SamplerState linearRepeatSampler : register(s0, space1);

float4 main(MeshToFragment input) : SV_TARGET
{
    float2 uv = input.uv.xy;
    
    return albedoTexture.SampleLevel(linearRepeatSampler, uv, 0);
}
```

This shader has 2 external resources that need to be bound: `albedoTexture` and `linearRepeatSampler`. We can easily imagine that the albedo texture will change between draw calls if the object's material has a different texture setup.

In this example, the [register](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-variable-register) keyword is used to specify the exact location of the resource. This is necessary because the number of resources you can bind to a single shader is limited (most graphics APIs have a limit of 32 textures and 16 samplers). This location is composed of three components: a letter (here 't' for texture or 's' for sampler), a number following this letter, which is the index of the slot used by the resource, and finally a space index. The space index allows several resources to use the same slot without overlapping if they are in different spaces.

These slots and space indices are directly provided to the graphics API when binding a resource to a shader. To ensure that the indices match between the API and shader declaration, reflection APIs were developed. These allow you to query this information from the compiled shader to avoid hardcoding binding values and mismatches between different shaders.

Then, if we look at the render loop on the CPU with these bindings, we have something like this:

```c++
for (Material material : visibleObjects.GetMaterials())
{
    // Retrieve all meshes associated with the current material
    auto meshes = visibleObjects.GetMeshesWithMaterial(material);

    // Prepare the material's shader and associated render state
    auto shader = material.GetShader();
    PrepareShader(shader); // Set shader, pipeline state, etc.

    // Bind the material's textures
    for (Texture texture : material.GetTextures())
        BindTexture(texture);

    // Bind the material's samplers
    for (Sampler sampler : material.GetSamplers())
        BindSampler(sampler);

    // Bind any constant or uniform buffers associated with the material
    for (Buffer buffer : material.GetBuffers())
        BindBuffer(buffer);

    for (Mesh mesh : meshes)
    {
        // Bind the vertex and index buffers for the mesh
        BindVertexBuffer(mesh.GetVertexBuffer());
        BindIndexBuffer(mesh.GetIndexBuffer());

        // Draw the mesh using the material
        DrawIndexed(mesh.GetIndexCount(), mesh.GetIndexStart(), mesh.GetBaseVertex());
    }
}
```

## Bindless

In a bindless pipeline, resources like textures and buffers are placed in globally accessible arrays or descriptor tables, and shaders reference them via indices. These indices can be passed as constants during draw calls or stored in other GPU-accessible structures, such as buffers. This approach removes the need for binding operations during the render loop, allowing the CPU to issue draw calls with minimal overhead.

> Note that even though this is called "bindless," we still need to "bind" the descriptor table and send the indices to the shader.

Following the previous example, here's the same shader code but using bindless resources instead:

```hlsl
Texture2D<float4> textures[] : register(t, space0);
SamplerState samplers[] : register(s, space1);

// Passed through push constants / root signature constants
cbuffer MaterialData
{
    uint textureIndex;
    uint samplerIndex;
};

float4 main(MeshToFragment input) : SV_TARGET
{
    float2 uv = input.uv.xy;

    // Access the texture and sampler dynamically using indices
    return textures[textureIndex].SampleLevel(samplers[samplerIndex], uv, 0);
}
```

Note that the slot index in this example is omitted from the register declaration. This is because the entire space is used to store textures, and the same applies to the sampler array. This approach is advantageous because it eliminates the limitation on the number of resources that can be bound to a shader, which we had previously.

The indices are sent in a constant buffer, which is patched directly from the CPU using a [push constant](https://vkguide.dev/docs/chapter-3/push_constants/) or a [root constant](https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-constants-directly-in-the-root-signature). These updates are very fast compared to other methods of uploading data to the GPU, making them suitable for use within the render loop.

Now, if we examine the updated render loop, it would look something like this, using indices in arrays instead of manually passing each resource to the shader or material.

```c++
// Bind global descriptor tables containing textures, samplers, and buffers
BindGlobalResources(); // This should be done once and then updated only if needed

for (RenderObject object : visibleObjects)
{
    PushConstants pushConstants;
    pushConstants.materialIndex = object.materialIndex;      // Index into material data buffer
    pushConstants.vertexBufferIndex = object.vertexBufferIndex; // Index into vertex buffer
    pushConstants.indexBufferIndex = object.indexBufferIndex;   // Index into index buffer
    pushConstants.transformIndex = object.transformIndex;       // Index into transform buffer

    UploadPushConstants(pushConstants);

    DrawIndexed(object.indexCount, object.indexStart, object.baseVertex);
}
```

In this example, every resource accessed by the shader uses bindless rendering. Therefore, all data must be prepared in advance and stored in large buffers or arrays of resources. These buffers can then be partially updated if needed when loading new assets into the scene.

As you can see, this CPU render loop is much simpler and requires far fewer graphics API calls, making it significantly faster.

## Mixing Both APIs

In real applications, it is much more common to use a mix of both APIs. For example, bindless rendering is excellent for textures as it removes the limit on the number of textures that can be bound to a shader. However, for other types of data, such as transform data, it is unnecessary because this data can already be passed as a buffer with an index using a regular buffer.

There are also some performance considerations to be mindful of when using bindless rendering on the GPU. Due to the extra indirection involved in accessing resources, memory load operations can be slower, especially if the index is vectorial.

## Conclusion

Bindless rendering represents a significant evolution in how modern graphics pipelines handle resource management. By leveraging global descriptor tables or arrays and eliminating the need for frequent binding operations, bindless approaches allow developers to improve the efficiency and flexibility of the rendering loop. The flexibility of dynamically accessing resources in shaders lifts previous limitations, such as the strict cap on the number of bound textures.

While bindless rendering introduces minor GPU overhead due to dynamic access and memory indirection, its benefits in outweigh these costs. Many real-world applications combine bindless and traditional bindings to strike an optimal balance between flexibility and efficiency.

## References

- 📄 [Resource binding in HLSL - Win32 apps | Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/direct3d12/resource-binding-in-hlsl)
- 📄 [Bindless Graphics Tutorial | NVIDIA](https://www.nvidia.com/en-us/drivers/bindless-graphics/)
- 📄 [Setting up a bindless rendering pipeline - Vulkanised 2023](https://vulkan.org/user/pages/09.events/vulkanised-2023/vulkanised_2023_setting_up_a_bindless_rendering_pipeline.pdf)
- 📄 [Bindless Descriptors – Wicked Engine](https://wickedengine.net/2021/04/bindless-descriptors/)
