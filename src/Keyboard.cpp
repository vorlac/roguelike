#include "Keyboard.h"

#include "LoadingUtils.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string KeyboardConfiguration::EvaluateKey(KeyboardKey key, std::string context, bool shift,
                                               bool ctrl, bool alt)
{
    std::string action("none");
    std::stringstream formattedKey;
    if (shift)
        formattedKey << "Shift+";
    if (ctrl)
        formattedKey << "Control+";
    if (alt)
        formattedKey << "Alt+";
    formattedKey << RaylibKeys::Keys[key];

    std::string stringKey = formattedKey.str();

    if (contexts.contains(context))
    {
        if (contexts[context].contains(stringKey))
            action = contexts[context][stringKey];
    }
    else
    {
        TraceLog(LOG_ERROR, "No keyboard context '%s' exists.", context.c_str());
    }
    return action;
}

KeyboardConfiguration LoadKeyboardConfigfuration()
{
    TraceLog(LOG_INFO, "Loading keyboard configuration");
    KeyboardConfiguration config;

    json data = LoadAndParseJson("data/keyboard.json");
    // if (data.contains("contexts"))
    // parse each context
    for (auto& [contextName, contextValues] : data["contexts"].items())
    {
        // Check to see if the context exists if not create it
        if (!config.contexts.contains(contextName))
        {
            TraceLog(LOG_INFO, "Creating context: %s", contextName.c_str());
            config.contexts[contextName] = Keymap();
            // Iterate and add keys
            for (auto& [command, keys] : data["contexts"][contextName].items())
            {
                for (auto& key : keys)
                {
                    std::string stringKey = (std::string)key;
                    TraceLog(LOG_INFO, "- Adding keybind for %s - %s", command.c_str(),
                             ((std::string)key).c_str());
                    config.contexts[contextName][stringKey] = command.c_str();
                }
            }
        }
        // Error on context already existing and skip it
        else
        {
            // TODO
        }
    }
    return config;
}
