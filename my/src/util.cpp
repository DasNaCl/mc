#include <util.hpp>

#include <algorithm>
#include <iostream>
#include <cassert>

#ifndef NDEBUG
#include <csignal>
#include <cassert>

bool StaticOptions::enabled_breakpoints = false;

void StaticOptions::enable_breakpoints()
{ enabled_breakpoints = true; }

void breakpoint()
{
  if(StaticOptions::enabled_breakpoints)
    std::raise(SIGINT);
}
#else
void breakpoint()
{  }
#endif

std::uint_fast32_t hash_combine(std::uint_fast32_t lhs, std::uint_fast32_t rhs)
{ return lhs ^ rhs + 0x517cc1b7 + (lhs << 6U) + (lhs >> 2U); }

