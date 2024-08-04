#pragma once

#include <spdlog/spdlog.h>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>

#include "A2ASTec30AP.h"
#include "Sim.h"
#include "Model.h"

#include <BFFCLAPI/UDPClient.h>

struct LogSettings
{
    spdlog::level::level_enum logLevel = spdlog::level::info;
};

boost::property_tree::ptree readSettings(const std::filesystem::path& file);

LogSettings readLogSettings(const boost::property_tree::ptree& settings);
bffcl::UDPClient::Settings readCLSettings(const boost::property_tree::ptree& settings);
Sim::Settings readSimSettings(const boost::property_tree::ptree& settings);
A2AStec30AP::Settings readAPSettings(const boost::property_tree::ptree& settings);
Model::Settings readModelSettings(const boost::property_tree::ptree& settings);