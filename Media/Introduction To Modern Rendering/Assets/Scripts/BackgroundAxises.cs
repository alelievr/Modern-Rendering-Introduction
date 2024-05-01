using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class BackgroundAxises : MonoBehaviour
{
    public float mainLineWidth = 0.025f;
    public float secondaryLineWidth = 0.01f;
    public float tickMarkLength = 0.2f;
    public float tickMarkWidthMultiplier = 0.8f;
    public bool show3D = false; // 2D by default
    public Color mainColor = Color.white;
    public Color secondaryColor = Color.gray;
    public float scale = 1;
    public bool coloredAxis = true;

    public bool showSecondaryLines = true;

    public Color xAxisColor = Color.red;
    public Color yAxisColor = Color.green;
    public Color zAxisColor = Color.blue;
    public Color neutralAxisColor = Color.white;

    public int lineCount = 10;

    List<LineRenderer> lineRenderers = new List<LineRenderer>();

    void Start()
    {
        int lineAxisCount = this.lineCount;
        float lineLength = lineCount;

        // Draw tick marks on X axis
        for (int x = -lineAxisCount; x <= lineAxisCount; x++)
        {
            if (x != 0)
                AddLine(new Vector3(x, -tickMarkLength, 0), new Vector3(x, tickMarkLength, 0), xAxisColor, mainLineWidth * tickMarkWidthMultiplier);
            // secondary tick marks
            if (x != lineAxisCount && showSecondaryLines)
                AddLine(new Vector3(x + 0.5f, -tickMarkLength * 0.8f, 0), new Vector3(x + 0.5f, tickMarkLength * 0.8f, 0), secondaryColor, secondaryLineWidth * tickMarkWidthMultiplier);
            
            if (show3D)
            {
                AddLine(new Vector3(x, 0, -tickMarkLength), new Vector3(x, 0, tickMarkLength), xAxisColor, mainLineWidth * tickMarkWidthMultiplier);
                // secondary tick marks
                if (x != lineAxisCount && showSecondaryLines)
                    AddLine(new Vector3(x + 0.5f, 0, -tickMarkLength * 0.8f), new Vector3(x + 0.5f, 0, tickMarkLength * 0.8f), secondaryColor, secondaryLineWidth * tickMarkWidthMultiplier);
            }
        }

        // Draw tick marks on Y axis
        for (int y = -lineAxisCount; y <= lineAxisCount; y++)
        {
            if (y != 0)
                AddLine(new Vector3(-tickMarkLength, y, 0), new Vector3(tickMarkLength, y, 0), yAxisColor, mainLineWidth * tickMarkWidthMultiplier);
            // secondary tick marks
            if (y != lineAxisCount && showSecondaryLines)
                AddLine(new Vector3(-tickMarkLength * 0.8f, y + 0.5f, 0), new Vector3(tickMarkLength * 0.8f, y + 0.5f, 0), secondaryColor, secondaryLineWidth * tickMarkWidthMultiplier);

            if (show3D)
            {
                AddLine(new Vector3(0, y, -tickMarkLength), new Vector3(0, y, tickMarkLength), yAxisColor, mainLineWidth * tickMarkWidthMultiplier);
                // secondary tick marks
                if (y != lineAxisCount && showSecondaryLines)
                    AddLine(new Vector3(0, y + 0.5f, -tickMarkLength * 0.8f), new Vector3(0, y + 0.5f, tickMarkLength * 0.8f), secondaryColor, secondaryLineWidth * tickMarkWidthMultiplier);
            }
        }

        // Draw tick marks on Z axis
        if (show3D)
        {
            for (int z = -lineAxisCount; z <= lineAxisCount; z++)
            {
                AddLine(new Vector3(-tickMarkLength, 0, z), new Vector3(tickMarkLength, 0, z), zAxisColor, mainLineWidth * tickMarkWidthMultiplier);
                // secondary tick marks
                if (z != lineAxisCount && showSecondaryLines)
                    AddLine(new Vector3(-tickMarkLength * 0.8f, 0, z + 0.5f), new Vector3(tickMarkLength * 0.8f, 0, z + 0.5f), secondaryColor, secondaryLineWidth * tickMarkWidthMultiplier);

                AddLine(new Vector3(0, -tickMarkLength, z), new Vector3(0, tickMarkLength, z), zAxisColor, mainLineWidth * tickMarkWidthMultiplier);
                // secondary tick marks
                if (z != lineAxisCount && showSecondaryLines)
                    AddLine(new Vector3(0, -tickMarkLength * 0.8f, z + 0.5f), new Vector3(0, tickMarkLength * 0.8f, z + 0.5f), secondaryColor, secondaryLineWidth * tickMarkWidthMultiplier);
            }
        }
        
        // Draw X Y and Z axises
        AddLine(new Vector3(-lineLength, 0, 0), new Vector3(lineLength, 0, 0), xAxisColor, mainLineWidth);
        AddLine(new Vector3(0, -lineLength, 0), new Vector3(0, lineLength, 0), yAxisColor, mainLineWidth);
        if (show3D)
            AddLine(new Vector3(0, 0, -lineLength), new Vector3(0, 0, lineLength), zAxisColor, mainLineWidth);
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
