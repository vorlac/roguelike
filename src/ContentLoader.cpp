#include "raylib.h"

#include "ContentLoader.h"
#include "LoadingUtils.h"

ContentLoader::ContentLoader() {
  TraceLog(LOG_INFO, "Loading Content....");
  for (const auto &file : GetJsonFileListFromPath("data/game/")) {
    TraceLog(LOG_INFO, "Loading: %s", file.c_str());
  }
}

static inline int asdf{};
