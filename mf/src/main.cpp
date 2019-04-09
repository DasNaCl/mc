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
  opt.parse(argc, argv);

  Tokenizer tokenizer = Tokenizer("STDIN", std::cin);

  Token tok;
  do
  {
    std::cout << (tok = tokenizer.get()) << "\n";
  } while(tok.kind != TokenKind::EndOfFile);

  auto astnodes = parse(tokenizer);

  return astnodes.empty() ? 1 : 0;
}
