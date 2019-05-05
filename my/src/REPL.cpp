#include "REPL.hpp"

#include "tokenizer.hpp"
#include "parser.hpp"
#include "ast.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cctype>

void REPL::loop()
{
  Statement::Ptr root;
  while(!(std::dynamic_pointer_cast<ErrorStatement>(root)))
  {
    if(std::cin.eof())
      break;
    if(!root)
    {
      // parse STDIN
      auto v = parse_input(); // <- TODO: fixme to be more generic
      if(v.empty())
        continue;
      root = v.front();
    }
    else
    {
      // apply evaluation strategy
      // TODO: lookup strategy in global config [<- can be served via cmd arg]
      ///// for now just cbv

      std::string last_ast, new_ast;
      do
      {
        root = root->eval(EvaluationStrategy::CallByValue);

        std::stringstream aststream;
        root->print(aststream);

        new_ast = aststream.str();
        last_ast = new_ast;
      } while(last_ast != new_ast);
      
      // print
      root->print(std::cout);
      std::cout << "\n";

      root = nullptr;
    }
  }
}

std::vector<Statement::Ptr> REPL::parse_input()
{
  std::cout << " > ";
redo:
  std::string input;

  std::getline(std::cin, input);
  std::transform(input.begin(), input.end(), input.begin(),
                 [](unsigned char ch) { return std::tolower(ch); });

  if(input == "q" || input == "quit")
    return { std::make_shared<ErrorStatement>(SourceRange()) };
  else if(std::cin.eof())
    return {};
  else if(input.empty())
    goto redo;

  std::stringstream ss(input);
  Tokenizer tokenizer("REPL", ss);

  return parse(tokenizer);
}

