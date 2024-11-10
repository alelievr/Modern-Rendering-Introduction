#include "renderdoc_app.h"
#include "RenderDoc.hpp"
#include <windows.h>
#include <libloaderapi.h>
#include <cassert>
#include <iostream>

static RENDERDOC_API_1_1_2* rdoc_api = NULL;
bool RenderDoc::captureNextFrame = false;
bool RenderDoc::isLoaded = false;

void RenderDoc::EnqueueCaptureNextFrame()
{
    captureNextFrame = true;
}

void RenderDoc::LoadRenderDoc()
{
    HMODULE mod = GetModuleHandleA("renderdoc.dll");

    if (mod == nullptr)
        mod = LoadLibraryA("renderdoc.dll");

    // At init, on windows
    if (mod != nullptr)
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI =
            (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
        assert(ret == 1);
        isLoaded = true;
    }
    else
    {
        DWORD errorMessageID = ::GetLastError();
        if (errorMessageID == 0) {
            // No error has occurred.
            return;
        }

        LPSTR messageBuffer = nullptr;

        // Format the error message from the system
        size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&messageBuffer, 0, NULL);

        std::cerr << "Error in LoadRenderDoc " << ": " << messageBuffer << std::endl;

        // Free the buffer allocated by FormatMessage
        LocalFree(messageBuffer);
    }
}

void RenderDoc::StartFrameCapture()
{
    if (captureNextFrame)
    {
        // To start a frame capture, call StartFrameCapture.
        // You can specify NULL, NULL for the device to capture on if you have only one device and
        // either no windows at all or only one window, and it will capture from that device.
        // See the documentation below for a longer explanation
        if (rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);
    }
}

void RenderDoc::StartCaptureImmediately()
{
    captureNextFrame = true;
    StartFrameCapture();
}

void RenderDoc::EndFrameCapture()
{
    if (captureNextFrame)
    {
        // To end a frame capture, call EndFrameCapture.
        // This will finalize the capture and allow you to open the capture in RenderDoc.
        // See the documentation below for a longer explanation
        if (rdoc_api)
        {
            rdoc_api->EndFrameCapture(NULL, NULL);
            rdoc_api->LaunchReplayUI(1, NULL);
        }
        captureNextFrame = false;
    }
}
