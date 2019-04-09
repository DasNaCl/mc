#include <util.hpp>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cassert>

std::uint_fast32_t hash_combine(std::uint_fast32_t lhs, std::uint_fast32_t rhs)
{ return lhs ^ rhs + 0x517cc1b7 + (lhs << 6U) + (lhs >> 2U); }

CmdOptions::CmdOptions(std::string name, std::string description)
  : name(name), description(description)
{  }

CmdOptions::CmdOptionsAdder CmdOptions::add_options()
{
  return CmdOptionsAdder(*this);
}

void CmdOptions::add(std::string option, std::string description, std::type_index type, std::string default_value)
{
  std::vector<std::string> names;
  
  std::stringstream ss(option);
  for(std::string str; std::getline(ss, str, ','); )
  {
    if(!str.empty())
    {
      if(str.size() == 1)
      {
        // short arg
        
        names.push_back(str);
        names.push_back("-" + str);
      }
      else
      {
        // long arg
        
        names.push_back("--" + str);
      }
    }
  }

  opts.push_back({names, description, type, default_value});
}

void CmdOptions::parse(int argc, const char** argv)
{
  for(int i = 1; i < argc; ++i)
  {
    // argument must be prefixed with - or --
    const std::string arg = argv[i - 1];
    assert(!arg.empty());

    bool long_arg = false;
    bool short_arg = false;
    if(arg[0] == '-')
    {
      if(arg.size() > 1)
      {
        if(arg[1] == '-')
        {
          short_arg = true;
        }
        else
        {
          long_arg = true;
        }
      }
    }
    assert((long_arg && !short_arg) || (!long_arg && short_arg));
    // TODO: parse stuff like -a, -abc, -a -b -c, --long-arg
    
    auto it = std::find_if(opts.begin(), opts.end(),
                [arg](const Option& opt)
                { return std::find(opt.names.begin(), opt.names.end(), arg) != opt.names.end(); });

    if(it == opts.end())
    {
      handle_unrecognized_option(argv[i]);
    }
    else
    {
      it->parse(i, argc, argv);
    }
  }
}

void CmdOptions::print_help()
{
  std::cout << name << "  -  " << description << "\n"
            << "Usage:\n";
  for(auto& opt : opts)
  {
    std::cout << "\t";
    for(auto beg = opt.names.begin(); std::next(beg) != opt.names.end(); ++beg)
      std::cout << *beg << ",";
    std::cout << opt.names.back() << "\t\t"
              << opt.description << std::endl;
  }
}

void CmdOptions::handle_unrecognized_option(std::string opt)
{
  std::cout << "Unrecognized option: \"" << opt << "\"\n";
}

CmdOptions::CmdOptionsAdder::CmdOptionsAdder(CmdOptions& ref)
  : ref(ref)
{  }

CmdOptions::CmdOptionsAdder& CmdOptions::CmdOptionsAdder::operator()(std::string option, std::string description, std::type_index type, std::string default_value)
{
  ref.add(option, description, type, default_value);

  return *this;
}

