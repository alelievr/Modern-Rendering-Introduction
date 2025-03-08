#pragma once


#include "AppBox/AppBox.h"
#include "AppSettings/ArgsParser.h"
#include "Instance/Instance.h"
#include <vector>

class InputController : InputEvents
{
public:
    std::vector<InputEvents*> registeredEvents = {};
    
    InputController() {}

    void OnKey(int key, int action) override
    {
        for (auto event : registeredEvents)
			event->OnKey(key, action);
    }

    void OnMouse(bool first, double xpos, double ypos) override
    {
        for (auto event : registeredEvents)
            event->OnMouse(first, xpos, ypos);
    }

    void OnScroll(double xoffset, double yoffset) override
	{
		for (auto event : registeredEvents)
			event->OnScroll(xoffset, yoffset);
	}
};
