#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <span>
#include <print>
#include <system_error>
#include <charconv>

#include "sample.hh"

void non_constexpr_function();
#define ASSERT(expr) if consteval { if (!(expr)) { non_constexpr_function(); } } else { assert(expr); }
#define MSG(format, ...) if !consteval { std::println(stderr, format, __VA_ARGS__); }

typedef unsigned long long ulonglong_t;
typedef signed long long ilonglong_t;
typedef int8_t safepos_t;

enum class SafeDirection : safepos_t { LEFT = -1, RIGHT = 1 };

struct SafeMovement { ulonglong_t amt; SafeDirection dir; };

struct SafePosition
{
    safepos_t value{0};

    explicit constexpr SafePosition(ilonglong_t v)
    {
        value = clamp_value(v).newPos;
    }

    constexpr size_t rotate(SafeMovement move)
    {
        ilonglong_t v = value + (move.amt * static_cast<safepos_t>(move.dir));
        auto result = clamp_value(v);
        value = result.newPos;
        return result.wrapCount;
    }
    
    struct ClampResult { size_t wrapCount; safepos_t newPos; };
    constexpr static ClampResult clamp_value(ilonglong_t v)
    {
        size_t wraps = 0;

        while (v > 99)
        {
            v -= 100;
            ++wraps;
        }

        while (v < 0)
        {
            v += 100;
            ++wraps;
        }
        
        ASSERT(v >= 0 && v <= 99);
        return ClampResult{.wrapCount = wraps, .newPos = static_cast<safepos_t>(v)};
    }
};

constexpr SafeMovement moves_from_str(std::string_view str)
{
    if (str.length() < 2)
    {
        MSG("string '{}' is ill-formed", str);
        ASSERT(false);
    }

    SafeDirection dir{};
    switch(str[0])
    {
    case 'L':
        dir = SafeDirection::LEFT;
        break;
    case 'R':
        dir = SafeDirection::RIGHT;
        break;
    default:
        MSG("string '{}' is ill-formed", str);
        ASSERT(false);
    }


    ulonglong_t value = 0;
    if consteval
    {
        auto result = std::from_chars(str.begin() + 1, str.end(), value);
        if (result.ptr != str.end() || result.ec != std::errc{})
        {
            ASSERT(false);
        }
    }
    else
    {
        value = std::strtoull(str.data() + 1, nullptr, 10);
    }

    return SafeMovement{.amt = value, .dir = dir};
}

template<size_t Extent>
consteval size_t zeroes_comptime(SafePosition initial, const std::span<const std::string_view, Extent> strs)
{
    size_t zeroes = 0;

    for (std::string_view str : strs)
    {
        SafeMovement move = moves_from_str(str);
        zeroes += initial.rotate(move);
    }

    return zeroes;
}

int main()
{
    constexpr SafePosition initial{50};
    size_t zeroes = zeroes_comptime(initial, std::span{SAMPLE});
    std::println("{} zeroes", zeroes);
    return 0;
}
