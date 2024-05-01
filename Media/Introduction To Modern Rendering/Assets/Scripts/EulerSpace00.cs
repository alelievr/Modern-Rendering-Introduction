using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class EulerSpace00 : MonoBehaviour
{
    public float rotationRadius = 2;
    public float timeScale = 2;

    Dot dot;
    Line dottedLineX;
    Line dottedLineY;
    Line dottedLineZ;
    AnchoredText anchoredTextX;
    AnchoredText anchoredTextY;
    AnchoredText anchoredTextZ;

    void Start()
    {
        dot = Instantiate(Resources.Load("Dot")).GetComponent<Dot>();
        dottedLineX = Instantiate(Resources.Load("Line")).GetComponent<Line>();
        dottedLineY = Instantiate(Resources.Load("Line")).GetComponent<Line>();
        dottedLineZ = Instantiate(Resources.Load("Line")).GetComponent<Line>();
        anchoredTextX = Instantiate(Resources.Load("AnchoredText")).GetComponent<AnchoredText>();
        anchoredTextY = Instantiate(Resources.Load("AnchoredText")).GetComponent<AnchoredText>();
        anchoredTextZ = Instantiate(Resources.Load("AnchoredText")).GetComponent<AnchoredText>();

        dottedLineX.color = Utils.GetXAxisColor();
        dottedLineX.dotted = true;
        dottedLineY.color = Utils.GetYAxisColor();
        dottedLineY.dotted = true;
        dottedLineZ.color = Utils.GetZAxisColor();
        dottedLineZ.dotted = true;

        anchoredTextX.SetAnchor(dottedLineX);
        anchoredTextY.SetAnchor(dottedLineY);
        anchoredTextZ.SetAnchor(dottedLineZ);
    }

    void Update()
    {
        // Circular motion
        var dotPos = new Vector3(Mathf.Cos(Time.time / timeScale) * rotationRadius, Mathf.Sin(Time.time / timeScale) * rotationRadius, Mathf.Sin(Time.time / timeScale) * rotationRadius);
        dot.transform.position = dotPos;
        dottedLineX.UpdateLine(new Vector3(0, dotPos.y, dotPos.z), dotPos);
        dottedLineY.UpdateLine(new Vector3(dotPos.x, 0, dotPos.z), dotPos);
        dottedLineZ.UpdateLine(new Vector3(dotPos.x, dotPos.z, 0), dotPos);

        anchoredTextX.SetText($"X: {dotPos.x:+0.0;-0.0}", Utils.GetXAxisColor());
        anchoredTextY.SetText($"Y: {dotPos.y:+0.0;-0.0}", Utils.GetYAxisColor());
        anchoredTextZ.SetText($"Z: {dotPos.z:+0.0;-0.0}", Utils.GetZAxisColor());
    }
}
