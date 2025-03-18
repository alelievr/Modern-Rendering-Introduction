using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class BackgroundGrid : MonoBehaviour
{
    public enum Orientation
    {
        XY,
        XZ,
    }

    public float mainLineWidth = 0.025f;
    public float secondaryLineWidth = 0.01f;
    public bool show3D = false; // 2D by default
    public Color mainColor = Color.white;
    public Color secondaryColor = Color.gray;
    public float scale = 1;
    public bool coloredAxis = true;
    public Orientation orientation = Orientation.XY;

    public bool showSecondaryLines = true;

    public Color xAxisColor = Color.red;
    public Color yAxisColor = Color.green;
    public Color zAxisColor = Color.blue;
    public Color neutralAxisColor = Color.white;

    public bool showXAxis = true;
    public bool showYAxis = true;
    public bool showZAxis = true;
    
    public int lineCount = 10;

    List<LineRenderer> lineRenderers = new List<LineRenderer>();

    void Start()
    {
        int lineAxisCount = this.lineCount;
        float lineLength = lineCount;
        // Lines on X and Y axis
        int slices = show3D ? lineCount : 0;

        for (int z = -slices; z <= slices; z++)
        {
            // Vertical lines
            for (int x = -lineAxisCount; x <= lineAxisCount; x++)
            {
                float w = mainLineWidth;
                var color = mainColor;
                bool transparent = true;
                if (z == 0 && x == 0 && showYAxis)
                {
                    if (orientation == Orientation.XZ)
                        color = coloredAxis ? zAxisColor : neutralAxisColor;
                    else
                        color = coloredAxis ? yAxisColor : neutralAxisColor;
                    transparent = false;
                    w += 0.01f;
                }
                // Main line
                AddLine(new Vector3(x, -lineLength, z), new Vector3(x, lineLength, z), color, w, transparent);
                // Secondary line
                if (x != lineAxisCount && showSecondaryLines)
                    AddLine(new Vector3(x + 0.5f, -lineLength, z), new Vector3(x + 0.5f, lineLength, z), secondaryColor, secondaryLineWidth);
            }

            // Horizontal lines
            for (int y = -lineAxisCount; y <= lineAxisCount; y++)
            {
                float w = mainLineWidth;
                var color = mainColor;
                bool transparent = true;
                if (z == 0 && y == 0 && showXAxis)
                {
                    color = coloredAxis ? xAxisColor : neutralAxisColor;
                    transparent = false;
                    w += 0.01f;
                }
                AddLine(new Vector3(-lineLength, y, z), new Vector3(lineLength, y, z), color, w, transparent);
                if (y != lineAxisCount && showSecondaryLines)
                    AddLine(new Vector3(-lineLength, y + 0.5f, z), new Vector3(lineLength, y + 0.5f, z), secondaryColor, secondaryLineWidth);
            }
        }

        // Lines on Z axis
        if (show3D)
        {
            for (int x = -lineAxisCount; x <= lineAxisCount; x++)
            {
                for (int y = -lineAxisCount; y <= lineAxisCount; y++)
                {
                    float w = mainLineWidth;
                    var color = mainColor;
                    bool transparent = true;
                    if (x == 0 && y == 0 && showZAxis)
                    {
                        color = coloredAxis ? zAxisColor : neutralAxisColor;
                        transparent = false;
                        w += 0.01f;
                    }
                    AddLine(new Vector3(x, y, -lineLength), new Vector3(x, y, lineLength), color, w, transparent);
                    if ((y != lineAxisCount || x != lineAxisCount) && showSecondaryLines)
                        AddLine(new Vector3(x + 0.5f, y + 0.5f, -lineLength), new Vector3(x + 0.5f, y + 0.5f, lineLength), secondaryColor, secondaryLineWidth);
                }
            }
        }
    }

    void AddLine(Vector3 start, Vector3 end, Color color, float width, bool transparent = true)
    {
        if (orientation == Orientation.XZ)
        {
            start = new Vector3(start.x, start.z, start.y);
            end = new Vector3(end.x, end.z, end.y);
        }

        var mat = transparent ? new Material(Shader.Find("Sprites/Default")) : new Material(Resources.Load<Material>("Opaque Axis Lines"));
        if (!transparent)
            mat.SetColor("_Color", color);
        GameObject line = new GameObject("Line");
        line.transform.SetParent(transform);
        LineRenderer lineRenderer = line.AddComponent<LineRenderer>();
        lineRenderer.material = mat;
        lineRenderer.startColor = color;
        lineRenderer.endColor = color;
        lineRenderer.startWidth = width;
        lineRenderer.endWidth = width;
        lineRenderer.positionCount = 2;
        lineRenderer.useWorldSpace = false;
        lineRenderer.SetPosition(0, start * scale + transform.position);
        lineRenderer.SetPosition(1, end * scale + transform.position);
        lineRenderers.Add(lineRenderer);
    }
}
