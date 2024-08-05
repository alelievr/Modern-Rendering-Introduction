using System;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class Matrix03 : MonoBehaviour
{
    public Dot dot;
    public float timeScale = 6;
    public TMP_Text text;

    Vector3 startPos;

    void Start()
    {
        startPos = dot.transform.position;
    }

    // Update is called once per frame
    void Update()
    {
        dot.transform.position = Quaternion.Euler(Time.timeSinceLevelLoad / timeScale * 360f, 0, 0) * startPos;
        text.text = $"Position: ({dot.transform.position.x:F1}, {dot.transform.position.y:F1}, {dot.transform.position.z:F1})";
    }
}
