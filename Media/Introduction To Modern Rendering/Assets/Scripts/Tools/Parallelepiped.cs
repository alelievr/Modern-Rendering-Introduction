using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

[ExecuteAlways]
public class Parallelepiped : MonoBehaviour
{
    public Arrow a;
    public Arrow b;
    public Arrow c;
    public Color color = Color.white;
    // public float dotVerticesSize = 0.1f;
    public float lineWidth = 0.025f;
    public bool showEdges;

    List<Line> lines = new();
    // Line bc;
    // Line ca;
    // Dot aDot;
    // Dot bDot;
    // Dot cDot;

    AnchoredText anchoredTextA;
    AnchoredText anchoredTextB;
    AnchoredText anchoredTextC;

    Mesh mesh;
    Material material;

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

        if (showEdges)
        {
            lines.Clear();
            for (int i = 0; i < 9; i++)
            {
                var l = Instantiate(Resources.Load("Line"), transform).GetComponent<Line>();
                lines.Add(l);
            }
        }
        // if (showDotVertices)
        // {
        //     aDot = Instantiate(Resources.Load("Dot"), transform).GetComponent<Dot>();
        //     bDot = Instantiate(Resources.Load("Dot"), transform).GetComponent<Dot>();
        //     cDot = Instantiate(Resources.Load("Dot"), transform).GetComponent<Dot>();
        // }
        
        UpdateMesh();

        var mf = gameObject.GetComponent<MeshFilter>();
        if (mf == null)
            mf = gameObject.AddComponent<MeshFilter>();
        mf.sharedMesh = mesh;
        material = Instantiate(Resources.Load("LitMeshMaterial") as Material);
        material.color = color;
        var mr = gameObject.GetComponent<MeshRenderer>();
        if (mr == null)
            mr = gameObject.AddComponent<MeshRenderer>();
        mr.sharedMaterial = material;
    }

    Vector3[] corners;

    void UpdateMesh()
    {
        var ad = a.GetVector();
        var bd = b.GetVector();
        
        // Vector3[] v = new[] { a.start, c.end, c.end + ad, a.end, b.end + ad, c.end + ad + bd, c.end + bd, b.end };
        corners = new[] { b.end, c.end + bd, c.end, a.start, b.end + ad, c.end + ad + bd, c.end + ad, a.end };

        if (mesh == null)
            mesh = new Mesh();

        mesh.vertices = new[]
        {
            corners[0], corners[1], corners[2], corners[3],
            corners[7], corners[4], corners[0], corners[3],
            corners[4], corners[5], corners[1], corners[0],
            corners[6], corners[7], corners[3], corners[2],
            corners[5], corners[6], corners[2], corners[1],
            corners[7], corners[6], corners[5], corners[4]
        };

        mesh.triangles = new[]
        {
            3, 1, 0, 3, 2, 1,
            7, 5, 4, 7, 6, 5,
            11, 9, 8, 11, 10, 9,
            15, 13, 12, 15, 14, 13,
            19, 17, 16, 19, 18, 17,
            23, 21, 20, 23, 22, 21,
        };
        mesh.RecalculateNormals();
        mesh.RecalculateBounds();
    }

    void Update()
    {
        transform.position = b.start;
        UpdateMesh();
        if (showEdges && lines.Count > 0)
        {
            lines[0].UpdateLine(corners[2], corners[1], false);
            lines[1].UpdateLine(corners[2], corners[6], false);
            lines[2].UpdateLine(corners[7], corners[6], false);
            lines[3].UpdateLine(corners[7], corners[4], false);
            lines[4].UpdateLine(corners[0], corners[1], false);
            lines[5].UpdateLine(corners[0], corners[4], false);
            lines[6].UpdateLine(corners[6], corners[5], false);
            lines[7].UpdateLine(corners[1], corners[5], false);
            lines[8].UpdateLine(corners[4], corners[5], false);

            foreach (var l in lines)
                l.width = lineWidth;
        }
        // else if (ab != null)
        // {
        //     DestroyImmediate(ab.gameObject);
        //     DestroyImmediate(bc.gameObject);
        //     DestroyImmediate(ca.gameObject);
        // }
        //
        // ab.color = bc.color = ca.color = edgeColor;
        //
        // if (showDotVertices)
        // {
        //     aDot.transform.position = transform.TransformPoint(a);
        //     bDot.transform.position = transform.TransformPoint(b);
        //     cDot.transform.position = transform.TransformPoint(c);
        //
        //     aDot.size = bDot.size = cDot.size = dotVerticesSize;
        // }
        // else if (aDot != null)
        // {
        //     DestroyImmediate(aDot.gameObject);
        //     DestroyImmediate(bDot.gameObject);
        //     DestroyImmediate(cDot.gameObject);
        // }
        //
        if (material != null)
            material.color = color;
        
        // if (showText && showEdges && showDotVertices)
        // {
        //     anchoredTextA.color = anchoredTextB.color = anchoredTextC.color = textColor;
        //
        //     var h = (ab.GetDirection() - ca.GetDirection()) / 2.0f;
        //     anchoredTextA.transform.position = aDot.transform.position - h * textDistanceToVertices;
        //     h = (-ab.GetDirection() + bc.GetDirection()) / 2.0f;
        //     anchoredTextB.transform.position = bDot.transform.position - h * textDistanceToVertices;
        //     h = (ca.GetDirection() - bc.GetDirection()) / 2.0f;
        //     anchoredTextC.transform.position = cDot.transform.position - h * textDistanceToVertices;
        // }
        //
        // if (!showText && anchoredTextA != null)
        // {
        //     DestroyImmediate(anchoredTextA.gameObject);
        //     DestroyImmediate(anchoredTextB.gameObject);
        //     DestroyImmediate(anchoredTextC.gameObject);
        // }
    }
    void OnDisable()
    {
        DestroyImmediate(mesh);
        DestroyImmediate(material);
    }
}