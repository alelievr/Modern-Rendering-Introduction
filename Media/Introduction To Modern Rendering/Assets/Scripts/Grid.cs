using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Grid : MonoBehaviour
{
    public float lineWidth = 0.1f;
    public bool show3D = false; // 2D by default
    public Color color = Color.white;

    public int lineCount = 10;

    List<LineRenderer> lineRenderers = new List<LineRenderer>();

    void Start()
    {
        int slices = show3D ? lineCount : 0;
        for (int z = -slices; z <= slices; z++)
        {
            for (int x = -lineCount; x <= lineCount; x++)
            {
                GameObject line = new GameObject("Line");
                line.transform.SetParent(transform);
                LineRenderer lineRenderer = line.AddComponent<LineRenderer>();
                lineRenderer.material = new Material(Shader.Find("Sprites/Default"));
                lineRenderer.startColor = color;
                lineRenderer.endColor = color;
                lineRenderer.startWidth = lineWidth;
                lineRenderer.endWidth = lineWidth;
                lineRenderer.positionCount = 2;
                lineRenderer.SetPosition(0, new Vector3(x, -lineCount, z));
                lineRenderer.SetPosition(1, new Vector3(x, lineCount, z));
                lineRenderers.Add(lineRenderer);
            }

            for (int y = -lineCount; y <= lineCount; y++)
            {
                GameObject line = new GameObject("Line");
                line.transform.SetParent(transform);
                LineRenderer lineRenderer = line.AddComponent<LineRenderer>();
                lineRenderer.material = new Material(Shader.Find("Sprites/Default"));
                lineRenderer.startColor = color;
                lineRenderer.endColor = color;
                lineRenderer.startWidth = lineWidth;
                lineRenderer.endWidth = lineWidth;
                lineRenderer.positionCount = 2;
                lineRenderer.SetPosition(0, new Vector3(-lineCount, y, z));
                lineRenderer.SetPosition(1, new Vector3(lineCount, y, z));
                lineRenderers.Add(lineRenderer);
            }
        }
    }

    void Update()
    {
        
    }
}
