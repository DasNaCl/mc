#pragma once

#include <cstdint>
#include <ostream>

struct SourceRange
{
  const char* module;

  std::size_t column_beg;
  std::size_t row_beg;

  std::size_t column_end;
  std::size_t row_end;

  SourceRange() = default;

  SourceRange(const char* module, std::size_t column_beg, std::size_t row_beg,
                                  std::size_t column_end, std::size_t row_end);

  friend std::ostream& operator<<(std::ostream& os, const SourceRange& src_range);
};

