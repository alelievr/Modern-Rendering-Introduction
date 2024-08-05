using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using TMPro;

public class Matrix04 : MonoBehaviour
{
    public TMP_Text text;
    public float timeScale = 8.0f;
    public Transform obj;
    public float scale = 1.0f; 
    public bool nonUniform = false;

    void Start()
    {
        
    }

    float F(float t)
    {
        float r = Mathf.Sin(Time.timeSinceLevelLoad / timeScale * 2 * Mathf.PI - Mathf.PI / 2 + t) * 0.5f + 0.5f;
        return r * scale;
    }

    void Update()
    {
        if (nonUniform)
        {
            obj.localScale = new Vector3(
                F(0),
                F(Mathf.PI / 3),
                F(Mathf.PI / 3 * 2)
                ) + Vector3.one;
        }
        else
        {
            obj.localScale = Vector3.one * (F(0) + 1);
        }

        text.text = $"Scale: ({obj.localScale.x:F1}, {obj.localScale.y:F1}, {obj.localScale.z:F1})";
    }
}
