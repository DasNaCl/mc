#pragma once

#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>

#include <cstdint>
#include <iosfwd>

struct Symbol
{
  Symbol(const std::string& str);
  Symbol(const char* str);
  Symbol(const Symbol& s);
  Symbol(Symbol&& s);
  ~Symbol() noexcept;

  Symbol& operator=(const std::string& str);
  Symbol& operator=(const char* str);
  Symbol& operator=(const Symbol& s);
  Symbol& operator=(Symbol&& s);

  friend std::ostream& operator<<(std::ostream& os, const Symbol& s);

  const std::string& get_string() const;
  std::uint_fast32_t get_hash() const;
private:
  static std::string& lookup_or_emplace(std::uint_fast32_t hash, const char* str);
private:
  thread_local static tsl::hopscotch_map<std::uint_fast32_t, std::string> symbols;

  std::uint_fast32_t hash;
};
struct SymbolHasher
{
  std::size_t operator()(Symbol symb) const
  { return symb.get_hash(); }
};
struct SymbolComparer
{
  bool operator()(Symbol lhs, Symbol rhs) const
  { return lhs.get_hash() == rhs.get_hash(); }
};

template<class T, 
         unsigned int NeighborhoodSize = 62,
         bool StoreHash = false,
         class GrowthPolicy = tsl::hh::power_of_two_growth_policy<2>>
using SymbolMap = tsl::hopscotch_map<Symbol, T, SymbolHasher, SymbolComparer, std::allocator<std::pair<Symbol, T>>,
                                      NeighborhoodSize, StoreHash, GrowthPolicy>;

using SymbolSet = tsl::hopscotch_set<Symbol, SymbolHasher, SymbolComparer, std::allocator<Symbol>,
                                     62, false, tsl::hh::power_of_two_growth_policy<2>>;
template<unsigned int NeighborhoodSize = 62,
         bool StoreHash = false,
         class GrowthPolicy = tsl::hh::power_of_two_growth_policy<2>>
using SymbolSetT = tsl::hopscotch_set<Symbol, SymbolHasher, SymbolComparer, std::allocator<Symbol>,
                                         NeighborhoodSize, StoreHash, GrowthPolicy>;

bool operator==(const Symbol& a, const Symbol& b);
bool operator!=(const Symbol& a, const Symbol& b);
std::ostream& operator<<(std::ostream& os, const std::vector<Symbol>& symbs);


