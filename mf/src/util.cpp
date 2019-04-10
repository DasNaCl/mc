#include <util.hpp>

#include <algorithm>
#include <iostream>
#include <cassert>

std::uint_fast32_t hash_combine(std::uint_fast32_t lhs, std::uint_fast32_t rhs)
{ return lhs ^ rhs + 0x517cc1b7 + (lhs << 6U) + (lhs >> 2U); }

template<>
void CmdOptions::TaggedValue<std::vector<std::string>>::parse(int& i, int argc, const char** argv)
{
  while(i < argc)
  { val.push_back(argv[i++]); }
}

template<>
void CmdOptions::TaggedValue<bool>::parse(int& i, int argc, const char** argv)
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

CmdOptions::CmdOptions(std::string name, std::string description)
  : name(name), description(description), opts()
{  }

CmdOptions::CmdOptionsAdder CmdOptions::add_options()
{
  return CmdOptionsAdder(*this);
}

void CmdOptions::add(std::string option, std::string description, CmdOptions::Value::Ptr val, std::string default_value)
{
  std::vector<std::string> short_names, long_names;
  
  std::stringstream ss(option);
  for(std::string str; std::getline(ss, str, ','); )
  {
    if(!str.empty())
    {
      (str.size() > 1 ? long_names : short_names).push_back(str);
    }
  }
  auto value = val->clone();
  value->default_value = default_value;

  opts.push_back({long_names, short_names, description, value});
}

CmdOptions::ValueMapping CmdOptions::parse(int argc, const char** argv)
{
  ValueMapping map;
  for(auto& opt : opts)
  {
    for(auto& n : opt.short_names)
      map[n] = opt.value;
    for(auto& n : opt.long_names)
      map[n] = opt.value;
  }
  for(int i = 1; i < argc; )
  {
    // argument must be prefixed with - or --
    std::string arg = argv[i];
    assert(!arg.empty());

    bool long_arg = false;
    if(arg[0] == '-')
    {
      if(arg[1] == '-' && arg.size() > 1)
      {
        long_arg = true;
        arg.erase(arg.begin());
        arg.erase(arg.begin());
      }
      else
      {
        arg.erase(arg.begin());
      }
    }
    else
    {
      handle_unrecognized_option(arg);
      continue;
    }

    if(long_arg)
    {
      auto it = std::find_if(opts.begin(), opts.end(), [arg](const Option& opt)
                             { return std::find(opt.long_names.begin(), opt.long_names.end(), arg) != opt.long_names.end(); });
      if(it == opts.end())
      {
        handle_unrecognized_option(argv[i]);
        ++i;
      }
      else
      {
        for(; it != opts.end(); ++it)
          it->value->parse(i, argc, argv);
      }
    }
    else
    {
      auto it = std::find_if(opts.begin(), opts.end(),
                [arg](const Option& opt)
                { return std::find_if(opt.short_names.begin(), opt.short_names.end(), [arg](const std::string& str)
                                                                          { return arg.find(str) != std::string::npos; }) != opt.short_names.end(); });
      do
      {
        if(it == opts.end())
        {
          handle_unrecognized_option(argv[i]);
          ++i;
        }
        else
        {
          it->value->parse(i, argc, argv);
        }
        it = std::find_if(std::next(it), opts.end(),
                [arg](const Option& opt)
                { return std::find_if(opt.short_names.begin(), opt.short_names.end(), [arg](const std::string& str)
                                                                          { return arg.find(str) != std::string::npos; }) != opt.short_names.end(); });
      } while(it != opts.end());
    }
  }
  return map;
}

void CmdOptions::print_help()
{
  std::cout << name << "  -  " << description << "\n"
            << "Usage:\n";
  for(auto& opt : opts)
  {
    std::cout << "\t";
    for(auto beg = opt.short_names.begin(), lbeg = opt.long_names.begin(); std::next(lbeg) != opt.long_names.end(); beg == opt.short_names.end() ? ++lbeg : ++beg)
      std::cout << *(beg == opt.short_names.end() ? lbeg : beg) << ",";

    std::cout << opt.long_names.back() << "\t" << opt.description << std::endl;
  }
}

void CmdOptions::handle_unrecognized_option(std::string opt)
{
  std::cout << "Unrecognized option: \"" << opt << "\"\n";
}

CmdOptions::CmdOptionsAdder::CmdOptionsAdder(CmdOptions& ref)
  : ref(ref)
{  }

CmdOptions::CmdOptionsAdder& CmdOptions::CmdOptionsAdder::operator()(std::string option, std::string description, CmdOptions::Value::Ptr value, std::string default_value)
{
  ref.add(option, description, value, default_value);

  return *this;
}

thread_local tsl::hopscotch_map<std::uint_fast32_t, std::string> Symbol::symbols = {};

Symbol::Symbol(const std::string& str)
  : hash(hash_string(str)), str(symbols[hash])
{  }

Symbol::Symbol(const char* str)
  : hash(hash_string(str)), str(symbols[hash])
{  }

