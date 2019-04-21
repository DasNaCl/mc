#include <tokenizer.hpp>
#include <parser.hpp>

#include <ast_printer.hpp>

#include <util.hpp>

#include <iostream>
#include <fstream>

int main(int argc, const char* argv[])
{
  CmdOptions opt("mf", "This is the compiler for the mf language.");
  opt.add_options()
    ("t,just-tokenize", "Emit tokens of given modules.")
    ("p,just-parse", "Emit abstract syntax tree of given modules.")
    (",-,f,files", "List of files to compile.", CmdOptions::TaggedValue<std::vector<std::string>>::create(), "")
    ;
  auto map = opt.parse(argc, argv);
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
      ASTPrinter printer;
      printer.visit_all(astnodes);
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
      } while(tok.kind != TokenKind::EndOfFile);
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
