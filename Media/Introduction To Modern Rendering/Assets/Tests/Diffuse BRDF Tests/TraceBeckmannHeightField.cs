using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Experimental.Rendering;
using Unity.Collections;
using UnityEditor;

public class TraceBeckmannHeightField : MonoBehaviour
{
    [Header("Compute Shader & Textures")]
    public ComputeShader traceCS;
    public ComputeShader normalizeCS;
    public RenderTexture beckmannHeightField; // your 2D height field
    public RenderTexture outputLUT;         // 3D LUT: width=N·L, height=L·V, depth=roughness

    public float lightAngleDegrees = 45.0f; // angle of the incident light in degrees

    [Header("Tracing Settings")]
    [Range(1, 10)] public int maxBounces = 3;

    public int tracingResolution = 256;
    // This is the number of ray we launch to accumulate a single dimension of the LUT per pixel.
    public int rayCountPerDimension = 4;

    [Header("Debug")]
    public RenderTexture debugTrace;
    public Material debugLinesMaterial;
    public bool forceRoughness = false;
    public float forceRoughnessValue = 0.5f;

    GraphicsBuffer debugLinesArgs;
    GraphicsBuffer debugLinesData;
    RenderTexture normalizedLUT;

    GenerateBeckmannHeightmap beckmannHeightmap;

    void OnEnable()
    {
        debugLinesArgs = new GraphicsBuffer(GraphicsBuffer.Target.IndirectArguments, 1, GraphicsBuffer.IndirectDrawArgs.size);
        debugLinesArgs.SetData(new uint[] { 2, 0, 0, 0 });

        normalizedLUT = new RenderTexture(outputLUT.width, outputLUT.height, 0, GraphicsFormat.R32_SFloat);
        normalizedLUT.volumeDepth = outputLUT.volumeDepth; 
        normalizedLUT.dimension = TextureDimension.Tex3D;
        normalizedLUT.enableRandomWrite = true;
        normalizedLUT.Create();

        debugLinesData = new GraphicsBuffer(GraphicsBuffer.Target.Structured, 65536, sizeof(float) * 10);

        DispatchClearLUT();

        beckmannHeightmap = FindObjectsByType<GenerateBeckmannHeightmap>(FindObjectsSortMode.None)[0];

        StartCoroutine(GenerateLUT());
    }

    void OnDisable()
    {
        if (debugLinesArgs != null)
        {
            debugLinesArgs.Release();
            debugLinesArgs = null;
        }

        normalizedLUT?.Release();
    }


    IEnumerator GenerateLUT()
    {
        // for (int x = 0; x < outputLUT.width; x++)
        int x = 0;
        {
            float roughness = Mathf.Lerp(0, 1.0f, x / (float)outputLUT.width) + 0.5f / outputLUT.width;

            if (forceRoughness)
                roughness = forceRoughnessValue;

            // Isotropic roughness as we have only 1 dimensions reserved for roughness
            beckmannHeightmap.alphaX = beckmannHeightmap.alphaY = roughness;

            for (int y = 0; y < outputLUT.height; y++)
            {
                float ndotl = Mathf.Lerp(0, 1, y / (float)outputLUT.width);
                ndotl += 0.5f / outputLUT.width; // evaluate the center pixel of the LUT

                // convert back to degrees
                lightAngleDegrees = Mathf.Acos(ndotl) * Mathf.Rad2Deg;
                for (int r = 0; r < rayCountPerDimension; r++)
                {
                    DispatchClearDebug();
                    DispatchTrace();
                    yield return new WaitForEndOfFrame();
                }

                yield return new WaitForEndOfFrame();
            }

            Debug.Log("Progress: " + (x + 1) + "/" + outputLUT.width);
        }

        normalizeCS.SetTexture(0, "_Input", outputLUT);
        normalizeCS.SetTexture(0, "_Output", normalizedLUT);
        normalizeCS.SetFloat("_HitCountPerPixel", tracingResolution * tracingResolution * rayCountPerDimension);
        normalizeCS.Dispatch(0, Mathf.CeilToInt(outputLUT.width / 8.0f), Mathf.CeilToInt(outputLUT.height / 8.0f), Mathf.CeilToInt(outputLUT.volumeDepth));

        // TODO: readback the LUT and write it to a new 3D texture asset
        var req = AsyncGPUReadback.Request(normalizedLUT, 0, (request) =>
        {
            Texture3D texture = new Texture3D(normalizedLUT.width, normalizedLUT.height, normalizedLUT.volumeDepth, GraphicsFormat.R32_SFloat, TextureCreationFlags.None);
            if (request.hasError)
            {
                Debug.LogError("Error reading back the LUT texture.");
            }
            else
            {
                int sliceSize = normalizedLUT.width * normalizedLUT.height;
                NativeArray<float> textureData = new NativeArray<float>(sliceSize * normalizedLUT.volumeDepth, Allocator.Temp);

                for (int z = 0; z < normalizedLUT.volumeDepth; z++)
                {
                    var slice = request.GetData<float>(z);
                    var targetSlice = new NativeSlice<float>(textureData, z * sliceSize, sliceSize);
                    targetSlice.CopyFrom(slice);
                }

                texture.SetPixelData(textureData, 0);
                texture.Apply(false, false);

                AssetDatabase.CreateAsset(texture, "Assets/Tests/BakedIsotropicDiffuseBRDF.asset");
            }
        });

        req.WaitForCompletion();
    }

