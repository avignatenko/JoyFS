#include "Logging.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <boost/algorithm/string.hpp>
#include <map>

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
