using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

[ExecuteAlways]
public class Arrow : MonoBehaviour
{
    public Color color = Color.white;

    public Vector3 start;
    public Vector3 end;

    public float width = 0.25f;
    public float scale = 1;

    public GameObject arrowHead;
    public GameObject arrowTail;

    const float arrowHeadHeight = 0.8157f;

    MaterialPropertyBlock materialPropertyBlock;

    public void Initialize(Vector3 start, Vector3 end, Color color = default)
    {
        this.start = start;
        this.end = end;
        if (color != default)
            this.color = color;

        Vector3 direction = end - start;

        float arrowOffset = arrowHeadHeight * scale; // Take in account the size of the arrow head
        arrowHead.transform.rotation = Quaternion.LookRotation(direction) * Quaternion.Euler(90, 0, 0);
        arrowHead.transform.localPosition = end - direction.normalized * arrowOffset;
        arrowHead.transform.localScale = Vector3.one * scale;

        arrowTail.transform.localPosition = start;
        arrowTail.transform.rotation = Quaternion.LookRotation(direction) * Quaternion.Euler(90, 0, 0); // model is rotated 90 degrees
        arrowTail.transform.localScale = new Vector3(width * scale, direction.magnitude - arrowOffset, width * scale);

        if (color != default)
            UpdateColors();
    }

    void UpdateColors()
    {
        if (materialPropertyBlock == null)
            materialPropertyBlock = new MaterialPropertyBlock();

        materialPropertyBlock.SetColor("_Color", color);
        arrowHead.GetComponent<MeshRenderer>().SetPropertyBlock(materialPropertyBlock);
        arrowTail.GetComponent<MeshRenderer>().SetPropertyBlock(materialPropertyBlock);
    }

    void OnEnable() => UpdateColors();

    void Update()
    {
        Initialize(start, end, color);
    }
}
