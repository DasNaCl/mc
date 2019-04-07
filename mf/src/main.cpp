#include <tokenizer.hpp>
#include <iostream>
#include <fstream>

int main(int argc, const char* argv[])
{
  Tokenizer tokenizer = Tokenizer("STDIN", std::cin);

  Token tok;
  do
  {
    std::cout << (tok = tokenizer.get()) << "\n";
  } while(tok.kind != TokenKind::EndOfFile);

  return 0;
}
