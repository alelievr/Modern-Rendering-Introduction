using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class RenderPipelineVisibilityBuffer : MonoBehaviour
{
    public GameObject gBufferObjects;
    public GameObject forwardObjects;
    public TMP_Text visibilityBufferText;
    public TMP_Text deferredMaterialsText;
    public TMP_Text deferredLightingText;
    public GameObject visibilityBufferUIView;
    public GameObject gBufferUIView;
    public float drawDelay = 0.05f;
    public float delayBetweenPasses = 1.0f;

    public Shader gBuffer0Shader;
    public Shader gBuffer1Shader;
    public Shader gBuffer2Shader;
    public Shader depthShader;
    public Shader triangleIDShader;
    public Shader drawIDShader;

    public RenderTexture triangleID;
    public RenderTexture drawID;
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
        visibilityBufferText.enabled = false;
        deferredLightingText.enabled = false;
        deferredMaterialsText.enabled = false;

        StartCoroutine(DrawPass());
    }

    IEnumerator DrawPass()
    {
        // Simulate GBuffer rendering
        Camera.main.clearFlags = CameraClearFlags.SolidColor;
        Camera.main.backgroundColor = Color.black;
        visibilityBufferText.enabled = true;

        Camera.main.targetTexture = gBuffer0;

        visibilityBufferUIView.SetActive(true);
        gBufferUIView.SetActive(false);
    
        foreach (Transform child in gBufferObjects.transform)
        {
            child.gameObject.SetActive(true);
            var drawIdBlock = new MaterialPropertyBlock();
            drawIdBlock.SetColor("_DrawIDColor", Random.ColorHSV(0, 1, 1, 1, 1, 1));
            child.gameObject.GetComponent<MeshRenderer>().SetPropertyBlock(drawIdBlock);
            RenderVisibilityBuffer();
            yield return new WaitForSeconds(drawDelay);
        }

        yield return new WaitForSeconds(delayBetweenPasses);

        visibilityBufferUIView.SetActive(false);
        gBufferUIView.SetActive(true);
        RenderGBuffer();
        deferredMaterialsText.enabled = true;
        visibilityBufferText.enabled = false;

        yield return new WaitForSeconds(delayBetweenPasses);

        gBufferUIView.SetActive(false);

        // Simulate deferred lighting
        foreach (Transform child in forwardObjects.transform)
            child.gameObject.SetActive(true);

        deferredLightingText.enabled = true;
        deferredMaterialsText.enabled = false;
        Camera.main.clearFlags = CameraClearFlags.Skybox;
    }

    void RenderVisibilityBuffer()
    {
        Camera.main.SetReplacementShader(triangleIDShader, "");
        Camera.main.targetTexture = triangleID;
        Camera.main.Render();
        Camera.main.targetTexture = null;

        Camera.main.SetReplacementShader(drawIDShader, "");
        Camera.main.targetTexture = drawID;
        Camera.main.Render();
        Camera.main.targetTexture = null;

        Camera.main.SetReplacementShader(depthShader, "");
        Camera.main.targetTexture = depth;
        Camera.main.Render();
        Camera.main.targetTexture = null;

        Camera.main.ResetReplacementShader();
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
