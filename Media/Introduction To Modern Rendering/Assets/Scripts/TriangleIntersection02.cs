using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Unity.VisualScripting;
using TMPro;

public class TriangleIntersection02 : MonoBehaviour
{
    public Vector3 a, b, c;
    public Vector3 p;
    public float lineRotationX;
    public Vector3 lineOrigin;

    public Color pointColor = Color.white;
    public Color triangleColor = Color.white;
    public Color eArrowColor = Color.white;
    public float pointSize = 0.1f;
    public float lineWidth = 0.025f;

    public float animationTime = 6;

    public TMP_Text text;

    Triangle mainTriangle;

    AnchoredText pText;
    AnchoredText eText;

    Arrow ab, ac;
    Arrow eArrow;

    Dot pointP;

    Line l;

    void Start()
    {
        mainTriangle = SpawnTriangle(a, b, c, triangleColor, true, true, true);

        pointP = Instantiate(Resources.Load("Dot")).GetComponent<Dot>();
        pointP.color = pointColor;
        pointP.size = pointSize;

        pText = Instantiate(Resources.Load("AnchoredText")).GetComponent<AnchoredText>();
        eText = Instantiate(Resources.Load("AnchoredText")).GetComponent<AnchoredText>();
        ab = Instantiate(Resources.Load("Arrow")).GetComponent<Arrow>();
        ac = Instantiate(Resources.Load("Arrow")).GetComponent<Arrow>();
        eArrow = Instantiate(Resources.Load("Arrow")).GetComponent<Arrow>();
        l = Instantiate(Resources.Load("Line")).GetComponent<Line>();

        pText.SetText("P");
        pText.SetAnchor(pointP.transform);
        pText.transformOffset = new Vector3(0.25f, 0, 0);

        eText.SetText("⋅e⃗");
        eText.SetAnchor(eArrow);

        l.dotted = false;
        l.color = Color.white;
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
        var dir = new Vector3(Mathf.Sin(lineRotationX), Mathf.Cos(lineRotationX), 0f);

        var n = Vector3.Cross(b - a, c - a).normalized;
        
        float t = Vector3.Dot(-lineOrigin, n) / Vector3.Dot(dir, n);

        p = t * dir + lineOrigin;

        pointP.transform.position = p;

        ab.Initialize(a, b, Utils.GetXAxisColor());
        ac.Initialize(a, c, Utils.GetZAxisColor());

        var acVector = c - a;
        var abVector = b - a;
        var cross = Vector3.Cross(acVector, dir);
        float determinant = Vector3.Dot(abVector, cross);

        eArrow.Initialize(a, a + cross, eArrowColor);

        l.UpdateLine(lineOrigin - dir * 1000f, lineOrigin + dir * 1000f);

        float area = Vector3.Cross(b - a, c - a).magnitude / 2;
        float uValue = Vector3.Cross(b - p, c - p).magnitude / 2 / area;
        float vValue = Vector3.Cross(c - p, a - p).magnitude / 2 / area;
        float wValue = Vector3.Cross(a - p, b - p).magnitude / 2 / area;
        text.text = @$"AB⋅e⃗= {determinant}";
    }
}
