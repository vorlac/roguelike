#include "gl/skity/render/hw/hw_render_target.hpp"

namespace skity {

    enum {
        CACHE_PERGE_LIMIT = 1000,
    };

    HWRenderTarget* HWRenderTargetCache::QueryTarget(uint32_t width, uint32_t height)
    {
        HWRenderTargetCache::Size target_size{ width, height };

        const auto& it = info_map_.find(target_size);

        if (it != info_map_.end())
        {
            for (auto& info : it->second)
            {
                if (!info.used)
                {
                    info.used = true;
                    info.age = current_age_;
                    return info.target;
                }
            }
        }

        return nullptr;
    }

    HWRenderTarget* HWRenderTargetCache::StoreCache(std::unique_ptr<HWRenderTarget> target)
    {
        HWRenderTargetCache::Size target_size{ target->Width(), target->Height() };
        Info target_info{ current_age_, true, target.get() };

        auto it = info_map_.find(target_size);
        if (it != info_map_.end())
        {
            it->second.emplace_back(target_info);
        }
        else
        {
            std::vector<Info> info_list{ target_info };
            info_map_.insert(std::make_pair(target_size, info_list));
        }

        target_cache_.insert(std::make_pair(target_info.target, std::move(target)));

        return target_info.target;
    }

    void HWRenderTargetCache::BeginFrame()
    {
        current_age_++;
        ClearUsedFlags();
    }

    void HWRenderTargetCache::EndFrame()
    {
        for (auto map_it = info_map_.begin(); map_it != info_map_.end();)
        {
            for (auto it = map_it->second.begin(); it != map_it->second.end();)
            {
                if (current_age_ - it->age > CACHE_PERGE_LIMIT)
                {
                    auto rt = it->target;
                    it = map_it->second.erase(it);
                    target_cache_.erase(rt);
                }
                else
                {
                    it++;
                }
            }

            if (map_it->second.empty())
                map_it = info_map_.erase(map_it);
            else
                map_it++;
        }
    }

    void HWRenderTargetCache::CleanUp()
    {
        for (auto& it : target_cache_)
            it.second->Destroy();
    }

    void HWRenderTargetCache::ClearUsedFlags()
    {
        for (auto& it : info_map_)
            for (auto& list : it.second)
                list.used = false;
    }

}  // namespace skity
