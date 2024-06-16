using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

[ExecuteAlways]
public class Triangle : MonoBehaviour
{
    public Vector3 a;
    public Vector3 b;
    public Vector3 c;
    public Color color = Color.white;
    public Color edgeColor = Color.white;
    public Color textColor = Color.white;
    public bool showText = true;
    public bool showEdges = true;
    public bool showDotVertices = true;
    public float dotVerticesSize = 0.1f;
    public float textDistanceToVertices = 0.25f;
    public float lineWidth = 0.025f;

    Line ab;
    Line bc;
    Line ca;
    Dot aDot;
    Dot bDot;
    Dot cDot;

    AnchoredText anchoredTextA;
    AnchoredText anchoredTextB;
    AnchoredText anchoredTextC;

    Mesh triangleMesh;
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
            ab = Instantiate(Resources.Load("Line"), transform).GetComponent<Line>();
            bc = Instantiate(Resources.Load("Line"), transform).GetComponent<Line>();
            ca = Instantiate(Resources.Load("Line"), transform).GetComponent<Line>();

            ab.dotted = bc.dotted = ca.dotted = false;
            
            ab.UpdateLine(a, b); 
            bc.UpdateLine(b, c);
            ca.UpdateLine(c, a);
        }

        if (showDotVertices)
        {
            aDot = Instantiate(Resources.Load("Dot"), transform).GetComponent<Dot>();
            bDot = Instantiate(Resources.Load("Dot"), transform).GetComponent<Dot>();
            cDot = Instantiate(Resources.Load("Dot"), transform).GetComponent<Dot>();
        }

        if (showText)
        {
            anchoredTextA = Instantiate(Resources.Load("AnchoredText"), transform).GetComponent<AnchoredText>();
            anchoredTextB = Instantiate(Resources.Load("AnchoredText"), transform).GetComponent<AnchoredText>();
            anchoredTextC = Instantiate(Resources.Load("AnchoredText"), transform).GetComponent<AnchoredText>();

            anchoredTextA.anchorMode = anchoredTextB.anchorMode = anchoredTextC.anchorMode = AnchoredText.AnchorMode.CopyCurrentTransform;
            
            anchoredTextA.SetText("A", color);
            anchoredTextB.SetText("B", color);
            anchoredTextC.SetText("C", color);
        }

        triangleMesh = new Mesh
        {
            vertices = new Vector3[] { a, b, c },
            triangles = new int[] { 0, 1, 2 }
        };
        triangleMesh.RecalculateNormals();
        triangleMesh.RecalculateBounds();

        var mf = gameObject.GetComponent<MeshFilter>();
        if (mf == null)
            mf = gameObject.AddComponent<MeshFilter>();
        mf.sharedMesh = triangleMesh;
        material = Instantiate(Resources.Load("TriangleMaterial") as Material);
        material.color = color;
        var mr = gameObject.GetComponent<MeshRenderer>();
        if (mr == null)
            mr = gameObject.AddComponent<MeshRenderer>();
        mr.sharedMaterial = material;
    }

    void Update()
    {
        if (showEdges)
        {
            ab.UpdateLine(a, b, false);
            bc.UpdateLine(b, c, false);
            ca.UpdateLine(c, a, false);

            ab.width = bc.width = ca.width = lineWidth;
        }
        else if (ab != null)
        {
            DestroyImmediate(ab.gameObject);
            DestroyImmediate(bc.gameObject);
            DestroyImmediate(ca.gameObject);
        }

        ab.color = bc.color = ca.color = edgeColor;

        if (showDotVertices)
        {
            aDot.transform.position = transform.TransformPoint(a);
            bDot.transform.position = transform.TransformPoint(b);
            cDot.transform.position = transform.TransformPoint(c);

            aDot.size = bDot.size = cDot.size = dotVerticesSize;
        }
        else if (aDot != null)
        {
            DestroyImmediate(aDot.gameObject);
            DestroyImmediate(bDot.gameObject);
            DestroyImmediate(cDot.gameObject);
        }

        triangleMesh.vertices = new Vector3[] { a, b, c };
        triangleMesh.RecalculateNormals();
        triangleMesh.RecalculateBounds();

        if (material != null)
            material.color = color;

        if (showText && showEdges && showDotVertices)
        {
            anchoredTextA.color = anchoredTextB.color = anchoredTextC.color = textColor;

            var h = (ab.GetDirection() - ca.GetDirection()) / 2.0f;
            anchoredTextA.transform.position = aDot.transform.position - h * textDistanceToVertices;
            h = (-ab.GetDirection() + bc.GetDirection()) / 2.0f;
            anchoredTextB.transform.position = bDot.transform.position - h * textDistanceToVertices;
            h = (ca.GetDirection() - bc.GetDirection()) / 2.0f;
            anchoredTextC.transform.position = cDot.transform.position - h * textDistanceToVertices;
        }

        if (!showText && anchoredTextA != null)
        {
            DestroyImmediate(anchoredTextA.gameObject);
            DestroyImmediate(anchoredTextB.gameObject);
            DestroyImmediate(anchoredTextC.gameObject);
        }
    }

    public void SetALabel(string text)
    {
        anchoredTextA.SetText(text, textColor);
    }

    public void SetBLabel(string text)
    {
        anchoredTextB.SetText(text, textColor);
    }

    public void SetCLabel(string text)
    {
        anchoredTextC.SetText(text, textColor);
    }

    void OnDisable()
    {
        DestroyImmediate(triangleMesh);
        DestroyImmediate(material);
    }
}
