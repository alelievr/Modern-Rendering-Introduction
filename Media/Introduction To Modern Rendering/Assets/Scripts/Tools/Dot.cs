using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Dot : MonoBehaviour
{
    public float size = 0.1f;
    public Color color = Color.white;

    Material material;

    void OnEnable()
    {
        transform.localScale = Vector3.one * size;
        material = GetComponentInChildren<MeshRenderer>().material;
        material.color = color;
    }

    void Update()
    {
        transform.localScale = Vector3.one * size;
        material.color = color;
    }
}
