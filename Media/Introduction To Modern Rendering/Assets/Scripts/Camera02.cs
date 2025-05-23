using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Unity.VisualScripting;
using TMPro;

public class Camera02 : MonoBehaviour
{
    public float orthographic;
    public RenderTexture output;
    public TextMeshProUGUI text;

    [Range(0, 1)]
    public float f = 0;

    public float fov;
    public float near;
    public float far;

    Line[] lines = new Line[12];

    Camera cam;
    
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
        }

        cam = gameObject.AddComponent<Camera>();
        cam.backgroundColor = Color.black;
        cam.clearFlags = CameraClearFlags.SolidColor;
        cam.cullingMask &= ~(1 << gameObject.layer);
        cam.targetTexture = output;

        Update();
    }

    void Update()
    {
        cam.fieldOfView = fov;
        cam.nearClipPlane = near;
        cam.farClipPlane = far;

        // Compute the 8 points of the frustum:
        var farCorners = new Vector3[4];
        cam.CalculateFrustumCorners(new Rect(0, 0, 1, 1), cam.farClipPlane, Camera.MonoOrStereoscopicEye.Mono, farCorners);
        var nearCorners = new Vector3[4];
        cam.CalculateFrustumCorners(new Rect(0, 0, 1, 1), cam.nearClipPlane, Camera.MonoOrStereoscopicEye.Mono, nearCorners);

        float nearPlaneSize = nearCorners[0].x - nearCorners[2].x;
        float farPlaneSize = farCorners[0].x - farCorners[2].x;

        // Transform points using local matrix
        for (int i = 0; i < 4; i++)
        {
            farCorners[i] = transform.TransformPoint(farCorners[i]);
            nearCorners[i] = transform.TransformPoint(nearCorners[i]);

            farCorners[i] = Vector3.Lerp(farCorners[i], nearCorners[i], f);
        }

        var proj = GL.GetGPUProjectionMatrix(cam.projectionMatrix, false);
        Shader.SetGlobalFloat("_ProjectionViewSlider", f);
        Shader.SetGlobalMatrix("_ViewProjectionMatrix", proj * cam.worldToCameraMatrix);
        Shader.SetGlobalMatrix("_InverseViewMatrix", cam.worldToCameraMatrix.inverse);
        Shader.SetGlobalFloat("_NearPlane", cam.nearClipPlane);
        Shader.SetGlobalFloat("_NearPlaneSize", nearPlaneSize);

        // Update line positions
        lines[0].UpdateLine(nearCorners[0], nearCorners[1]);
        lines[1].UpdateLine(nearCorners[1], nearCorners[2]);
        lines[2].UpdateLine(nearCorners[2], nearCorners[3]);
        lines[3].UpdateLine(nearCorners[3], nearCorners[0]);
        lines[4].UpdateLine(farCorners[0], farCorners[1]);
        lines[5].UpdateLine(farCorners[1], farCorners[2]);
        lines[6].UpdateLine(farCorners[2], farCorners[3]);
        lines[7].UpdateLine(farCorners[3], farCorners[0]);
        lines[8].UpdateLine(nearCorners[0], farCorners[0]);
        lines[9].UpdateLine(nearCorners[1], farCorners[1]);
        lines[10].UpdateLine(nearCorners[2], farCorners[2]);
        lines[11].UpdateLine(nearCorners[3], farCorners[3]);

        if (text != null)
            text.text = $"Field Of View: {fov:0.0}";
    }
}
