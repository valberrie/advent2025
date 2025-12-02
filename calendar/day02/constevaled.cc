#include <print>

#include "code.hh"

int main()
{
    constexpr elfid_t total = invalid_ids_in_string(SAMPLE);
    std::println(stdout, "{}", total);

    return 0;
}