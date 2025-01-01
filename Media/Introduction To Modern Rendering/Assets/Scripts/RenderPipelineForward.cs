using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RenderPipelineForward : MonoBehaviour
{
    public GameObject objectParent;
    public float drawDelay = 0.05f;

    void Start()
    {
        foreach (Transform child in objectParent.transform)
            child.gameObject.SetActive(false);
        StartCoroutine(DrawPass());
    }

    IEnumerator DrawPass()
    {
        foreach (Transform child in objectParent.transform)
        {
            child.gameObject.SetActive(true);
            yield return new WaitForSeconds(drawDelay);
        }
    }
}
