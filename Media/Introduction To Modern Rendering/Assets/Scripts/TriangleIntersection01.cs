using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Unity.VisualScripting;
using TMPro;

public class TriangleIntersection01 : MonoBehaviour
{
    public Vector3 a, b, c;
    public Vector3 p;

    public float aValue, bValue, cValue;

    public Color triangleColor = Color.white;
    public Color pointColor = Color.white;
    public float pointSize = 0.1f;
    public float lineWidth = 0.025f;

    public float animationTime = 6;

    public TMP_Text text;

    Triangle mainTriangle;

    AnchoredText pText;

    Dot pointP;

    void Start()
    {
        mainTriangle = SpawnTriangle(a, b, c, triangleColor, true, true, true);

        pointP = Instantiate(Resources.Load("Dot")).GetComponent<Dot>();
        pointP.color = pointColor;
        pointP.size = pointSize;

        pText = Instantiate(Resources.Load("AnchoredText")).GetComponent<AnchoredText>();

        mainTriangle.SetALabel(aValue.ToString());
        mainTriangle.SetBLabel(bValue.ToString());
        mainTriangle.SetCLabel(cValue.ToString());

        pText.SetAnchor(pointP.transform);
        pText.transformOffset = new Vector3(0.25f, 0, 0);
    }

    Triangle SpawnTriangle(Vector3 a, Vector3 b, Vector3 c, Color color, bool showText = false, bool showEdges = false, bool showDotVertices = false)
    {
        var triangle = Instantiate(Resources.Load("Triangle").GetComponent<Triangle>());
        triangle.a = a;
        triangle.b = b;
        triangle.c = c;
        triangle.color = color;
        triangle.showText = showText;
        triangle.showEdges = showEdges;
        triangle.showDotVertices = showDotVertices;
        triangle.lineWidth = lineWidth;

        return triangle;
    }

    void Update()
    {
        pointP.transform.position = p;

        float area = Vector3.Cross(b - a, c - a).magnitude / 2;
        float uValue = Vector3.Cross(b - p, c - p).magnitude / 2 / area;
        float vValue = Vector3.Cross(c - p, a - p).magnitude / 2 / area;
        float wValue = Vector3.Cross(a - p, b - p).magnitude / 2 / area;

        float finalValue = aValue * uValue + bValue * vValue + cValue * wValue;

        pText.SetText($"P = {finalValue:0.00}");
    }
}
