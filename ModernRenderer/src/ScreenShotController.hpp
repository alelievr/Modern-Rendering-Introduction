#pragma once
#include "AppBox/AppBox.h"
#include "Scene.hpp"
#include <atlimage.h>
#include <windows.h>

class ScreenShotController : InputEvents
{
private:
    HWND hwnd;
    HDC hdcClient;
    std::wstring sceneName;

public:
    ScreenShotController(HWND h, std::wstring name)
    {
        hwnd = h;
        hdcClient = GetDC(hwnd);
        sceneName = name;
    }

    void OnKey(int key, int action)
    {
        if (key == GLFW_KEY_P && action == GLFW_PRESS)
        {
            // Get the client area dimensions
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;

            // Create a compatible bitmap in CImage and copy the client area content
            CImage image;
            image.Create(width, height, 32);
            BitBlt(image.GetDC(), 0, 0, width, height, hdcClient, 0, 0, SRCCOPY);
            image.ReleaseDC();

            auto path = std::wstring(L"../Media/Recordings/") + sceneName.c_str() + L".png";
            image.Save(path.c_str(), Gdiplus::ImageFormatPNG);
        }
    }
};
