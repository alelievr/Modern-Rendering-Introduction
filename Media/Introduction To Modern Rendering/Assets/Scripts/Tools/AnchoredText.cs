using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using static Unity.Mathematics.math;

[ExecuteAlways]
public class AnchoredText : MonoBehaviour
{
    public enum AnchorMode
    {
        None,
        Transform,
        LineCenter,
        Manual,
        ArrowCenter,
        CopyCurrentTransform,
        TriangleCenter,
    }

    public AnchorMode anchorMode;
    public Color color = Color.white;
    public string text;

    [Header("Manual Anchor")]
    public Vector3 position;
    public Vector3 direction;

    [HideInInspector]
    public Vector3 transformOffset;

    TextMeshPro textMesh;
    Transform followTransform;
    Line followLine;
    Arrow followArrow;
    Triangle followTriangle;

    Camera[] cameras;

    void OnEnable()
    {
        textMesh = GetComponentInChildren<TextMeshPro>();
        textMesh.color = color;
        textMesh.text = text;

        cameras = FindObjectsByType<Camera>(FindObjectsInactive.Exclude, FindObjectsSortMode.None);

        Camera.onPreRender += BeforeRender;
    }

    void OnDisable()
    {
        Camera.onPreRender -= BeforeRender;
    }

    public void SetText(string text, Color color = default)
    {
        this.text = text;
        textMesh.text = text;
        if (color != default)
            textMesh.color = color;
    }

    public void SetAnchor(Transform t)
    {
        anchorMode = AnchorMode.Transform;
        followTransform = t;
    }

    public void SetAnchor(Line line)
    {
        anchorMode = AnchorMode.LineCenter;
        followLine = line;
    }

    public void SetAnchor(Arrow arrow)
    {
        anchorMode = AnchorMode.ArrowCenter;
        followArrow = arrow;
    }

    public void SetAnchor(Vector3 position, Vector3 direction = default)
    {
        anchorMode = AnchorMode.Manual;
        this.position = position;
        this.direction = direction;
    }

    public void SetAnchor(Triangle triangle)
    {
        anchorMode = AnchorMode.TriangleCenter;
        followTriangle = triangle;
    }

    void Update()
    {
        if (textMesh.text != text)
            textMesh.text = text;
    }

    void BeforeRender(Camera camera)
    {
        if (anchorMode == AnchorMode.None || transform == null)
            return;

        Vector3 newPosition = Vector3.zero;

        if (color != textMesh.color)
            textMesh.color = color;

        var textSize = textMesh.GetRenderedValues(true);
        float radiusX = textSize.x / 2 + 0.1f;
        float radiusY = textSize.y / 2 + 0.1f;
        var cameraTransform = camera.transform;

        if (anchorMode == AnchorMode.CopyCurrentTransform)
        {
            position = transform.position;
            direction = Vector3.zero;
        }

        switch (anchorMode)
        {
            case AnchorMode.Transform:
                newPosition = followTransform.position + transformOffset;
                break;
            case AnchorMode.LineCenter:
                newPosition = followLine.start + (followLine.end - followLine.start) / 2;
                // Approximate offset depending on the size of the text:
                Vector3 lineNormal = normalize(cross(followLine.end - followLine.start, cameraTransform.forward));
                // Weight radius depending on the 2D orientation of the line:
                radiusX *= dot(lineNormal, cameraTransform.right);
                radiusY *= dot(lineNormal, cameraTransform.up);
                float radius = abs(radiusX) > abs(radiusY) ? radiusX : radiusY;
                if (!isnan(lineNormal.x))
                    newPosition += lineNormal * radius;
                break;
            case AnchorMode.ArrowCenter:
                newPosition = followArrow.start + (followArrow.end - followArrow.start) / 2;
                // Approximate offset depending on the size of the text:
                lineNormal = normalize(cross(followArrow.end - followArrow.start, cameraTransform.forward));
                // Weight radius depending on the 2D orientation of the line:
                radiusX *= dot(lineNormal, cameraTransform.right);
                radiusY *= dot(lineNormal, cameraTransform.up);
                radius = abs(radiusX) > abs(radiusY) ? radiusX : radiusY;
                if (!isnan(lineNormal.x))
                    newPosition += lineNormal * radius;
                break;
            case AnchorMode.Manual:
            case AnchorMode.CopyCurrentTransform:
                newPosition = position;

                // Approximate offset depending on the size of the text:
                lineNormal = normalize(cross(direction, cameraTransform.forward));
                // Weight radius depending on the 2D orientation of the line:
                radiusX *= dot(lineNormal, cameraTransform.right);
                radiusY *= dot(lineNormal, cameraTransform.up);
                radius = abs(radiusX) > abs(radiusY) ? radiusX : radiusY;
                if (!isnan(lineNormal.x))
                    newPosition += lineNormal * radius;

                break;
            case AnchorMode.TriangleCenter:
                var centroid = (followTriangle.a + followTriangle.b + followTriangle.c) / 3;
                newPosition = followTriangle.transform.TransformPoint(centroid);
                break;
        }

        transform.position = newPosition;
        transform.rotation = Quaternion.LookRotation(cameraTransform.forward, cameraTransform.up);
    }
}
