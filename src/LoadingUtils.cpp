
#include <fstream>
#include <filesystem>

#include "raylib.h"

#include "LoadingUtils.h"

std::vector<std::string> GetJsonFileListFromPath(std::string path)
{
	std::vector<std::string> fileList;
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		if (file.path().string().find(".json") != std::string::npos)
		{
			fileList.push_back(file.path().string());
		}
	}
	return fileList;
}

json LoadAndParseJson(std::string file)
{
    TraceLog(LOG_INFO, "LoadingUtils loading: %s", file.c_str());
    std::ifstream jsonStream(file);
    std::string s(std::istreambuf_iterator<char>(jsonStream), {});

    json data;
    try
    {
        data = json::parse(s);
    }
    catch (json::parse_error& ex)
    {
        TraceLog(LOG_ERROR, "Parse error at byte: %d", ex.byte);
    }
    return data;
}