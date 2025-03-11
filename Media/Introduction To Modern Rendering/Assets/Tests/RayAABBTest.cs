using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
using Random = UnityEngine.Random;
using static Unity.Mathematics.math;
using Unity.Mathematics;

public class RayAABBTest : MonoBehaviour
{
    public ComputeShader cs1;
    public ComputeShader cs2;
    public RenderTexture rt;

    public Vector3 offset;
    public int aabbCount = 100;
    public float randomRadius = 1;
    public float scale = 1;

    public bool useCS1 = true;
    public bool useBoth = false;

    GraphicsBuffer buffer;
    
    void Update()
    {
        Random.InitState(42);
        var aabbs = new Vector3[aabbCount * 2];
        for (int i = 0; i < aabbCount; i++)
        {
            int index = i * 2;
            var center = (float3)Random.insideUnitSphere * randomRadius;
            center += float3(0, 0, randomRadius) + float3(offset);
            var halfSize = abs(Random.insideUnitSphere) + 0.1f;
            halfSize *= scale;
            aabbs[index + 0] = center - halfSize; // Min
            aabbs[index + 1] = center + halfSize; // Max
        }
        buffer.SetData(aabbs);

        if (useCS1 || useBoth)
            DispatchCompute(cs1, "Fast AABB Intersection");
        if (!useCS1 || useBoth)
            DispatchCompute(cs2, "Branchless AABB Intersection");
    }

    void OnEnable()
    {
        buffer = new GraphicsBuffer(GraphicsBuffer.Target.Structured, GraphicsBuffer.UsageFlags.None, aabbCount, sizeof(float) * 6);
    }

    void OnDisable()
    {
        buffer.Release();
    }

    public void DispatchCompute(ComputeShader cs, string name)
    {
        var cmd = new CommandBuffer();
        cmd.name = name;

        cmd.SetRenderTarget(rt);
        cmd.ClearRenderTarget(true, true, Color.black);
        cmd.SetComputeTextureParam(cs, 0, "_Output", rt);
        cmd.SetComputeBufferParam(cs, 0, "_AABBs", buffer);
        cmd.SetComputeIntParam(cs, "_AABBCount", aabbCount);
        cmd.SetComputeVectorParam(cs, "_RayOrigin", Vector4.zero);

        cmd.DispatchCompute(cs, 0, rt.width / 8, rt.height / 8, 1);

        Graphics.ExecuteCommandBuffer(cmd);
        cmd.Release();
    }
}
