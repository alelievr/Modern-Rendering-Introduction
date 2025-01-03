# Bindless Bindings

Bindless rendering is an approach that simplifies the resource management between CPU and shaders. To appreciate the benefits of bindless rendering, we first have to look at what is a binding.

## Bindings

A shader often requires to read or write from an external resource such as a texture, a buffer or a sampler. At the time where the shader is rendered, all the resources need to be bound (pointing to a valid resource created by the CPU). This binding operation is executed on the CPU and require fine management of which shader need which resource, thus increasing the CPU cost of the render loop. In modern graphics API this cost can be alleviated by preparing objects that contain a list of bindings ahead of time, but it still mean handling possibly thousands of them if there is a scene with lots of materials.

Let's see a simple example to demonstrate this, we have a shader that read a texture:

```hlsl
Texture2D<float4> albedoTexture : register(t0, space0);
SamplerState linearRepeatSampler : register(s0, space1);

float4 main(MeshToFragment input) : SV_TARGET
{
    float2 uv = input.uv.xy;
    
    return albedoTexture.SampleLevel(linearRepeatSampler, uv, 0);
}
```

This shader has 2 external resources that need to be bound: `albedoTexture` and `linearRepeatSampler`. We can easily imagine that the albedo texture will be changing between draw calls if the object material has a different texture set up.

In this example, the [register](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-variable-register) keyword is used to specify the exact location of the resource, this is necessary because the amount of resources you can bind to a single shader is limited (most of graphics APIs have a limit of 32 textures and 16 samplers). This location is composed of 3 components: a letter (here 't' for texture or 's' for sampler), a number following this letter which is the index of the slot used by the resource and finally a space index which allows several resources to use the same slot without overlapping if they use a different space.

These slots and space indices are directly provided to the graphics API when binding a resource to a shader. To make sure that the indices match between the API and shader declaration, reflection APIs were developed that allows to query this information from the compiled shader to avoid hardcoding binding values and mismatching between different shaders.

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

> Note that even though this is called bindless, we still need to "bind" the descriptor table and send the indices to the shader.

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

Note that the slot index in this example is gone from the register declaration, this is because the whole space is used to store textures and the same is true for the sampler array. This is great because it allows to get rid of the number of resources bound to a shader limitation we had previously.

The indices are sent over in a constant buffer, which is patched directly from the CPU using a [push constant](https://vkguide.dev/docs/chapter-3/push_constants/) / [root constant](https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-constants-directly-in-the-root-signature), these updates are very fast compared to other ways of uploading data to the GPU so it's okay to use inside the render loop.

Now if we look at the new render loop, it'd be something like this using indices in arrays instead of manually passing each resource to the shader/material.

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

In this example, every resources access by the shader is using bindless, thus every data has to be prepared in advance into large buffers storing or array of resources. These buffers can then be partially updated if needed when loading new assets in the scene. 

As you can see this CPU render loop is much more simple and require a lot less graphics API calls making it significantly faster.

## Mixing both APIs

In real applications it's much more common to use a mix of both APIs, for example bindless is amazing for textures as it lifts the limit of textures bound to a shader. But for other type of data such as transform data, it's unnecessary because we can already pass it as a buffer with an index using a regular buffer.

There are also some performance consideration to be careful with bindless on the GPU, because of the extra indirection when accessing the resources, memory load operation can be slower, especially if the index is vectorial.

## Conclusion

Bindless rendering represents a significant evolution in how modern graphics pipelines handle resource management. By leveraging global descriptor tables or arrays and eliminating the need for frequent binding operations, bindless approaches allow developers to improve the efficiency and flexibility of the rendering loop. The flexibility of dynamically accessing resources in shaders lifts previous limitations, such as the strict cap on the number of bound textures.

While bindless rendering introduces minor GPU overhead due to dynamic access and memory indirection, its benefits in outweigh these costs. Many real-world applications combine bindless and traditional bindings to strike an optimal balance between flexibility and efficiency.

## References

https://learn.microsoft.com/en-us/windows/win32/direct3d12/resource-binding-in-hlsl

https://www.nvidia.com/en-us/drivers/bindless-graphics/

https://vulkan.org/user/pages/09.events/vulkanised-2023/vulkanised_2023_setting_up_a_bindless_rendering_pipeline.pdf
