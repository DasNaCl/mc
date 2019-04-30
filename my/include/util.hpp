#pragma once

#include <cstdint>

struct StaticOptions
{
  friend void breakpoint();
#ifndef NDEBUG
  static void enable_breakpoints();
private:
  static bool enabled_breakpoints;
#endif
};

extern void breakpoint();

extern std::uint_fast32_t hash_combine(std::uint_fast32_t lhs, std::uint_fast32_t rhs);

template<typename T, typename H = std::uint_fast32_t>
constexpr H hash_string(T str)
{
  if(!(&str[0])) return 0;
  H hash = str[0];
  for(auto* p = &str[0]; p && *p != '\0'; p++)
    hash ^= (hash * 31) + (*p);
  return hash;
}

