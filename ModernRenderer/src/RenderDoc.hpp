#pragma once

#include <string>


class RenderDoc
{
private:
    static bool captureNextFrame;
    static bool isLoaded;

public:
    static void LoadRenderDoc();
    static void EnqueueCaptureNextFrame();
    static void StartFrameCapture();
    static void StartCaptureImmediately();
    static void EndFrameCapture();
};