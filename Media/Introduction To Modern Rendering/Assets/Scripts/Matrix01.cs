using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class Matrix01 : MonoBehaviour
{
    public GameObject go;
    public TMP_Text ui;
    public float translationSize = 4;
    public float timeScale = 4;

    Vector3 startPosition;
    
    void Start()
    {
        startPosition = go.transform.position;
    }

    void Update()
    {
        float t = Mathf.Sin(Time.timeSinceLevelLoad * timeScale) * translationSize;

        go.transform.position = startPosition + Vector3.right * t;
        ui.text = $"Translation: ({t:F1}, 0, 0)";
    }
}
