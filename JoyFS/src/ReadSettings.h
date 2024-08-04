#pragma once

#include <spdlog/spdlog.h>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>
#include <map>


boost::property_tree::ptree readSettings(const std::filesystem::path& file);
