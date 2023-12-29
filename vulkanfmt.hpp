#pragma once

#include <fmt/core.h>
#include <fmt/format.h>
#include <vulkan/vulkan.hpp>

template <> struct fmt::formatter<vk::Format>: formatter<string_view>
{
    auto format(vk::Format format, format_context& ctx) const
    {
        string_view formatString = vk::to_string(format);
        return formatter<string_view>::format(formatString, ctx);
    }
};
template <> struct fmt::formatter<vk::ColorSpaceKHR>: formatter<string_view>
{
        auto format(vk::ColorSpaceKHR colorSpace, format_context& ctx) const
    {
        string_view colorSpaceString = vk::to_string(colorSpace);
        return formatter<string_view>::format(colorSpaceString, ctx);
    }
};
template <> struct fmt::formatter<vk::Extent2D>
{
    constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it != '}') throw format_error("invalid format");
        return it;
    }
    auto format(vk::Extent2D extent, format_context& ctx) const -> format_context::iterator
    {
        return format_to(ctx.out(), "{}x{}", extent.width, extent.height);
    }
};