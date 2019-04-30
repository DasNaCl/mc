#include <SourceRange.hpp>

SourceRange::SourceRange(const char* module, std::size_t column_beg, std::size_t row_beg,
                                             std::size_t column_end, std::size_t row_end)
  : module(module), column_beg(column_beg), row_beg(row_beg), column_end(column_end), row_end(row_end)
{  }

void SourceRange::widen(const SourceRange& other)
{
  column_beg = std::min(column_beg, other.column_beg);
  row_beg = std::min(row_beg, other.row_beg);

  column_end = std::max(column_end, other.column_end);
  row_end = std::max(row_end, other.row_end);
}

std::ostream& operator<<(std::ostream& os, const SourceRange& src_range)
{
  os << src_range.module << ":" << src_range.column_beg
                         << ":" << src_range.row_beg
                         << ":";
  return os;
}

SourceRange operator+(const SourceRange& left, const SourceRange& right)
{
  SourceRange range = left;

  range.widen(right);

  return range;
}

