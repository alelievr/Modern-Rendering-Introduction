using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[ExecuteAlways]
public class Line : MonoBehaviour
{
    public enum Mode
    {
        Positions,
        Transforms,
    }

    public Mode mode;
    
    [Header("Positions")]
    public Vector3 start;
    public Vector3 end;

    [Header("Transforms")]
    public Transform startTransform;
    public Transform endTransform;
    
    [Header("Settings")]
    public Color color = Color.white;
    public bool dotted = false;
    public float spacing = 0.25f;
    public float width = 0.05f;

    LineRenderer lineRenderer;

    void OnEnable()
    {
        lineRenderer = GetComponent<LineRenderer>();
        if (lineRenderer == null)
            lineRenderer = gameObject.AddComponent<LineRenderer>();
        lineRenderer.sharedMaterial = new Material(Shader.Find("Hidden/LineRenderer/DottedLine"));
        lineRenderer.startColor = color;
        lineRenderer.endColor = color;
        lineRenderer.startWidth = width;
        lineRenderer.endWidth = width;
        lineRenderer.SetPosition(0, start);
        lineRenderer.SetPosition(1, end);

        if (dotted)
        {
            lineRenderer.sharedMaterial.SetFloat("_LineSpacing", spacing);
            lineRenderer.sharedMaterial.SetFloat("_LineLength", (end - start).magnitude);
        }
        else
        {
            lineRenderer.sharedMaterial.SetFloat("_LineSpacing", 0);
            lineRenderer.sharedMaterial.SetFloat("_LineLength", 0);
        }
    }

    public void UpdateLine(Vector3 start, Vector3 end, bool useWorldSpace = true)
    {
        this.start = start;
        this.end = end;
        if (lineRenderer == null || lineRenderer.sharedMaterial == null)
            return;
        lineRenderer.SetPosition(0, start);
        lineRenderer.SetPosition(1, end);
        lineRenderer.useWorldSpace = useWorldSpace;
        if (dotted)
            lineRenderer.sharedMaterial.SetFloat("_LineLength", (end - start).magnitude);
        else
            lineRenderer.sharedMaterial.SetFloat("_LineLength", 0);

        if (lineRenderer.startColor != color)
        {
            lineRenderer.startColor = color;
            lineRenderer.endColor = color;
        }

        if (lineRenderer.startWidth != width)
        {
            lineRenderer.startWidth = width;
            lineRenderer.endWidth = width;
        }
    }

    void Update()
    {
        if (mode == Mode.Transforms)
        {
            start = startTransform.position;
            end = endTransform.position;
        }
    }

    public Vector3 GetDirection()
    {
        return (end - start).normalized;
    }

    void OnDisable()
    {
        if (lineRenderer != null)
            DestroyImmediate(lineRenderer.sharedMaterial);
    }
}
