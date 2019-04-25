#pragma once

#include <tsl/hopscotch_map.h>
#include <typeindex>
#include <cstdint>
#include <sstream>
#include <vector>
#include <string>
#include <memory>
#include <map>

std::uint_fast32_t hash_combine(std::uint_fast32_t lhs, std::uint_fast32_t rhs);

struct CmdOptions
{
  template<typename T>
  struct TaggedValue;

  class Value : public std::enable_shared_from_this<Value>
  {
  public:
    using Ptr = std::shared_ptr<Value>;

    virtual Ptr clone() = 0;
    virtual void parse(int& i, int argc, const char** argv) = 0;

    template<typename T>
    T& get();

    std::string default_value;
  };
  template<typename T>
  struct TaggedValue : Value
  {
    using Ptr = std::shared_ptr<TaggedValue<T>>;

    static Value::Ptr create()
    { return std::make_shared<TaggedValue<T>>(); }

    Value::Ptr clone() override
    { return std::make_shared<TaggedValue<T>>(*this); }

    void parse(int& i, int argc, const char** argv) override;

    T val;    
  };
  struct CmdOptionsAdder
  {
    CmdOptionsAdder(CmdOptions& ref);

    CmdOptionsAdder& operator()(std::string option, std::string description,
                                Value::Ptr value = std::make_shared<TaggedValue<bool>>(),
                                std::string default_value = "false");
  private:
    CmdOptions& ref;
  };
  struct Option
  {
    std::vector<std::string> long_names;
    std::vector<std::string> short_names;

    std::string description;
    
    Value::Ptr value;
  };
public:
  using ValueMapping = std::map<std::string, Value::Ptr>;

  CmdOptions(std::string name, std::string description);

  CmdOptionsAdder add_options();
  void add(std::string option, std::string description, Value::Ptr value = std::make_shared<TaggedValue<bool>>(),
                                                        std::string default_value = "false");
  ValueMapping parse(int argc, const char** argv);

  void print_help();
  void handle_unrecognized_option(std::string opt);
private:
  std::string name;
  std::string description;

  std::vector<Option> opts;
};

template<typename T>
void CmdOptions::TaggedValue<T>::parse(int& i, int argc, const char** argv)
{
  if constexpr(std::is_same<T, std::vector<std::string>>::value)
  {
    ++i;
    while(i < argc)
    {
      if(argv[i][0] == '-')
      {
        if(argv[i][1] == '-' && argv[i][2] == '\0')
        {
          val.push_back(argv[i++]);
          continue;
        }
        else
        {
          // another command line argument, exit earlier
          break;
        }
      }

      val.push_back(argv[i++]);
    }
  }
  else if constexpr(std::is_same<T, bool>::value)
  {
    // TODO: check for overrides, e.g. "--transmogrify yes"

    constexpr const char* truth[]   = { "yes", "true", "t", "y" };
    constexpr const char* falsity[] = { "no", "false", "n", "f" };

    auto cpy = default_value;
    std::transform(cpy.begin(), cpy.end(), cpy.begin(), [](unsigned char c) { return std::tolower(c); });

    if(std::find(std::begin(truth), std::end(truth), cpy) != std::end(truth))
    {
      //default is true, so we set val to false
      val = false;
    }
    else if(std::find(std::begin(falsity), std::end(falsity), cpy) != std::end(falsity))
    {
      //default is false, so we set val to true
      val = true;
    }
    ++i;
  }
  else
  {
    std::stringstream ss(argv[i++]);
    ss >> val;
  }
}

template<typename T>
T& CmdOptions::Value::get()
{ return static_cast<TaggedValue<T>&>(*this).val; }

template<typename T, typename H = std::uint_fast32_t>
constexpr H hash_string(T str)
{
  if(!(&str[0])) return 0;
  H hash = str[0];
  for(auto* p = &str[0]; p && *p != '\0'; p++)
    hash ^= (hash * 31) + (*p);
  return hash;
}

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

std::ostream& operator<<(std::ostream& os, const std::vector<Symbol>& symbs);


