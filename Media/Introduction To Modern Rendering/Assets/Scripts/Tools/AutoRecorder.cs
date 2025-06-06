#if UNITY_EDITOR
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor.Recorder;
using UnityEditor.Recorder.Encoder;
using UnityEditor.Recorder.Input;
using System.IO;
using UnityEngine.SceneManagement;
using System.Linq;
using System;

public class AutoRecorder : MonoBehaviour
{
    [Serializable]
    public struct OverrideResolution
    {
        public bool enabled;
        public int width;
        public int height;
    }

    public enum Mode
    {
        Picture,
        Gif,
    }

    public enum RecordStopMode
    {
        TimeInSeconds,
        FrameCount,
    }

    [Header("Recording Mode")]
    public Mode mode;
    public bool recordOnPlay = true;

    [Header("Recording Stop Condition")]
    public RecordStopMode recordStopMode = RecordStopMode.TimeInSeconds;
    public float recordingTimeInSeconds = 5.0f;
    public int recordingFrameCount = 150;

    [Header("Recording Settings")]
    public float frameRate = 30.0f;
    public uint gifQuality = 40;
    public float waitBeforeRecording = 0.0f;
    public OverrideResolution overrideResolution;

    RecorderController m_RecorderController;

    float startTime;
    int startFrame;

    void OnEnable()
    {
        startTime = 1e20f;
        var controllerSettings = ScriptableObject.CreateInstance<RecorderControllerSettings>();
        m_RecorderController = new RecorderController(controllerSettings);

        // Setup Recording
        controllerSettings.AddRecorderSettings(CreateRecorder());
        controllerSettings.SetRecordModeToManual();
        controllerSettings.FrameRate = frameRate;
        controllerSettings.CapFrameRate = true;
        controllerSettings.FrameRatePlayback = FrameRatePlayback.Constant;

        StartCoroutine(StartRecording(controllerSettings));
    }

    IEnumerator StartRecording(RecorderControllerSettings controllerSettings)
    {
        yield return new WaitForSeconds(waitBeforeRecording);

        RecorderOptions.VerboseMode = false;
        m_RecorderController.PrepareRecording();

        if (recordOnPlay)
        {
            m_RecorderController.StartRecording();
            startTime = Time.timeSinceLevelLoad;
            startFrame = Time.frameCount;
        }

        Debug.Log($"Started recording for file {controllerSettings.RecorderSettings.First().OutputFile}");
    }

    RecorderSettings CreateRecorder()
    {
        var mediaOutputFolder = Path.Combine(Application.dataPath, "..", "..", "..", "docs", "assets", "Recordings");

        switch (mode)
        {
            case Mode.Picture:
                var i = ScriptableObject.CreateInstance<ImageRecorderSettings>();
                i.name = "PNG Recorder";
                i.Enabled = true;

                i.CaptureAlpha = false;
                i.OutputFormat = ImageRecorderSettings.ImageRecorderOutputFormat.PNG;

                i.imageInputSettings = new GameViewInputSettings
                {
                    OutputWidth = overrideResolution.enabled ? overrideResolution.width : 1920,
                    OutputHeight = overrideResolution.enabled ? overrideResolution.height : 1080
                };

                // Simple file name (no wildcards) so that FileInfo constructor works in OutputFile getter.
                i.OutputFile = Path.Combine(mediaOutputFolder, SceneManager.GetActiveScene().name);
                return i;
            case Mode.Gif:
                var m = ScriptableObject.CreateInstance<MovieRecorderSettings>();
                m.name = "GIF Recorder";
                m.Enabled = true;

                // This example performs an MP4 recording
                m.EncoderSettings = new GifEncoderSettings
                {
                    Quality = gifQuality,
                };
                m.CaptureAlpha = false;

                m.ImageInputSettings = new GameViewInputSettings
                {
                    OutputWidth = 1920,
                    OutputHeight = 1080
                };

                // Simple file name (no wildcards) so that FileInfo constructor works in OutputFile getter.
                m.OutputFile = Path.Combine(mediaOutputFolder, SceneManager.GetActiveScene().name);
                return m;
            default:
                return null;
        }
    }

    void Update()
    {
        float d = Time.timeSinceLevelLoad - startTime;
        if (mode == Mode.Picture && d > 1.0f / 30.0f)
        {
            m_RecorderController.StopRecording();
            Debug.Log("Recording finished");
            enabled = false;
        }

        switch (recordStopMode)
        {
            case RecordStopMode.TimeInSeconds:
                if (d > recordingTimeInSeconds)
                {
                    m_RecorderController.StopRecording();
                    Debug.Log("Recording finished");
                    enabled = false;
                }
                break;
            case RecordStopMode.FrameCount:
                if (m_RecorderController.IsRecording() && Time.frameCount - startFrame >= recordingFrameCount)
                {
                    m_RecorderController.StopRecording();
                    Debug.Log("Recording finished");
                    enabled = false;
                }
                break;
        }
    }
}
#endif