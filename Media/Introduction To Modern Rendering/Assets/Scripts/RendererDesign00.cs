using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RendererDesign00 : MonoBehaviour
{
    public GameObject[] objects;
    public float animationSpeed = 0.5f;
    public bool triggerDraw;
    Camera rtCamera;
    public Material screenMaterial;
    public bool flip = false;
    public bool skip = false;
    public bool forceDisplayRT = false;

    [System.NonSerialized]
    bool renderingObjects = false;

    // Start is called before the first frame update
    void Start()
    {
        rtCamera = GetComponent<Camera>();
        foreach (GameObject obj in objects)
            obj.SetActive(false);

        screenMaterial.SetTexture("_MainTex", Texture2D.blackTexture);
    }

    void Update()
    {
        if (forceDisplayRT)
            screenMaterial.SetTexture("_MainTex", rtCamera.targetTexture);
        if (triggerDraw && !renderingObjects)
        {
            StartCoroutine(RenderObjects());
            triggerDraw = false;
        }
    }

    IEnumerator RenderObjects()
    {
        renderingObjects = true;
        // Clear objects
        foreach (GameObject obj in objects)
            obj.SetActive(false);

        rtCamera.Render();
        yield return new WaitForSeconds(animationSpeed / objects.Length);

        foreach (GameObject obj in objects)
        {
            obj.SetActive(true);
            // Assign random color to each object
            obj.GetComponent<Renderer>().material.color = Random.ColorHSV(0f, 1f, 0.3f, 1f, 0.1f, 1f);
            rtCamera.Render();
            yield return new WaitForSeconds(animationSpeed / objects.Length);
        }

        while (!flip && !skip)
            yield return new WaitForEndOfFrame();
        
        if (skip)
        {
            renderingObjects = false;
            yield break;
        }

        screenMaterial.SetTexture("_MainTex", rtCamera.targetTexture);

        renderingObjects = false;
    }
}
