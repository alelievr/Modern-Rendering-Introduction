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

    public void Initialize(Vector3 start, Vector3 end, Color color)
    {
        this.start = start;
        this.end = end;
        this.color = color;

        Vector3 direction = end - start;

        float arrowOffset = arrowHeadHeight * scale; // Take in account the size of the arrow head
        arrowHead.transform.rotation = Quaternion.LookRotation(direction) * Quaternion.Euler(90, 0, 0);
        arrowHead.transform.localPosition = end - direction.normalized * arrowOffset;
        arrowHead.transform.localScale = Vector3.one * scale;

        arrowTail.transform.localPosition = start;
        arrowTail.transform.rotation = Quaternion.LookRotation(direction) * Quaternion.Euler(90, 0, 0); // model is rotated 90 degrees
        arrowTail.transform.localScale = new Vector3(width * scale, direction.magnitude - arrowOffset, width * scale);

        if (!EditorApplication.isPlaying)
            UpdateColors();
    }

    void UpdateColors()
    {
        if (EditorApplication.isPlaying)
        {
            arrowHead.GetComponent<MeshRenderer>().material.color = color;
            arrowTail.GetComponent<MeshRenderer>().material.color = color;
        }
        else
        {
            arrowHead.GetComponent<MeshRenderer>().sharedMaterial.color = color;
            arrowTail.GetComponent<MeshRenderer>().sharedMaterial.color = color;
        }
    }

    void OnEnable() => UpdateColors();

    void Update()
    {
        Initialize(start, end, color);
    }
}
