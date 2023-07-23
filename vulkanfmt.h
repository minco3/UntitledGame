#pragma once
#include <fmt/core.h>
#include <fmt/format.h>
#include <vulkan/vk_enum_string_helper.h>

template <> struct fmt::formatter<VkFormat>: formatter<string_view>
{
    auto format(VkFormat format, format_context& ctx) const
    {
        string_view formatString = string_VkFormat(format);
        return formatter<string_view>::format(formatString, ctx);
    }
};
template <> struct fmt::formatter<VkColorSpaceKHR>: formatter<string_view>
{
        auto format(VkColorSpaceKHR colorSpace, format_context& ctx) const
    {
        string_view colorSpaceString = string_VkColorSpaceKHR(colorSpace);
        return formatter<string_view>::format(colorSpaceString, ctx);
    }
};
template <> struct fmt::formatter<VkResult>: formatter<string_view>
{
        auto format(VkResult result, format_context& ctx) const
    {
        string_view resultString = string_VkResult(result);
        return formatter<string_view>::format(resultString, ctx);
    }
};
template <> struct fmt::formatter<VkExtent2D>
{
    constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it != '}') throw format_error("invalid format");
        return it;
    }
    auto format(VkExtent2D extent, format_context& ctx) const -> format_context::iterator
    {
        return format_to(ctx.out(), "{}x{}", extent.width, extent.height);
    }
};