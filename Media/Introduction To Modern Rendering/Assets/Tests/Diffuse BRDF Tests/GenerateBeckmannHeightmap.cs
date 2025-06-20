using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Experimental.Rendering;

// Based on Generating Procedural Beckmann Surfaces - Eric Heitz
// https://drive.google.com/file/d/0BzvWIdpUpRx_U1NOUjlINmljQzg/view?resourcekey=0-v4_BEatN1wDHRTnTACcIOQ
public class GenerateBeckmannHeightmap : MonoBehaviour
{
    [Header("Height Field Settings")]
    public float alphaX = 0.5f;
    public float alphaY = 0.5f;
    public int numWaves = 1000;

    [Header("Output")]
    public RenderTexture heightTexture;
    public ComputeShader beckmannCS;

    GraphicsBuffer beckmannBuffer;

    float lastAlphaX = -1.0f;
    float lastAlphaY = -1.0f;
    int lastNumWaves = -1;

    struct Wave
    {
        public float phi; // phase
        public float fx; // frequency in x direction
        public float fy; // frequency in y direction
    }

    void Start()
    {
        InitRenderTexture();
    }

    void InitRenderTexture()
    {
        if (heightTexture == null)
            throw new System.Exception("Height texture is not assigned.");

        if (heightTexture.enableRandomWrite != true)
        {
            heightTexture.Release();
            heightTexture.graphicsFormat = GraphicsFormat.R32_SFloat;
            heightTexture.enableRandomWrite = true;
            heightTexture.Create();
        }
    }

    List<Wave> GenerateWaves()
    {
        var waves = new List<Wave>();
        waves.Capacity = numWaves;

        var rand = new System.Random(42);
        for (int i = 0; i < numWaves; i++)
        {
            float U1 = (float)rand.NextDouble();
            float U2 = (float)rand.NextDouble();
            float U3 = (float)rand.NextDouble();

            float phi = 2.0f * Mathf.PI * U1;
            float theta = 2.0f * Mathf.PI * U2;
            float r = Mathf.Sqrt(-Mathf.Log(1.0f - U3));

            Wave w;
            w.phi = phi;
            w.fx = r * Mathf.Cos(theta) * alphaX;
            w.fy = r * Mathf.Sin(theta) * alphaY;

            waves.Add(w);
        }

        return waves;
    }

    void DispatchCompute()
    {
        beckmannCS.SetInt("_Resolution", heightTexture.width);
        beckmannCS.SetInt("_WaveCount", numWaves);
        beckmannCS.SetFloat("_Scale", Mathf.Sqrt(2.0f / numWaves));
        beckmannCS.SetBuffer(0, "_Waves", beckmannBuffer);
        beckmannCS.SetTexture(0, "_Result", heightTexture);

        int groups = Mathf.CeilToInt(heightTexture.width / 8.0f);
        beckmannCS.Dispatch(0, groups, groups, 1);
    }

    void Update()
    {
        if (beckmannBuffer == null || beckmannBuffer.count != numWaves)
        {
            beckmannBuffer?.Release();
            beckmannBuffer = new GraphicsBuffer(GraphicsBuffer.Target.Structured, numWaves, sizeof(float) * 3);
            var waves = GenerateWaves();
            beckmannBuffer.SetData(waves);
        }

        if (lastAlphaX != alphaX || lastAlphaY != alphaY || lastNumWaves != numWaves)
        {
            lastAlphaX = alphaX;
            lastAlphaY = alphaY;
            lastNumWaves = numWaves;

            var waves = GenerateWaves();
            beckmannBuffer.SetData(waves);

            DispatchCompute();
        }
    }

    void OnDestroy()
    {
        beckmannBuffer?.Release();
    }
}
