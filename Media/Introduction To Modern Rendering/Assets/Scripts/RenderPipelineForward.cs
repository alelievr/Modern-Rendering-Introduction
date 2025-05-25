using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RenderPipelineForward : MonoBehaviour
{
    public GameObject objectParent;
    public float drawDelay = 0.05f;
    public int drawPerFrame = 1;

    void Start()
    {
        foreach (Transform child in objectParent.transform)
            child.gameObject.SetActive(false);
        StartCoroutine(DrawPass());
    }

    IEnumerator DrawPass()
    {
        foreach (Transform child in objectParent.transform)
            child.gameObject.SetActive(false);

        int countInFrame = 0;
        foreach (Transform child in objectParent.transform)
        {
            if (countInFrame >= drawPerFrame)
            {
                countInFrame = 0;
                if (drawDelay > 0)
                    yield return new WaitForSeconds(drawDelay);
            }
            child.gameObject.SetActive(true);
            countInFrame++;
        }
    }
}
