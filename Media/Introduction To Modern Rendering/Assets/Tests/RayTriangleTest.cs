using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
using Random = UnityEngine.Random;

public class RayTriangleTest : MonoBehaviour
{
    public ComputeShader cs1;
    public ComputeShader cs2;
    public RenderTexture rt;

    public Vector3 v0, v1, v2;
    public int triangleCount = 100;
    public float randomRadius = 1;

    public bool useCS1 = true;
    public bool useBoth = false;

    GraphicsBuffer buffer;
    
    void Update()
    {
        var triangles = new Vector3[triangleCount * 3];
        for (int i = 0; i < triangleCount; i++)
        {
            int index = i * 3;
            triangles[index + 0] = v0 + Random.insideUnitSphere * randomRadius;
            triangles[index + 1] = v1 + Random.insideUnitSphere * randomRadius;
            triangles[index + 2] = v2 + Random.insideUnitSphere * randomRadius;
        }
        buffer.SetData(triangles);

        if (useCS1 || useBoth)
            DispatchCompute(cs1, "Fast Triangle Intersection");
        if (!useCS1 || useBoth)
            DispatchCompute(cs2, "Moller-Trumbore Intersection");
    }

    void OnEnable()
    {
        buffer = new GraphicsBuffer(GraphicsBuffer.Target.Structured, GraphicsBuffer.UsageFlags.None, triangleCount, sizeof(float) * 9);
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
        cmd.SetComputeBufferParam(cs, 0, "_TriangleVertices", buffer);
        cmd.SetComputeIntParam(cs, "_TriangleCount", triangleCount);
        cmd.SetComputeVectorParam(cs, "_RayOrigin", Vector4.zero);

        cmd.DispatchCompute(cs, 0, rt.width / 8, rt.height / 8, 1);

        Graphics.ExecuteCommandBuffer(cmd);
        cmd.Release();
    }
}
