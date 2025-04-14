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
            GraphicsSettings.defaultRenderPipeline = Resources.Load<RenderPipelineAsset>("HDRP");
        else
            GraphicsSettings.defaultRenderPipeline = null;

#if UNITY_EDITOR
        EditorSceneManager.sceneClosing -= ResetRenderPipeline;
        EditorSceneManager.sceneClosing += ResetRenderPipeline;
#endif
    }

    void ResetRenderPipeline(Scene scene, bool removingScene)
    {
        GraphicsSettings.defaultRenderPipeline = null;
#if UNITY_EDITOR
        EditorSceneManager.sceneClosing -= ResetRenderPipeline;
#endif
    }
}
