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
void CmdOptions::TaggedValue<T>::parse(int& i, int , const char** argv)
{
  std::stringstream ss(argv[i++]);
  ss >> val;
}


template<typename T>
T& CmdOptions::Value::get()
{ return static_cast<TaggedValue<T>&>(*(clone().get())).val; }

template<typename T>
std::uint_fast32_t hash_string(T str)
{
  if(!(&str[0])) return 0;
  std::uint_fast32_t hash = str[0];
  for(auto* p = &str[0]; p && *p != '\0'; p++)
    hash ^= (hash * 31) + (*p);
  return hash;
}

struct Symbol
{
  Symbol(const std::string& str);
  Symbol(const char* str);

private:
  static thread_local tsl::hopscotch_map<std::uint_fast32_t, std::string> symbols;

  std::uint_fast32_t hash;
  std::string& str;
};


