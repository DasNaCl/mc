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
      root = parse_input().front(); // <- TODO: fixme to be more generic
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

      root = nullptr;
    }
  }
}

std::vector<Statement::Ptr> REPL::parse_input()
{
redo:
  std::cout << " > ";
  std::string input;

  std::getline(std::cin, input);
  std::transform(input.begin(), input.end(), input.begin(),
                 [](unsigned char ch) { return std::tolower(ch); });

  if(input == "q" || input == "quit")
    return { std::make_shared<ErrorStatement>(SourceRange()) };
  else if(input.empty())
    goto redo;

  std::stringstream ss;
  Tokenizer tokenizer("REPL", ss);

  return parse(tokenizer);
}

