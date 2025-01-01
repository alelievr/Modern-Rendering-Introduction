using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class RenderPipelineDepthPrepass : MonoBehaviour
{
    public GameObject depthPrepassObjects;
    public GameObject forwardObjects;
    public Material showDepthMaterial;
    public TMP_Text depthPrepassText;
    public TMP_Text forwardText;
    public float drawDelay = 0.05f;

    void Start()
    {
        foreach (Transform child in depthPrepassObjects.transform)
            child.gameObject.SetActive(false);
        foreach (Transform child in forwardObjects.transform)
            child.gameObject.SetActive(false);
        depthPrepassText.enabled = false;
        forwardText.enabled = false;

        StartCoroutine(DrawPass());
    }

    IEnumerator DrawPass()
    {
        // Simulate depth pre-pass rendering
        Camera.main.clearFlags = CameraClearFlags.SolidColor;
        Camera.main.backgroundColor = Color.black;
        depthPrepassText.enabled = true;

        foreach (Transform child in depthPrepassObjects.transform)
        {
            child.gameObject.GetComponent<MeshRenderer>().sharedMaterial = showDepthMaterial;
            child.gameObject.SetActive(true);
            yield return new WaitForSeconds(drawDelay);
        }

        foreach (Transform child in depthPrepassObjects.transform)
            child.gameObject.SetActive(false);

        // Simulate forward rendering
        forwardText.enabled = true;
        depthPrepassText.enabled = false;
        Camera.main.clearFlags = CameraClearFlags.Skybox;

        foreach (Transform child in forwardObjects.transform)
        {
            child.gameObject.SetActive(true);
            yield return new WaitForSeconds(drawDelay);
        }
    }
}
