using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[ExecuteAlways]
public class Dot : MonoBehaviour
{
    public float size = 0.1f;
    public Color color = Color.white;

    Material material;

    MaterialPropertyBlock materialPropertyBlock;
    MeshRenderer meshRenderer;

    void OnEnable()
    {
        if (materialPropertyBlock == null)
            materialPropertyBlock = new MaterialPropertyBlock();

        var meshRenderer = GetComponentInChildren<MeshRenderer>();
        transform.localScale = Vector3.one * size;
        materialPropertyBlock.SetColor("_Color", color);
        meshRenderer.SetPropertyBlock(materialPropertyBlock);
    }

    void Update()
    {
        if (meshRenderer == null)
            meshRenderer = GetComponentInChildren<MeshRenderer>();
        transform.localScale = Vector3.one * size;
        materialPropertyBlock.SetColor("_Color", color);
        meshRenderer.SetPropertyBlock(materialPropertyBlock);
    }
}
