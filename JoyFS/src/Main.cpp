
#include "A2ASTec30AP.h"
#include "Model.h"
#include "ReadSettings.h"
#include "Sim.h"
#include "SimModelLoop.h"

#include <BFFCLAPI/UDPClient.h>

#include <Utils/Common.h>
#include <Utils/QueueRunner.h>
#include <Utils/Timer.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <boost/algorithm/string.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/property_tree/ptree.hpp>

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

void initLogging(const LogSettings& settings)
{
    // init logging

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("altbff.log", true);
    file_sink->set_level(settings.logLevel);

    auto logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list{console_sink, file_sink});

    logger->set_level(spdlog::level::trace);

    spdlog::set_default_logger(logger);

    spdlog::info("AltBFF logging started");
}

void updateCLDefaultsFromModel(bffcl::UDPClient& cl, Model& model)
{
    auto& input = cl.lockInput();
    input.aileron.frictionCoeff = model.getFrictionCoeff(Model::Aileron);
    input.aileron.dumpingCoeff = model.getDumpingCoeff(Model::Aileron);

    input.aileron.positionFollowingP = model.getPositionFollowingP(Model::Aileron);
    input.aileron.positionFollowingI = model.getPositionFollowingI(Model::Aileron);
    input.aileron.positionFollowingD = model.getPositionFollowingD(Model::Aileron);

    input.elevator.frictionCoeff = model.getFrictionCoeff(Model::Elevator);
    input.elevator.dumpingCoeff = model.getDumpingCoeff(Model::Elevator);

    input.elevator.positionFollowingP = model.getPositionFollowingP(Model::Elevator);
    input.elevator.positionFollowingI = model.getPositionFollowingI(Model::Elevator);
    input.elevator.positionFollowingD = model.getPositionFollowingD(Model::Elevator);

    cl.unlockInput();
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

bool settingsUpdateLoop(bffcl::UDPClient& cl, Model& model, A2AStec30AP& autopilot, const path& settingsPath,
                        file_time_type& settingsWriteTime)
{
    file_time_type newSettingsWriteTime = last_write_time(settingsPath);
    if (newSettingsWriteTime > settingsWriteTime)
    {
        spdlog::info("Settings updated, reloading model settings...");
        ptree settings = readSettings(settingsPath);
        Model::Settings modelSettings = readModelSettings(settings);
        model.setSettings(modelSettings);

        updateCLDefaultsFromModel(cl, model);

        spdlog::info("Model settings updated");

        A2AStec30AP::Settings apSettings = readAPSettings(settings);
        autopilot.setSettings(apSettings);
        spdlog::info("AP settings updated");

        settingsWriteTime = newSettingsWriteTime;
    }
    return false;  // call again
}

int checkedMain(int argc, char** argv)
{
    path exeName = argv[0];
    path settingsPath = exeName.parent_path() / "settings.ini";

    ptree settings = readSettings(settingsPath);

    bffcl::UDPClient::Settings clSettings = readCLSettings(settings);
    Sim::Settings simSettings = readSimSettings(settings);
    Model::Settings modelSettings = readModelSettings(settings);
    LogSettings logSettings = readLogSettings(settings);
    A2AStec30AP::Settings apSettings = readAPSettings(settings);

    initLogging(logSettings);

    // create main components
    spdlog::info("Creating main components");

    bffcl::UDPClient cl(clSettings);
    Sim sim(simSettings);
    Model model(modelSettings);
    A2AStec30AP autopilot(apSettings);

    spdlog::info("Main components created successfully");

    updateCLDefaultsFromModel(cl, model);

    QueueRunner runner;  // setup runner for this thread

    auto kModelLoopFreq = std::chrono::milliseconds(1000 / 30);  // run at 30Hz
    Timer simModelLoop(kModelLoopFreq, runner,
                       [&cl, &sim, &model, &autopilot] { return simModelLoop(cl, sim, model, autopilot); });

    // settings refresh utility
    file_time_type settingsWriteTime = last_write_time(settingsPath);
    auto kSettingsLoopFreq = std::chrono::milliseconds(2000);  // run at 0.5Hz
    Timer settingsUpdateLoop(kSettingsLoopFreq, runner, [&cl, &model, &autopilot, settingsPath, &settingsWriteTime]
                             { return settingsUpdateLoop(cl, model, autopilot, settingsPath, settingsWriteTime); });

    spdlog::info("Starting sim/model loop");
    simModelLoop.start();
    spdlog::info("Sim/model loop started");

    spdlog::info("Starting settings refresh loop");
    settingsUpdateLoop.start();
    spdlog::info("Settings refresh loop started");

    spdlog::info("Starting main loop");
    runner.run();
    spdlog::info("Main loop finished");

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
        throw;
    }
    catch (...)
    {
        spdlog::critical("Unknown exception");
        spdlog::shutdown();
        throw;
    }
}
