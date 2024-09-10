using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

public class Tests : MonoBehaviour
{
    public ComputeShader shader;
    public RenderTexture renderTexture;

    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        var cmd = new CommandBuffer() { name = "Test" };
        cmd.SetComputeTextureParam(shader, 0, "_Output", renderTexture);
        var inverseViewProj = Camera.main.projectionMatrix * Camera.main.worldToCameraMatrix;
        inverseViewProj = inverseViewProj.inverse;
        cmd.SetComputeMatrixParam(shader, "inverseViewMatrix", inverseViewProj);
        cmd.DispatchCompute(shader, 0, renderTexture.width / 8, renderTexture.height / 8, 1);
        Graphics.ExecuteCommandBuffer(cmd);
        cmd.Dispose();
    }
}
