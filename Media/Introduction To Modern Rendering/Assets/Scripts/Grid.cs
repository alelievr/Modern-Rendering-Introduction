using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class Grid : MonoBehaviour
{
    public float mainLineWidth = 0.025f;
    public float secondaryLineWidth = 0.01f;
    public bool show3D = false; // 2D by default
    public Color mainColor = Color.white;
    public Color secondaryColor = Color.gray;
    public float scale = 1;
    public bool coloredAxis = true;

    public Color xAxisColor = Color.red;
    public Color yAxisColor = Color.green;
    public Color zAxisColor = Color.blue;
    public Color neutralAxisColor = Color.white;

    public int lineCount = 10;

    List<LineRenderer> lineRenderers = new List<LineRenderer>();

    void Start()
    {
        // Lines on X and Y axis
        int slices = show3D ? lineCount : 0;
        for (int z = -slices; z <= slices; z++)
        {
            // Vertical lines
            for (int x = -lineCount; x <= lineCount; x++)
            {
                float w = mainLineWidth;
                var color = mainColor;
                if (z == 0 && x == 0)
                {
                    color = coloredAxis ? yAxisColor : neutralAxisColor;
                    w += 0.01f;
                }
                // Main line
                AddLine(new Vector3(x, -lineCount, z), new Vector3(x, lineCount, z), color, w);
                // Secondary line
                if (x != lineCount)
                    AddLine(new Vector3(x + 0.5f, -lineCount, z), new Vector3(x + 0.5f, lineCount, z), secondaryColor, secondaryLineWidth);
            }

            // Horizontal lines
            for (int y = -lineCount; y <= lineCount; y++)
            {
                float w = mainLineWidth;
                var color = mainColor;
                if (z == 0 && y == 0)
                {
                    color = coloredAxis ? xAxisColor : neutralAxisColor;
                    w += 0.01f;
                }
                AddLine(new Vector3(-lineCount, y, z), new Vector3(lineCount, y, z), color, w);
                if (y != lineCount)
                    AddLine(new Vector3(-lineCount, y + 0.5f, z), new Vector3(lineCount, y + 0.5f, z), secondaryColor, secondaryLineWidth);
            }
        }

        // Lines on Z axis
        if (show3D)
        {
            for (int x = -lineCount; x <= lineCount; x++)
            {
                for (int y = -lineCount; y <= lineCount; y++)
                {
                    float w = mainLineWidth;
                    var color = mainColor;
                    if (x == 0 && y == 0)
                    {
                        color = coloredAxis ? zAxisColor : neutralAxisColor;
                    w += 0.01f;
                    }
                    AddLine(new Vector3(x, y, -lineCount), new Vector3(x, y, lineCount), color, w);
                    if (y != lineCount || x != lineCount)
                        AddLine(new Vector3(x + 0.5f, y + 0.5f, -lineCount), new Vector3(x + 0.5f, y + 0.5f, lineCount), secondaryColor, secondaryLineWidth);
                }
            }
        }
    }

    void AddLine(Vector3 start, Vector3 end, Color color, float width)
    {
        GameObject line = new GameObject("Line");
        line.transform.SetParent(transform);
        LineRenderer lineRenderer = line.AddComponent<LineRenderer>();
        lineRenderer.material = new Material(Shader.Find("Sprites/Default"));
        lineRenderer.startColor = color;
        lineRenderer.endColor = color;
        lineRenderer.startWidth = width;
        lineRenderer.endWidth = width;
        lineRenderer.positionCount = 2;
        lineRenderer.useWorldSpace = false;
        lineRenderer.SetPosition(0, start * scale);
        lineRenderer.SetPosition(1, end * scale);
        lineRenderers.Add(lineRenderer);
    }
}
