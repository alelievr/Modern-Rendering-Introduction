using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor.SceneManagement;
using UnityEngine.SceneManagement;
using UnityEngine.Rendering;

[ExecuteAlways]
public class RenderPipelineSetup : MonoBehaviour
{
    public enum RenderPipeline
    {
        Builtin,
        HDRP
    }

    public RenderPipeline renderPipeline;

    void OnEnable()
    {
        if (renderPipeline == RenderPipeline.HDRP)
            GraphicsSettings.renderPipelineAsset = Resources.Load<RenderPipelineAsset>("HDRP");
        else
            GraphicsSettings.renderPipelineAsset = null;

        EditorSceneManager.sceneClosing -= ResetRenderPipeline;
        EditorSceneManager.sceneClosing += ResetRenderPipeline;
    }

    void ResetRenderPipeline(Scene scene, bool removingScene)
    {
        GraphicsSettings.renderPipelineAsset = null;
        EditorSceneManager.sceneClosing -= ResetRenderPipeline;
    }
}
