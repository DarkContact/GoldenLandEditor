#pragma once
#include <cstdint>
#include <format>
#include <ranges>

#include "utils/StringUtils.h"
#include "Types.h"

template <typename R>
concept FormattableRange =
    std::ranges::range<R> &&
    !std::is_convertible_v<R, std::wstring_view> &&
    !std::is_convertible_v<R, std::string_view>;

template <FormattableRange R>
struct std::formatter<R> {
public:
    constexpr auto parse(auto& ctx) {
        return element_formatter.parse(ctx);
    }

    auto format(const R& range, auto& ctx) const {
        auto out = ctx.out();
        *out++ = '[';

        auto it = std::ranges::begin(range);
        auto end = std::ranges::end(range);

        if (it != end) {
            out = element_formatter.format(*it, ctx);
            ++it;

            while (it != end) {
                *out++ = ',';
                *out++ = ' ';

                out = element_formatter.format(*it, ctx);
                ++it;
            }
        }

        *out++ = ']';
        return out;
    }

private:
    using T = std::ranges::range_value_t<R>;
    std::formatter<T> element_formatter;
};

template <>
struct std::formatter<AgeVariable_t> : std::formatter<std::string_view> {
    auto format(const AgeVariable_t& v, std::format_context& ctx) const {
        char buffer[1024];

        std::visit([&buffer](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int32_t>)
                StringUtils::formatToBuffer(buffer, "int32: {}", arg);
            else if constexpr (std::is_same_v<T, uint32_t>)
                StringUtils::formatToBuffer(buffer, "uint32: {}", arg);
            else if constexpr (std::is_same_v<T, double>)
                StringUtils::formatToBuffer(buffer, "double: {}", arg);
            else if constexpr (std::is_same_v<T, std::string>)
                StringUtils::formatToBuffer(buffer, "string: \"{}\"", arg);
        }, v);

        return std::formatter<std::string_view>::format(std::string_view{buffer}, ctx);
    }
};
