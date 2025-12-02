#include <cstring>
#include <cmath>
#include <cstdint>
#include <cassert>

#include <vector>

#define PART_TWO

void assert_failed();
#define ASSERT(expr) if consteval { if (!(expr)) { assert_failed(); } } else { assert(expr); }
#define MSG(format, ...) if !consteval { std::println(stderr, format, __VA_ARGS__); }

typedef uint64_t elfid_t;
typedef uint16_t digit_t;

constexpr auto MAX_DIGITS = 96;

constexpr digit_t digits_in_id(elfid_t id)
{
    digit_t l = 1;
    while (id /= 10)
    {
        ++l;
    }

    return l;
}

constexpr elfid_t str_to_id(std::string_view str)
{
    elfid_t value = 0;
    if consteval
    {
        auto result = std::from_chars(str.begin(), str.end(), value);
        if (result.ptr != str.end() || result.ec != std::errc{})
        {
            ASSERT(false);
        }
    }
    else
    {
        value = std::strtoull(str.data(), nullptr, 10);
    }

    return value;
}

constexpr char nth_digit_as_char(elfid_t id, digit_t n)
{
    elfid_t final = id;
    while (n > 0)
    {
        final /= 10;
        --n;
    }

    const char *digits = "0123456789";
    return digits[final % 10];
}

constexpr uint8_t nth_digit(elfid_t id, digit_t n)
{
    char ch = nth_digit_as_char(id, n);
    ASSERT(ch >= '0' && ch <= '9');
    return ch - '0';
}

constexpr bool split_and_check(elfid_t id, digit_t digits, digit_t splitAt)
{
    if ((digits % splitAt) != 0)
    {
        return true;
    }

    if (digits == splitAt)
    {
        return true;
    }

    if (splitAt != 1)
    {
        // 123123123
        //       ^^^ i
        for (digit_t i = 1; i < digits / splitAt; ++i)
        {
            // 123123123
            //   ^  ^  ^ j
            for (digit_t j = 0; j < splitAt; ++j)
            {
                const digit_t last = nth_digit(id, j);
                const digit_t other = nth_digit(id, j + (splitAt * i));
                if (last != other)
                {
                    return true;
                }
            }
        }
    }
    else
    {
        digit_t i = 0;
        while (i + 1 < digits)
        {
            const digit_t first = nth_digit(id, i);
            const digit_t other = nth_digit(id, i + 1);
            if (first != other)
            {
                return true;
            }

            ++i;
        }
    }

    
    return false;
}

constexpr bool id_is_valid(elfid_t id)
{
    const digit_t digits = digits_in_id(id);
#ifdef PART_TWO
    if (digits < 2)
    {
        return true;
    }

    bool prime = true;
    for (digit_t n = 2; n <= std::sqrt(digits); ++n)
    {
        if ((digits % n) == 0)
        {
            prime = false;
            break;
        }
    }
    prime = prime || digits == 2;

    if (prime)
    {
        // we will only have repeats of 1 digit
        return split_and_check(id, digits, 1);
    }
    else
    {
        // we need to repeatedly get the lowest multiple of digits
        for (digit_t n = digits - 1; n > 1; --n)
        {
            if ((digits % n) == 0)
            {
                if (!split_and_check(id, digits, n))
                {
                    return false;
                }
            }
        }

        return true;
    }
#else
    if ((digits % 2) == 0)
    {
        return split_and_check(id, digits, digits / 2);
    }

    return true;
#endif
}

constexpr elfid_t invalid_ids_in_range(elfid_t min, elfid_t max)
{
    ASSERT(min < max);

    elfid_t total = 0;

    for (elfid_t n = min; n <= max; ++n) 
    {
        if (id_is_valid(n))
        {
            continue;
        }

        total += n;
    }

    return total;
}

constexpr elfid_t invalid_ids_in_string(std::string_view str)
{
    elfid_t total = 0;

    char startBuf[MAX_DIGITS]{};
    size_t startI = 0;

    char endBuf[MAX_DIGITS]{};
    size_t endI = 0;
    
    bool start = true;

    for (size_t i = 0; i < str.length(); ++i)
    {
        const char ch = str[i];

        if (start)
        {
            if (ch == '-')
            {
                start = false;
            }
            else
            {
                startBuf[startI] = ch;
                ++startI;
            }
        }
        else
        {
            if (ch == ',' || i == str.length() - 1)
            {
                if (i == str.length() - 1)
                {
                    endBuf[endI] = ch;
                    ++endI;
                }

                elfid_t min = str_to_id(startBuf);
                elfid_t max = str_to_id(endBuf);
                total += invalid_ids_in_range(min, max);

                // reset
                if consteval
                {
                    for (size_t i = 0; i < MAX_DIGITS; ++i)
                    {
                        startBuf[i] = 0;
                        endBuf[i] = 0;
                    }
                }
                else
                {
                    std::memset(startBuf, 0, MAX_DIGITS);
                    std::memset(endBuf, 0, MAX_DIGITS);
                }
                startI = 0;
                endI = 0;
                start = true;
            }
            else
            {
                endBuf[endI] = ch;
                ++endI;
            }
        }
    }

    return total;
}

constexpr std::string_view SAMPLE = "245284-286195,797927-983972,4949410945-4949555758,115-282,8266093206-8266228431,1-21,483873-655838,419252-466133,6190-13590,3876510-4037577,9946738680-9946889090,99954692-100029290,2398820-2469257,142130432-142157371,9797879567-9798085531,209853-240025,85618-110471,35694994-35766376,4395291-4476150,33658388-33694159,680915-772910,4973452995-4973630970,52-104,984439-1009605,19489345-19604283,22-42,154149-204168,7651663-7807184,287903-402052,2244-5558,587557762-587611332,307-1038,16266-85176,422394377-422468141";