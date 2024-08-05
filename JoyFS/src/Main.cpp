
#include "ReadSettings.h"
#include "Sim.h"

#include <fmt/format.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#include <windows.h>

#include <stdio.h>
#include <filesystem>
#include <iostream>
#include <string>

using namespace std;

using boost::property_tree::ptree;
using std::filesystem::file_time_type;
using std::filesystem::path;

spdlog::level::level_enum toLogLevel(const std::string& levelStr)
{
    spdlog::level::level_enum level = spdlog::level::info;

    std::map<std::string, spdlog::level::level_enum> settingsStrToEnum = {
        {"trace", spdlog::level::trace}, {"debug", spdlog::level::debug}, {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},   {"err", spdlog::level::err},     {"critical", spdlog::level::critical},
        {"off", spdlog::level::off}};

    if (auto levelFound = settingsStrToEnum.find(levelStr); levelFound != settingsStrToEnum.end())
        level = levelFound->second;

    return level;
}

void initLogging(const boost::property_tree::ptree& settings)
{
    // init logging

    auto logLevelConsoleStr = boost::algorithm::to_lower_copy(settings.get<std::string>("LogLevelConsole"));
    auto logLevelConsole = toLogLevel(logLevelConsoleStr);

    auto logLevelFileStr = boost::algorithm::to_lower_copy(settings.get<std::string>("LogLevelFile"));
    auto logLevelFile = toLogLevel(logLevelFileStr);

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(logLevelConsole);

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("joyfs.log", true);
    file_sink->set_level(logLevelFile);

    auto logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list{console_sink, file_sink});

    logger->set_level(spdlog::level::trace);

    spdlog::set_default_logger(logger);

    spdlog::info("JoyFS logging started");
}

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
    int value;
};

struct Joystick
{
    std::map<int, Button> buttons;
};

int main(int argc, char** argv)
{
    try
    {
        path exeName = argv[0];
        path settingsPath = exeName.parent_path() / "Settings.json";

        ptree settings = readSettings(settingsPath);
        ptree appSettings = settings.get_child("App");
        ptree joySettings = settings.get_child("Joysticks");

        initLogging(appSettings);

        SDL_SetMainReady();

        if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
        {
            throw std::runtime_error(fmt::format("Couldn't initialize SDL: {}", SDL_GetError()));
        }

        spdlog::info("Enumeration of system joysticks:");
        for (int i = 0; i < SDL_NumJoysticks(); i++)
        {
            spdlog::info("  {} = {} ", i, SDL_JoystickNameForIndex(i));
        }

        SDL_JoystickEventState(SDL_ENABLE);

        std::map<SDL_JoystickID, Joystick> joysticks;
        for (const auto& joy : joySettings)
        {
            int id = boost::lexical_cast<int>(joy.first);
            spdlog::info("Capturing device {}", id);
            SDL_Joystick* joystick = SDL_JoystickOpen(id);
            Joystick joystickSettings;
            for (const auto& button : joy.second.get_child("Buttons"))
            {
                int id = boost::lexical_cast<int>(button.first);
                spdlog::info("Adding button {}", id);

                Button b;
                b.operation = Operation::Delta;  // fixme

                auto hexStr = button.second.get<std::string>("Offset");
                b.offset = std::stoul(hexStr, nullptr, 16);

                b.size = button.second.get<int>("Size");
                b.value = button.second.get<int>("Value");

                spdlog::info("Button {} settings: {}, {}, {}, {}", id, b.operation, b.offset, b.size, b.value);

                joystickSettings.buttons[id] = b;
            }
            joysticks[SDL_JoystickInstanceID(joystick)] = joystickSettings;
        }

        spdlog::info("Starting event processing cycle");

        bool end = false;
        int pollingDelayMs = appSettings.get<int>("PollingDelayMs");
        spdlog::info("Polling delay {}", pollingDelayMs);
        SDL_Event event;

        Sim sim;

        for (; !end;)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(pollingDelayMs));

            sim.process();

            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_JOYHATMOTION:
                {
                    // data->mutable_hat()->set_index(event.jhat.hat);
                    // data->mutable_hat()->set_value(event.jhat.value);
                    break;
                }
                case SDL_JOYBUTTONDOWN:
                case SDL_JOYBUTTONUP:
                {
                    spdlog::trace("Button event: joy {}, id {}, pressed {}", event.jbutton.which, event.jbutton.button,
                                  event.jbutton.state);
                    // get id
                    auto joy = joysticks.find(event.jbutton.which);
                    if (joy == joysticks.end()) break;

                    auto button = joy->second.buttons.find(event.jbutton.button);
                    if (button == joy->second.buttons.end()) break;

                    spdlog::trace("Found button mapping: joy {}, button {}", joy->first, button->first);

                    break;
                }
                case SDL_QUIT:
                {
                    spdlog::info("Quitting...");
                    end = true;
                    break;
                }
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        spdlog::critical("Exception: {}", e.what());
    }
    catch (...)
    {
        spdlog::critical("Unknown exception");
    }

    SDL_Quit();
    spdlog::shutdown();
    return 0;
}
