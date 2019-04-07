#include <util.hpp>

std::uint_fast32_t hash_combine(std::uint_fast32_t lhs, std::uint_fast32_t rhs)
{ return lhs ^ rhs + 0x517cc1b7 + (lhs << 6U) + (lhs >> 2U); }

