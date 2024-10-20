using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Unity.VisualScripting;
using TMPro;
using UnityEngine.Animations;

public class Matrix04FrustumToBox : MonoBehaviour
{
    public float orthographic;
    public RenderTexture output;
    public float lineWidth = 0.1f;
    public float time = 4;
    public bool animate = false;
    public float animatedTime = 0;
    // public TextMeshProUGUI text;

    public float fov;
    public float near;
    public float far;

    Line[] lines = new Line[12];

    internal Camera cam;
    internal float t;
    internal Matrix4x4 projectionMatrix;
    internal Vector3[] nearCorners = new Vector3[4];
    internal Vector3[] farCorners = new Vector3[4];

    Vector3[] nearHCLIPCorners = new Vector3[4];
    Vector3[] farHCLIPCorners = new Vector3[4];

    internal Vector3[] intermediateNearCorners;
    internal Vector3[] intermediateFarCorners;

    void Start()
    {
        for (int i = 0; i < lines.Length; i++)
        {
            lines[i] = Instantiate(Resources.Load("Line")).GetComponent<Line>();
            lines[i].transform.SetParent(transform);
            lines[i].color = Color.white;
            lines[i].width = 0.01f;
            lines[i].dotted = false;
            lines[i].gameObject.layer = gameObject.layer;
            lines[i].width = lineWidth; 
        }

        cam = gameObject.AddComponent<Camera>();
        cam.backgroundColor = Color.black;
        cam.clearFlags = CameraClearFlags.SolidColor;
        cam.cullingMask &= ~(1 << gameObject.layer);
        cam.targetTexture = output;
        cam.enabled = false;
        projectionMatrix = Matrix4x4.Perspective(fov, 1, near, far);
        cam.projectionMatrix = projectionMatrix;

        // OpenGL...
        projectionMatrix[2, 3] *= -1;
        projectionMatrix[0, 0] *= -1;
        projectionMatrix[1, 1] *= -1;

        // Compute the 8 points of the frustum:
        cam.CalculateFrustumCorners(new Rect(0, 0, 1, 1), far, Camera.MonoOrStereoscopicEye.Mono, farCorners);
        cam.CalculateFrustumCorners(new Rect(0, 0, 1, 1), near, Camera.MonoOrStereoscopicEye.Mono, nearCorners);

        // Compute the 8 points of the frustum in HCLIP space:
        for (int i = 0; i < 4; i++)
        {
            nearHCLIPCorners[i] = projectionMatrix.MultiplyPoint(nearCorners[i]);
            farHCLIPCorners[i] = projectionMatrix.MultiplyPoint(farCorners[i]);
        }

        // DirectX clips every depth < 0
        for (int i = 0; i < 4; i++)
            nearHCLIPCorners[i] = new Vector3(nearHCLIPCorners[i].x, nearHCLIPCorners[i].y, Mathf.Max(nearHCLIPCorners[i].z, 0));

        Update();
    }

    void Update()
    {

        intermediateNearCorners = new Vector3[4];
        intermediateFarCorners = new Vector3[4];

        if (animate)
            t = animatedTime;
        else
            t = Mathf.PingPong(Time.time / time, 1);
        // Interpolate between projection and HCLIP corners for animation:
        for (int i = 0; i < 4; i++)
        {
            intermediateNearCorners[i] = Vector3.Lerp(nearCorners[i], nearHCLIPCorners[i], t);
            intermediateFarCorners[i] = Vector3.Lerp(farCorners[i], farHCLIPCorners[i], t);
        }

        // Update line positions
        lines[0].UpdateLine(intermediateNearCorners[0], intermediateNearCorners[1]);
        lines[1].UpdateLine(intermediateNearCorners[1], intermediateNearCorners[2]);
        lines[2].UpdateLine(intermediateNearCorners[2], intermediateNearCorners[3]);
        lines[3].UpdateLine(intermediateNearCorners[3], intermediateNearCorners[0]);
        lines[4].UpdateLine(intermediateFarCorners[0], intermediateFarCorners[1]);
        lines[5].UpdateLine(intermediateFarCorners[1], intermediateFarCorners[2]);
        lines[6].UpdateLine(intermediateFarCorners[2], intermediateFarCorners[3]);
        lines[7].UpdateLine(intermediateFarCorners[3], intermediateFarCorners[0]);
        lines[8].UpdateLine(intermediateNearCorners[0], intermediateFarCorners[0]);
        lines[9].UpdateLine(intermediateNearCorners[1], intermediateFarCorners[1]);
        lines[10].UpdateLine(intermediateNearCorners[2], intermediateFarCorners[2]);
        lines[11].UpdateLine(intermediateNearCorners[3], intermediateFarCorners[3]);
    }
}
