using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Texturing_UpdateDecalMaterialValue : MonoBehaviour
{
    public float value = 0.5f;
    public Material mat;
    public GameObject quad;

    void Update()
    {
        mat.SetFloat("_Distance", value);
        quad.transform.localPosition = new Vector3(0, 0, -value * 2);
    }
}
