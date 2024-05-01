using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using static Unity.Mathematics.math;

public class AnchoredText : MonoBehaviour
{
    public enum AnchorMode
    {
        None,
        Transform,
        LineCenter,
        Manual,
        ArrowCenter,
    }

    public AnchorMode anchorMode;
    public Color color = Color.white;
    public string text;

    [Header("Manual Anchor")]
    public Vector3 position;
    public Vector3 direction;

    TextMeshPro textMesh;
    Transform followTransform;
    Line followLine;
    Arrow followArrow;

    void OnEnable()
    {
        textMesh = GetComponentInChildren<TextMeshPro>();
        textMesh.color = color;
        textMesh.text = text;
    }

    public void SetText(string text, Color color = default)
    {
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

    // Update is called once per frame
    void Update()
    {
        if (anchorMode == AnchorMode.None)
            return;

        Vector3 newPosition = Vector3.zero;

        var textSize = textMesh.GetRenderedValues(true);
        float radiusX = textSize.x / 2 + 0.1f;
        float radiusY = textSize.y / 2 + 0.1f;
        var cameraTransform = Camera.main.transform;

        switch (anchorMode)
        {
            case AnchorMode.Transform:
                newPosition = followTransform.position;
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
        }

        transform.position = newPosition;
        if (Camera.main.orthographic)
        {
            transform.rotation = Quaternion.LookRotation(cameraTransform.forward, cameraTransform.up);
        }
        else
            transform.LookAt(cameraTransform);
    }
}
