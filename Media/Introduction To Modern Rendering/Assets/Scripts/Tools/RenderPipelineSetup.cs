using System.Collections;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor.SceneManagement;
#endif
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

#if UNITY_EDITOR
        EditorSceneManager.sceneClosing -= ResetRenderPipeline;
        EditorSceneManager.sceneClosing += ResetRenderPipeline;
#endif
    }

    void ResetRenderPipeline(Scene scene, bool removingScene)
    {
        GraphicsSettings.renderPipelineAsset = null;
#if UNITY_EDITOR
        EditorSceneManager.sceneClosing -= ResetRenderPipeline;
#endif
    }
}
