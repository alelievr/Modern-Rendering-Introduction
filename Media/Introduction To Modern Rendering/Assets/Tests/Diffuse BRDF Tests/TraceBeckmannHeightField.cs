using UnityEngine;
using UnityEngine.Rendering;

public class TraceBeckmannHeightField : MonoBehaviour
{
    [Header("Compute Shader & Textures")]
    public ComputeShader traceCS;
    public RenderTexture beckmannHeightField; // your 2D height field
    public RenderTexture outputLUT;         // 3D LUT: width=N·L, height=L·V, depth=roughness

    public float lightAngleDegrees = 45.0f; // angle of the incident light in degrees

    [Header("Tracing Settings")]
    [Range(1, 10)] public int maxBounces = 3;

    [Header("Debug")]
    public RenderTexture debugTrace;
    public Material debugLinesMaterial;

    GraphicsBuffer debugLinesArgs;
    GraphicsBuffer debugLinesData;

    void OnEnable()
    {
        debugLinesArgs = new GraphicsBuffer(GraphicsBuffer.Target.IndirectArguments, 1, GraphicsBuffer.IndirectDrawArgs.size);
        debugLinesArgs.SetData(new uint[] { 2, 0, 0, 0 });

        debugLinesData = new GraphicsBuffer(GraphicsBuffer.Target.Structured, 65536, sizeof(float) * 10);
    }

    void OnDisable()
    {
        if (debugLinesArgs != null)
        {
            debugLinesArgs.Release();
            debugLinesArgs = null;
        }
    }

    void Update()
    {
        DispatchClearLUT();

        DispatchTrace();
    }

    void DispatchTrace()
    {
        // bind resources
        traceCS.SetTexture(0, "_HeightField", beckmannHeightField);
        traceCS.SetTexture(0, "_OutputLUT", outputLUT);
        traceCS.SetTexture(0, "_DebugTrace", debugTrace);
        traceCS.SetBuffer(0, "_DebugLines", debugLinesData);
        traceCS.SetBuffer(0, "_IndirectLineDrawArgs", debugLinesArgs);

        traceCS.SetInt("_MaxBounces", maxBounces);

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

        traceCS.Dispatch(0, Mathf.CeilToInt(beckmannHeightField.height / 8.0f), Mathf.CeilToInt(beckmannHeightField.width / 8.0f), 1);

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
        traceCS.SetBuffer(1, "_IndirectLineDrawArgs", debugLinesArgs);
        traceCS.Dispatch(1, Mathf.CeilToInt(outputLUT.width / 8.0f), Mathf.CeilToInt(outputLUT.height / 8.0f), Mathf.CeilToInt(outputLUT.volumeDepth / 8.0f));
    }
}
