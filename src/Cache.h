#pragma once
#include <string_view>
#include <optional>
#include <string>

#include "utils/TracyProfiler.h"
#include "Types.h"

template<typename Callback, typename T>
concept LoadCallback = requires(Callback cb) {
    { cb() } -> std::same_as<std::optional<T>>;
};

template<class T, size_t MaxCached = 0>
class Cache {
public:
    Cache() noexcept {}

    template <LoadCallback<T> Callback>
    const T* load(std::string_view key, Callback&& callback) {
        Tracy_ZoneScoped;
        Tracy_ZoneText(key.data(), key.size());
        if (auto it = m_cache.find(key); it != m_cache.end())
            return &it->second;

        std::optional<T> resource = std::forward<Callback>(callback)();
        if (!resource)
            return nullptr;

        // Если есть лимит и он превышен, выгружаем ресурс
        if constexpr (MaxCached > 0 && !m_cache.empty() && m_cache.size() >= MaxCached) {
            m_cache.erase(m_cache.begin());
        }

        auto [it, _] = m_cache.emplace(std::string(key), std::move(*resource));
        return &it->second;
    }

    void clear() noexcept {
        m_cache.clear();
    }

private:
    StringHashTable<T> m_cache;
};
