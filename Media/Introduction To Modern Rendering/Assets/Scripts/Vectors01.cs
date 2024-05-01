using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using static Unity.Mathematics.math;

public class Vectors01 : MonoBehaviour
{
    public float radius = 2;
    public int pointCount;
    public float timeToCompleteRotation = 5.0f;

    AnchoredText text;
    Arrow arrow;
    Vector3 startPos;
    Dot dot;
    Color color;

    void Start()
    {
        Random.InitState(45);

        color = Random.ColorHSV(0, 1, 0.6f, 0.9f, 0.5f, 0.9f);
        arrow = Instantiate(Resources.Load("Arrow")).GetComponent<Arrow>();
        arrow.scale = 0.2f;
        arrow.color = color;
        startPos = new Vector3(1, 1, 1).normalized * radius;

        dot = Instantiate(Resources.Load("Dot")).GetComponent<Dot>();
        dot.color = color;
        dot.size = 0.02f;

        text = Instantiate(Resources.Load("AnchoredText")).GetComponent<AnchoredText>();
        text.color = color;
        text.SetAnchor(arrow);
    }

    void Update()
    {
        // Move the point on the surface of a sphere using time
        float t = Time.time * 360 / timeToCompleteRotation;
        var pos = Quaternion.Euler(t, t, 0) * startPos;
        
        dot.transform.position = pos;
        arrow.Initialize(Vector3.zero, pos, color);
        text.SetText($"({pos.x:0.0}, {pos.y:0.0}, {pos.z:0.0})");
    }
}
