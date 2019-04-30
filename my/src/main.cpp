
#include <tokenizer.hpp>
#include <parser.hpp>

#include <myopts.hpp>

#include <iostream>
#include <fstream>

#ifndef NDEBUG
#include <csignal>

void do_nothing_on_sigint(int)
{  }
#endif

int main(int argc, const char* argv[])
{
  CmdOptions opt("mf", "This is the compiler for the mf language.");
  opt.add_options()
    ("t,just-tokenize", "Emit tokens of given modules.")
    ("p,just-parse", "Emit abstract syntax tree of given modules.")
    (",-,f,files", "List of files to compile.", CmdOptions::TaggedValue<std::vector<std::string>>::create(), "")

#ifndef NDEBUG
    ("enable-breakpoints", "DEBUG: Emit SIGINT signal whenever something interesting happens. Useful for debugging.")
#endif
    ;
  auto map = opt.parse(argc, argv);

#ifndef NDEBUG
  if(map["enable-breakpoints"]->get<bool>())
  {
    StaticOptions::enable_breakpoints();
    std::signal(SIGINT, do_nothing_on_sigint);
  }
#endif

  std::vector<std::ifstream> input_files;
  std::vector<Tokenizer> tokenizers;
  if(auto vec = map["f"]->get<std::vector<std::string>>(); !vec.empty())
  {
    std::transform(vec.begin(), vec.end(), std::back_inserter(tokenizers),
                   [&input_files](const std::string& module)
                   {
                     if(module == "--")
                       return Tokenizer("STDIN", std::cin);
                     input_files.emplace_back(module);
                     if(!input_files.back())
                     {
                       // TODO: Error
                       assert(false);
                     }
                     return Tokenizer(module.c_str(), input_files.back());
                   });
  }

  if(map["p"]->get<bool>())
  {
    for(auto& tokenizer : tokenizers)
    {
      auto astnodes = parse(tokenizer);

      std::cout << "Abstract syntax tree of module \"" << tokenizer.module_name() << "\": \n";
      for(auto& astnode : astnodes)
      {
        astnode->print(std::cout);
        std::cout << "\n";
      }
    }
  }
  else if(map["t"]->get<bool>())
  {
    for(auto it = tokenizers.begin(); it != tokenizers.end(); ++it)
    {
      auto& tokenizer = *it;
      Token tok;
      do
      {
        std::cout << (tok = tokenizer.get()) << "\n";
      } while(tok.tok_kind() != TokenKind::EndOfFile);
      if(it + 1 != tokenizers.end())
        std::cout << "\n";
    }
  }
  else
  {
    std::cout << "Nothing selected, so will do nothing.\n";
  }
  return 0;
}
