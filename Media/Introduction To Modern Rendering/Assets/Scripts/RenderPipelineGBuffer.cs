using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class RenderPipelineGBuffer : MonoBehaviour
{
    public GameObject gBufferObjects;
    public GameObject forwardObjects;
    public TMP_Text gBufferText;
    public TMP_Text deferredLightingText;
    public GameObject gBufferUIView;
    public float drawDelay = 0.05f;
    public float delayBetweenPasses = 1.0f;

    public Shader gBuffer0Shader;
    public Shader gBuffer1Shader;
    public Shader gBuffer2Shader;
    public Shader depthShader;

    public RenderTexture gBuffer0;
    public RenderTexture gBuffer1;
    public RenderTexture gBuffer2;
    public RenderTexture depth;

    void Start()
    {
        foreach (Transform child in gBufferObjects.transform)
            child.gameObject.SetActive(false);
        foreach (Transform child in forwardObjects.transform)
            child.gameObject.SetActive(false);
        gBufferText.enabled = false;
        deferredLightingText.enabled = false;

        StartCoroutine(DrawPass());
    }

    IEnumerator DrawPass()
    {
        // Simulate GBuffer rendering
        Camera.main.clearFlags = CameraClearFlags.SolidColor;
        Camera.main.backgroundColor = Color.black;
        gBufferText.enabled = true;

        Camera.main.targetTexture = gBuffer0;

        gBufferUIView.SetActive(true);

        foreach (Transform child in gBufferObjects.transform)
        {
            child.gameObject.SetActive(true);
            RenderGBuffer();
            yield return new WaitForSeconds(drawDelay);
        }

        yield return new WaitForSeconds(delayBetweenPasses);

        foreach (Transform child in gBufferObjects.transform)
            child.gameObject.SetActive(false);

        gBufferUIView.SetActive(false);

        // Simulate deferred lighting
        foreach (Transform child in forwardObjects.transform)
            child.gameObject.SetActive(true);

        deferredLightingText.enabled = true;
        gBufferText.enabled = false;
        Camera.main.clearFlags = CameraClearFlags.Skybox;
    }

    void RenderGBuffer()
    {
        Camera.main.SetReplacementShader(gBuffer0Shader, "");
        Camera.main.targetTexture = gBuffer0;
        Camera.main.Render();
        Camera.main.targetTexture = null;

        Camera.main.SetReplacementShader(gBuffer1Shader, "");
        Camera.main.targetTexture = gBuffer1;
        Camera.main.Render();
        Camera.main.targetTexture = null;

        Camera.main.SetReplacementShader(gBuffer2Shader, "");
        Camera.main.targetTexture = gBuffer2;
        Camera.main.Render();
        Camera.main.targetTexture = null;

        Camera.main.SetReplacementShader(depthShader, "");
        Camera.main.targetTexture = depth;
        Camera.main.Render();
        Camera.main.targetTexture = null;

        Camera.main.ResetReplacementShader();
    }
}
