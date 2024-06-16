using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Unity.VisualScripting;
using TMPro;

public class TriangleIntersection00 : MonoBehaviour
{
    public Vector3 a, b, c;
    public Vector3 p;

    public Color pointColor = Color.white;
    public float pointSize = 0.1f;
    public float lineWidth = 0.025f;

    public float animationTime = 6;

    public TMP_Text text;

    Triangle mainTriangle;
    Triangle triangleA, triangleB, triangleC;

    AnchoredText u, v, w;
    AnchoredText pText;

    Dot pointP;

    void Start()
    {
        mainTriangle = SpawnTriangle(a, b, c, Color.clear, true, true, true);
        triangleA = SpawnTriangle(a, b, p, Utils.GetZAxisColor());
        triangleB = SpawnTriangle(b, c, p, Utils.GetXAxisColor());
        triangleC = SpawnTriangle(c, a, p, Utils.GetYAxisColor());

        pointP = Instantiate(Resources.Load("Dot")).GetComponent<Dot>();
        pointP.color = pointColor;
        pointP.size = pointSize;

        u = Instantiate(Resources.Load("AnchoredText")).GetComponent<AnchoredText>();
        v = Instantiate(Resources.Load("AnchoredText")).GetComponent<AnchoredText>();
        w = Instantiate(Resources.Load("AnchoredText")).GetComponent<AnchoredText>();
        pText = Instantiate(Resources.Load("AnchoredText")).GetComponent<AnchoredText>();

        u.SetText("w");
        v.SetText("u");
        w.SetText("v");
        pText.SetText("P");

        u.SetAnchor(triangleA);
        v.SetAnchor(triangleB);
        w.SetAnchor(triangleC);
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
        triangleA.c = p;
        triangleB.c = p;
        triangleC.c = p;

        float area = Vector3.Cross(b - a, c - a).magnitude / 2;
        float uValue = Vector3.Cross(b - p, c - p).magnitude / 2 / area;
        float vValue = Vector3.Cross(c - p, a - p).magnitude / 2 / area;
        float wValue = Vector3.Cross(a - p, b - p).magnitude / 2 / area;
        text.text = @$"area ΔABC = 1
<color=#{UnityEngine.ColorUtility.ToHtmlStringRGB(Utils.GetXAxisColor())}>area ΔABP = {uValue:0.00}</color>
<color=#{UnityEngine.ColorUtility.ToHtmlStringRGB(Utils.GetYAxisColor())}>area ΔBCP = {vValue:0.00}</color>
<color=#{UnityEngine.ColorUtility.ToHtmlStringRGB(Utils.GetZAxisColor())}>area ΔCAP = {wValue:0.00}</color>";
    }
}
