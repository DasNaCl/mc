#pragma once

#include <typeindex>
#include <cstdint>
#include <vector>
#include <string>

std::uint_fast32_t hash_combine(std::uint_fast32_t lhs, std::uint_fast32_t rhs);

struct CmdOptions
{
  struct CmdOptionsAdder
  {
    CmdOptionsAdder(CmdOptions& ref);

    CmdOptionsAdder& operator()(std::string option, std::string description,
                                std::type_index type = typeid(bool), std::string default_value = "false");
  private:
    CmdOptions& ref;
  };
  struct Option
  {
    std::vector<std::string> names;
    std::string description;
    std::type_index type;
    std::string default_value;

    void parse(int i, int argc, const char** argv);
  };
public:
  CmdOptions(std::string name, std::string description);

  CmdOptionsAdder add_options();
  void add(std::string option, std::string description, std::type_index type = typeid(bool),
                                                        std::string default_value = "false");

  void parse(int argc, const char** argv);

  void print_help();
  void handle_unrecognized_option(std::string opt);
private:
  std::string name;
  std::string description;

  std::vector<Option> opts;
};

