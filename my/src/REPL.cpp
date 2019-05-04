#include "REPL.hpp"

#include "tokenizer.hpp"
#include "parser.hpp"
#include "ast.hpp"

#include <algorithm>
#include <iostream>
#include <cctype>

void REPL::loop()
{
  Statement::Ptr root;
  std::string input;
  while(input != "q" && input != "quit")
  {
    if(std::cin.eof())
    {
      input = "q";
      continue;
    }
    if(!root)
    {
      // parse STDIN
      input = parse_input();
    }
    else
    {
      // apply evaluation strategy
      
    }
  }
}

std::string REPL::parse_input()
{
redo:
  std::cout << " > ";
  std::string input;

  std::getline(std::cin, input);
  std::transform(input.begin(), input.end(), input.begin(),
                 [](unsigned char ch) { return std::tolower(ch); });

  if(input == "q" || input == "quit")
    return input;
  else if(input.empty())
    goto redo;

  /*
  auto toks = tokenize(input);
  if(toks.size() > 0 && toks[0].tok_kind() != TokenKind::Equal
  && (toks.size() <= 1 || toks[1].tok_kind() != TokenKind::Equal))
    toks.insert(toks.begin(), Token(TokenKind::Equal, "="));
  TODO
  */
  return "";
}

