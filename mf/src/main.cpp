#include <tokenizer.hpp>
#include <iostream>
#include <fstream>

int main(int argc, const char* argv[])
{
  Tokenizer tokenizer = Tokenizer(std::cin);

  Token tok;
  do
  {
    tok = tokenizer.get();
    std::cout << "Token: " << static_cast<std::string>(tok) << "\n";
  } while(tok.kind != TokenKind::EndOfFile);

  return 0;
}
