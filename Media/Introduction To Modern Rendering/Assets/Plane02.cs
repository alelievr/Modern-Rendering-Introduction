using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Plane02 : MonoBehaviour
{
    public Arrow l;
    public AnchoredText textL;
    public TMPro.TextMeshProUGUI text;
    public BackgroundGrid grid;

    void Start()
    {
        textL.SetAnchor(l); 
        textL.SetText("l");
    }

    void Update()
    {
        grid.transform.position = new Vector3(3, 0, 0);
        Vector3 pointOnCircle = new Vector3(Mathf.Cos(Time.time), Mathf.Sin(Time.time), 0);
        l.Initialize(Vector3.zero, pointOnCircle, Color.white);
        text.text = "∥l∥= 1\n∥n∥= 1\nn⋅l = " + Vector3.Dot(Vector3.up, pointOnCircle).ToString("F2");
    }
}
