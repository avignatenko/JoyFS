#pragma once

#include <spdlog/spdlog.h>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>

struct LogSettings
{
    spdlog::level::level_enum logLevel = spdlog::level::info;
};

boost::property_tree::ptree readSettings(const std::filesystem::path& file);

LogSettings readLogSettings(const boost::property_tree::ptree& settings);
