#include "ReadSettings.h"

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <map>

using boost::property_tree::ptree;
using std::filesystem::file_time_type;
using std::filesystem::path;

ptree readSettings(const path& file)
{
    ptree settings;
    boost::property_tree::read_ini(file.string(), settings);
    return settings;
}

LogSettings readLogSettings(const ptree& settings)
{
    LogSettings logSettings;

    auto logLevelStr = boost::algorithm::to_lower_copy(settings.get<std::string>("App.LogLevel"));

    std::map<std::string, spdlog::level::level_enum> settingsStrToEnum = {
        {"trace", spdlog::level::trace}, {"debug", spdlog::level::debug}, {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},   {"err", spdlog::level::err},     {"critical", spdlog::level::critical},
        {"off", spdlog::level::off}};

    if (auto levelFound = settingsStrToEnum.find(logLevelStr); levelFound != settingsStrToEnum.end())
        logSettings.logLevel = levelFound->second;

    return logSettings;
}
