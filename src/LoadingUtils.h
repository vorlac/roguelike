#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

std::vector<std::string> GetJsonFileListFromPath(std::string path);
json LoadAndParseJson(std::string file);
