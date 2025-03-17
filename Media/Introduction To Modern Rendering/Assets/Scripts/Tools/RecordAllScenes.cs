using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using UnityEditor.SceneManagement;
using System.IO;
using Unity.EditorCoroutines.Editor;  // Requires the Editor Coroutines package

public class RecordAllScenes
{
    static List<string> scenes = new();
    static int currentSceneIndex = 0;
    static EditorCoroutine coroutine;

    [MenuItem("Modern Rendering/Record All Scenes")]
    public static void RecordAll()
    {
        scenes.Clear();
        foreach (var assetGUID in AssetDatabase.FindAssets("t:Scene", new[] { "Assets/Scenes" }))
        {
            var path = AssetDatabase.GUIDToAssetPath(assetGUID);
            scenes.Add(Path.GetFileNameWithoutExtension(path));
        }

        scenes.Sort();

        currentSceneIndex = 0;
        EditorApplication.playModeStateChanged -= OnPlayModeStateChanged;
        EditorApplication.playModeStateChanged += OnPlayModeStateChanged;
        LoadNextScene();
    }

    [MenuItem("Modern Rendering/Stop Record All Scenes")]
    public static void StopRecordAll()
    {
        EditorApplication.isPlaying = false;
        EditorApplication.playModeStateChanged -= OnPlayModeStateChanged;
        EditorCoroutineUtility.StopCoroutine(coroutine);
    }

    static void LoadNextScene()
    {
        if (currentSceneIndex >= scenes.Count)
        {
            Debug.Log("Finished processing all scenes.");
            EditorApplication.playModeStateChanged -= OnPlayModeStateChanged;
            return;
        }

        string scenePath = $"Assets/Scenes/{scenes[currentSceneIndex]}.unity";
        EditorSceneManager.OpenScene(scenePath);
        Debug.Log("Loaded scene: " + scenePath);
        // Enter play mode
        EditorApplication.isPlaying = true;
    }

    static void OnPlayModeStateChanged(PlayModeStateChange state)
    {
        if (state == PlayModeStateChange.EnteredPlayMode)
        {
            // Delay one frame to ensure scene objects are initialized
            EditorApplication.delayCall += () =>
            {
                // Attempt to locate the AutoRecorder component in the scene.
                AutoRecorder recorder = Object.FindFirstObjectByType<AutoRecorder>();
                float waitTime = 1f; 
                if (recorder != null || recorder.mode != AutoRecorder.Mode.Gif)
                {
                    waitTime = recorder.recordingTimeInSeconds + recorder.waitBeforeRecording + 2f;
                    Debug.Log("AutoRecorder found with recording time: " + recorder.recordingTimeInSeconds);
                }

                // Wait then exit play mode.
                coroutine = EditorCoroutineUtility.StartCoroutineOwnerless(WaitAndExit(waitTime, recorder));
            };
        }
        else if (state == PlayModeStateChange.ExitingPlayMode)
        {
            // When play mode is exited, load the next scene.
            LoadNextSceneDelay();
        }
    }

    static IEnumerator WaitAndExit(float waitTime, AutoRecorder recorder)
    {
        // convert time to frame count:
        int frameCount = Mathf.CeilToInt(waitTime * recorder.frameRate);
        int currentFrameCount = Time.frameCount;

        while (Time.frameCount < currentFrameCount + frameCount)
            yield return null;

        EditorApplication.isPlaying = false;
    }

    static void LoadNextSceneDelay()
    {
        EditorApplication.delayCall += () =>
        {
            if (EditorApplication.isPlaying)
                LoadNextSceneDelay();
            else
            {
                currentSceneIndex++;
                LoadNextScene();
            }
        };
    }
}
