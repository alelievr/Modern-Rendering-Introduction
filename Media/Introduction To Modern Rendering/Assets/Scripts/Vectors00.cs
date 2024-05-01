using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using static Unity.Mathematics.math;

public class Vectors00 : MonoBehaviour
{
    public float radius = 2;
    public int pointCount;

    void Start()
    {
        Random.InitState(45);

        for (int i = 0; i < 10; i++)
        {
            var color = Random.ColorHSV(0, 1, 0.6f, 0.9f, 0.5f, 0.9f);
            var dot = Instantiate(Resources.Load("Dot")).GetComponent<Dot>();
            dot.size = 0.05f;
            dot.color = color;
            var pos = dot.transform.position = round(Random.insideUnitSphere * radius);

            var text = Instantiate(Resources.Load("AnchoredText")).GetComponent<AnchoredText>();
            text.SetAnchor(pos + Vector3.up * 0.2f);
            text.SetText($"({pos.x:0}, {pos.y:0}, {pos.z:0})", color);
        }
    }
}