    void DispatchTrace()
    {
        // bind resources
        traceCS.SetTexture(0, "_HeightField", beckmannHeightField);
        traceCS.SetTexture(0, "_OutputLUT", outputLUT);
        traceCS.SetTexture(0, "_DebugTrace", debugTrace);
        traceCS.SetBuffer(0, "_DebugLines", debugLinesData);
        traceCS.SetBuffer(0, "_IndirectLineDrawArgs", debugLinesArgs);
        traceCS.SetFloat("_Roughness", beckmannHeightmap.alphaX);

        traceCS.SetInt("_MaxBounces", maxBounces);
        traceCS.SetInt("_Seed", Time.frameCount);

        // The output LUT stores N.L in X, L.V in Y, and roughness in Z
        traceCS.SetVector("_LUTSize", new Vector3(outputLUT.width, outputLUT.height, outputLUT.volumeDepth));

        // The light direction don't really matter as long as we have the same N.L value for all the rays
        Vector3 lightDir = Quaternion.Euler(0, 0, -lightAngleDegrees) * Vector3.up;
        // Debug.DrawLine(Vector3.zero, lightDir * 2, Color.yellow, 2.0f);

        // Assume _RayDir and N are known directions
        Vector3 rayDir = -lightDir.normalized;
        Vector3 normal = Vector3.up;

        // Build the tangent frame
        Vector3 up = Vector3.Cross(Vector3.forward, rayDir).normalized;
        Vector3 right = Vector3.Cross(rayDir, up).normalized;

        // int gridSize = 10;
        // for (int y = 0; y < gridSize; y++)
        // {
        //     for (int x = 0; x < gridSize; x++)
        //     {
        //         float u = (x + 0.5f) / gridSize;
        //         float v = (y + 0.5f) / gridSize;

        //         u -= 0.5f;
        //         v -= 0.5f;

        //         Vector3 rayOrigin = -rayDir + right * right.z * u + up * up.x * v;
        //         // Ray-plane intersection at plane defined by normal N and passing through origin
        //         float denom = Vector3.Dot(normal, rayDir);
        //         if (Mathf.Abs(denom) > 1e-5f)
        //         {
        //             float t = -Vector3.Dot(normal, rayOrigin) / denom;
        //             Vector3 pos = rayOrigin + (t - 0.01f) * rayDir;

        //             Debug.DrawLine(rayOrigin, pos, Color.green, 2.0f); // Ray segment
        //             Debug.DrawRay(pos, rayDir * 0.5f, Color.red, 2.0f); // Final direction indicator
        //         }
        //     }
        // }

        traceCS.SetVector("_RayDir", rayDir);
        traceCS.SetVector("_DebugCursorPosition", Input.mousePosition);

        traceCS.Dispatch(0, Mathf.CeilToInt(tracingResolution / 8.0f), Mathf.CeilToInt(tracingResolution / 8.0f), 1);

        var rp = new RenderParams
        {
            layer = 0,
            renderingLayerMask = RenderingLayerMask.defaultRenderingLayerMask,
            rendererPriority = 0,
            worldBounds = new Bounds(Vector3.zero, Vector3.one * 1000.0f),
            camera = null,
            motionVectorMode = MotionVectorGenerationMode.Camera,
            reflectionProbeUsage = ReflectionProbeUsage.Off,
            material = debugLinesMaterial,
            shadowCastingMode = ShadowCastingMode.Off,
            receiveShadows = false,
            lightProbeUsage = LightProbeUsage.Off,
            lightProbeProxyVolume = null,
        };
        Shader.SetGlobalBuffer("_DebugLines", debugLinesData);
        Graphics.RenderPrimitivesIndirect(rp, MeshTopology.Lines, debugLinesArgs, 1, 0);
    }

    void DispatchClearLUT()
    {
        // Clear the output LUT
        traceCS.SetTexture(1, "_OutputLUT", outputLUT);
        traceCS.Dispatch(1, Mathf.CeilToInt(outputLUT.width / 8.0f), Mathf.CeilToInt(outputLUT.height / 8.0f), Mathf.CeilToInt(outputLUT.volumeDepth / 8.0f));
    }

    void DispatchClearDebug()
    {
        traceCS.SetBuffer(2, "_IndirectLineDrawArgs", debugLinesArgs);
        traceCS.Dispatch(2, 1, 1, 1);
    }
}
