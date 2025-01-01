using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UIGifHack : MonoBehaviour
{
    public Material gifMaterial;
    public Color baseColor = Color.black;
    public float amplitude = 0.05f;
    public float speed = 1f;

    void Start()
    {
        
    }

    void Update()
    {
        // Slightly change the alpha of the material to animate it
        // tricks the gif recorder into thinking the image changes and forces it to respect the timings between frames
        baseColor.a = Mathf.PingPong(Time.time, speed) * amplitude;
        gifMaterial.SetColor("_Color", baseColor);
    }
}
