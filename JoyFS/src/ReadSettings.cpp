#include "ReadSettings.h"

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::property_tree::ptree;
using std::filesystem::file_time_type;
using std::filesystem::path;

ptree readSettings(const path& file)
{
    ptree settings;
    boost::property_tree::read_json(file.string(), settings);
    return settings;
}

