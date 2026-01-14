#pragma once
#include <string_view>
#include <optional>
#include <string>

#include "Types.h"

template<typename Callback, typename T>
concept LoadCallback = requires(Callback cb, std::string_view key, std::string* error) {
    { cb(key, error) } -> std::same_as<std::optional<T>>;
};

template<class T, size_t MaxCached = 0, size_t ReserveSize = 0>
class Cache {
public:
    Cache() noexcept {
        if constexpr (ReserveSize > 0) {
            m_cache.reserve(ReserveSize);
        }
    }

    template <LoadCallback<T> Callback>
    const T* load(std::string_view key, std::string* error, Callback&& callback) {
        if (auto it = m_cache.find(key); it != m_cache.end())
            return &it->second;

        std::optional<T> resource = std::forward<Callback>(callback)(key, error);
        if (!resource)
            return nullptr;

        // Если есть лимит и он превышен, выгружаем ресурс
        if constexpr (MaxCached > 0 && !m_cache.empty() && m_cache.size() >= MaxCached) {
            m_cache.erase(m_cache.begin());
        }

        auto [it, _] = m_cache.emplace(std::string(key), std::move(*resource));
        return &it->second;
    }

private:
    StringHashTable<T> m_cache;
};
