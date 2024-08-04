
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>
#include "ReadSettings.h"

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <windows.h>

#include <stdio.h>
#include <filesystem>
#include <iostream>
#include <string>

using namespace std;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    // todo: add closing
    spdlog::shutdown();

    return FALSE;  // let others work on this
}

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

    auto logLevelStr = boost::algorithm::to_lower_copy(settings.get<std::string>("LogLevel"));
    auto logLevel = toLogLevel(logLevelStr);

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("altfs.log", true);
    file_sink->set_level(logLevel);

    auto logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list{console_sink, file_sink});

    logger->set_level(spdlog::level::trace);

    spdlog::set_default_logger(logger);

    spdlog::info("AltBFF logging started");
}

int runTests(int argc, char** argv)
{
    // remove "test" from command line
    int argc2 = argc - 1;
    std::vector<char*> argv2;
    for (int i = 0; i < argc; ++i)
        if (i != 1) argv2.push_back(argv[i]);

    int result = Catch::Session().run(argc2, &argv2[0]);
    return result;
}

int checkedMain(int argc, char** argv)
{
    path exeName = argv[0];
    path settingsPath = exeName.parent_path() / "Settings.json";

    ptree settings = readSettings(settingsPath);
    ptree appSettings = settings.get_child("App");
    ptree joySettings = settings.get_child("Joysticks");

    initLogging(appSettings);

    if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
    {
        spdlog::error("Couldn't initialize SDL: {}", SDL_GetError());
        return -1;
    }

    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        spdlog::info(" {} = {} ", i, SDL_JoystickNameForIndex(i));
    }


  
    SDL_JoystickEventState(SDL_ENABLE);

    std::vector<SDL_Joystick*> joysticks;
    for(const auto& joy: joySettings)
    {
        int id = boost::lexical_cast<int>(joy.first);
        spdlog::info("Capturing device {}", id);
        SDL_Joystick* joystick = SDL_JoystickOpen(id);
        joysticks.push_back(joystick);
    }

  
    spdlog::info("Starting event processing cycle");

    bool end = false;
    int pollingDelayMs = appSettings.get<int>("PollingDelayMs");
    spdlog::info("Polling delay {}", pollingDelayMs);
    SDL_Event event;

    for (; !end;)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(pollingDelayMs));

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
                // data->mutable_button()->set_index(event.jbutton.button);
                // data->mutable_button()->set_value(event.jbutton.state == SDL_PRESSED ? true : false);
                break;
            }
            case SDL_QUIT:
            {
                end = true;
                break;
            }
            }
        }
    }

    return 0;
}

int main(int argc, char** argv)
{
    // check for tests session
    if (argc > 1 && strcmp(argv[1], "test") == 0) return runTests(argc, argv);

    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    try
    {
        return checkedMain(argc, argv);
    }
    catch (const std::exception& e)
    {
        spdlog::critical("Exception: {}", e.what());
        spdlog::shutdown();
    }
    catch (...)
    {
        spdlog::critical("Unknown exception");
        spdlog::shutdown();
    }
}
