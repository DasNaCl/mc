#include <SourceRange.hpp>

SourceRange::SourceRange(const char* module, std::size_t column_beg, std::size_t row_beg,
                                             std::size_t column_end, std::size_t row_end)
  : module(module), column_beg(column_beg), row_beg(row_beg), column_end(column_end), row_end(row_end)
{  }

std::ostream& operator<<(std::ostream& os, const SourceRange& src_range)
{
  os << src_range.module << ":" << src_range.column_beg
                         << ":" << src_range.row_beg
                         << ":";
  return os;
}

