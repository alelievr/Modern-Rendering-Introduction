using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RasterizationStencil : MonoBehaviour
{
    public Material white;
    public Material black;
    public MeshRenderer targetObject;
    public MeshRenderer lens;
    
    public bool showStencilRead;
    public bool showStencilWrite;

    List<MeshRenderer> renderers = new();
    
    void Start()
    {
        renderers = new List<MeshRenderer>(FindObjectsOfType<MeshRenderer>());
    }

    void Update()
    {
        if (showStencilRead)
        {
            foreach (var renderer in renderers)
                renderer.material = black;
            lens.material = white;
        }
        else if (showStencilWrite)
        {
            foreach (var renderer in renderers)
                renderer.material = black;
            targetObject.material = white;
            lens.gameObject.SetActive(false);
        }
    }
}
