using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Line : MonoBehaviour
{
    public Vector3 start;
    public Vector3 end;
    public Color color = Color.white;
    public bool dotted = false;
    public float spacing = 0.25f;
    public float width = 0.05f;

    void Start()
    {
        LineRenderer lineRenderer = gameObject.AddComponent<LineRenderer>();
        lineRenderer.material = new Material(Shader.Find("Hidden/LineRenderer/DottedLine"));
        lineRenderer.startColor = color;
        lineRenderer.endColor = color;
        lineRenderer.startWidth = width;
        lineRenderer.endWidth = width;
        lineRenderer.SetPosition(0, start);
        lineRenderer.SetPosition(1, end);

        if (dotted)
        {
            lineRenderer.material.SetFloat("_LineSpacing", spacing);
            lineRenderer.material.SetFloat("_LineLength", (end - start).magnitude);
        }
        else
        {
            lineRenderer.material.SetFloat("_LineSpacing", 0);
            lineRenderer.material.SetFloat("_LineLength", 0);
        }
    }

    public void UpdateLine(Vector3 start, Vector3 end)
    {
        this.start = start;
        this.end = end;
        LineRenderer lineRenderer = GetComponent<LineRenderer>();
        lineRenderer.SetPosition(0, start);
        lineRenderer.SetPosition(1, end);
        if (dotted)
        {
            lineRenderer.material.SetFloat("_LineLength", (end - start).magnitude);
        }
    }
}
