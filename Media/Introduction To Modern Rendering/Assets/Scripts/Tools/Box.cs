using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

[ExecuteAlways]
public class Box : MonoBehaviour
{
    public float w;
    public float h;
    public float d;
    public Color color = Color.white;
    // public float dotVerticesSize = 0.1f;
    public float lineWidth = 0.025f;

    List<Line> lines = new();

    void OnEnable()
    {
        // Clear all children
        for (int i = 0; i < 100; i++)
        {
            if (transform.childCount > 0)
                DestroyImmediate(transform.GetChild(0).gameObject);
            else
                break;
        }

        lines.Clear();
        for (int i = 0; i < 12; i++)
        {
            var l = Instantiate(Resources.Load("Line"), transform).GetComponent<Line>();
            l.dotted = false;
            lines.Add(l);
        }

        Update();
    }

    Vector3[] corners = new Vector3[8];
    void Update()
    {
        // Update box corner positions
        for (int i = 0; i < 8; i++)
        {
            corners[i] = new Vector3(
                (i & 1) == 0 ? -w : w,
                (i & 2) == 0 ? -h : h,
                (i & 4) == 0 ? -d : d
            );
        }

        if (lines.Count > 0)
        {
            // Top Edges
            lines[0].UpdateLine(corners[0], corners[1], false);
            lines[1].UpdateLine(corners[1], corners[3], false);
            lines[2].UpdateLine(corners[3], corners[2], false);
            lines[3].UpdateLine(corners[2], corners[0], false);

            // Bottom Edges
            lines[4].UpdateLine(corners[4], corners[5], false);
            lines[5].UpdateLine(corners[5], corners[7], false);
            lines[6].UpdateLine(corners[7], corners[6], false);
            lines[7].UpdateLine(corners[6], corners[4], false);

            // Vertical Edges
            lines[8].UpdateLine(corners[0], corners[4], false);
            lines[9].UpdateLine(corners[1], corners[5], false);
            lines[10].UpdateLine(corners[2], corners[6], false);
            lines[11].UpdateLine(corners[3], corners[7], false);

            foreach (var l in lines)
            {
                l.width = lineWidth;
                l.color = color;
            }
        }
    }
    void OnDisable()
    {
    }
}