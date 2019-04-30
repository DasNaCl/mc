#include <symbol.hpp>
#include <util.hpp>

thread_local tsl::hopscotch_map<std::uint_fast32_t, std::string> Symbol::symbols = {};

Symbol::Symbol(const std::string& str)
  : hash(hash_string(str))
{ lookup_or_emplace(hash, str.c_str()); }

Symbol::Symbol(const char* str)
  : hash(hash_string(str))
{ lookup_or_emplace(hash, str); }

Symbol::Symbol(const Symbol& s)
  : hash(s.hash)
{  }

Symbol::Symbol(Symbol&& s)
  : hash(s.hash)
{  }

Symbol::~Symbol() noexcept
{  }

Symbol& Symbol::operator=(const std::string& str)
{
  this->hash = hash_string(str);
  lookup_or_emplace(this->hash, str.c_str());

  return *this;
}

Symbol& Symbol::operator=(const char* str)
{
  this->hash = hash_string(str);
  lookup_or_emplace(this->hash, str);

  return *this;
}

Symbol& Symbol::operator=(const Symbol& s)
{
  this->hash = s.hash;

  return *this;
}

Symbol& Symbol::operator=(Symbol&& s)
{
  this->hash = s.hash;

  return *this;
}

std::ostream& operator<<(std::ostream& os, const Symbol& symb)
{
  os << Symbol::symbols[symb.hash];
  return os;
}

std::string& Symbol::lookup_or_emplace(std::uint_fast32_t hash, const char* str)
{
  auto it = symbols.find(hash);
  if(it != symbols.end())
    return symbols[hash];

  symbols[hash] = str;
  return symbols[hash];
}

std::ostream& operator<<(std::ostream& os, const std::vector<Symbol>& symbs)
{
  for(auto& s : symbs)
    os << s;
  return os;
}

std::uint_fast32_t Symbol::get_hash() const
{ return hash; }

const std::string& Symbol::get_string() const
{ return symbols[hash]; }

