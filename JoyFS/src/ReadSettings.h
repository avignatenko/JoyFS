#pragma once

#include <spdlog/spdlog.h>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>
#include <map>


struct JoysticksSettings
{
    enum class Operation
    {
        Delta,
        Set
    };
    
    struct Button
    {
        Operation operation;
        int offset;
        int size;
        int delta;
    };

    struct Joystick
    {
        std::map<int, Button> buttons;
    };

    std::map<int, Joystick> joysticks;
};

boost::property_tree::ptree readSettings(const std::filesystem::path& file);

