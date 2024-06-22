using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

public class RayTriangleTest : MonoBehaviour
{
    public ComputeShader cs1;
    public ComputeShader cs2;
    public RenderTexture rt;

    public Vector3 v0, v1, v2;

    public bool useCS1 = true;
    public bool useBoth = false;

    void Update()
    {
        if (useCS1 || useBoth)
            DispatchCompute(cs1, "Fast Triangle Intersection");
        if (!useCS1 || useBoth)
            DispatchCompute(cs2, "Moller-Trumbore Intersection");
    }

    public void DispatchCompute(ComputeShader cs, string name)
    {
        var cmd = new CommandBuffer();
        cmd.name = name;

        cmd.SetRenderTarget(rt);
        cmd.ClearRenderTarget(true, true, Color.black);
        cmd.SetComputeTextureParam(cs, 0, "_Output", rt);
        cmd.SetComputeVectorParam(cs, "_V0", v0);
        cmd.SetComputeVectorParam(cs, "_V1", v1);
        cmd.SetComputeVectorParam(cs, "_V2", v2);

        cmd.DispatchCompute(cs, 0, rt.width / 8, rt.height / 8, 1);

        Graphics.ExecuteCommandBuffer(cmd);
        cmd.Release();
    }
}
