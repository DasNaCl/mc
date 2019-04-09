#include <tokenizer.hpp>
#include <parser.hpp>

#include <util.hpp>

#include <iostream>
#include <fstream>

int main(int argc, const char* argv[])
{
  CmdOptions opt("mf", "This is the compiler for the mf language.");
  opt.add_options()
    ("t,just-tokenize", "Emit tokens of given modules.")
    ("p,just-parse", "Emit abstract syntax tree of given modules.")
    ;
  auto map = opt.parse(argc, argv);

  Tokenizer tokenizer = Tokenizer("STDIN", std::cin);
  if(map["p"]->get<bool>())
  {
    auto astnodes = parse(tokenizer);

    std::cout << "total nodes: " << astnodes.size() << "\n";
  }
  else if(map["t"]->get<bool>())
  {
    Token tok;
    do
    {
      std::cout << (tok = tokenizer.get()) << "\n";
    } while(tok.kind != TokenKind::EndOfFile);
  }

  return 0;
}
