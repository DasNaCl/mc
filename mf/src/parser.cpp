#include <parser.hpp>
#include <tokenizer.hpp>

struct Parser
{
public:
  Parser(Tokenizer& tokenizer)
    : tokenizer(tokenizer), ast()
  {  }
  
  std::vector<Statement::Ptr> parse() &&
  {
    if(peek(TokenKind::EndOfFile))
      return {};

    while(!peek(TokenKind::EndOfFile))
    {
      ast.emplace_back(parse_root());
    }
    return std::move(ast);
  }
private:
  void next_token()
  {
    current_token = lookahead.first;
    lookahead.first = lookahead.second;
    lookahead.second = tokenizer.get();
  }
  
  bool peek(TokenKind kind) const
  { return current_token.kind == kind; }

  bool accept(TokenKind kind)
  {
    if(peek(kind))
    {
      next_token();
      return true;
    }
    return false;
  }

  void expect(TokenKind kind)
  {
    if(!accept(kind))
      ; // error
  }
private:
  Tokenizer& tokenizer;
  std::vector<Statement::Ptr> ast;

  Token current_token;
  std::pair<Token, Token> lookahead;

private:
  // parsers
  
  Statement::Ptr parse_root()
  { return parse_function(); }

  Statement::Ptr parse_function(bool is_top_level = false)
  {
    // allowed are:
    //  foo (...) (...) {}
    //  foo (...) bar (...) {}
    //  foo (...) bar (...) baz {}
    //  (...) bar (...) baz {}
    //  (...) bar (...) {}
    //  foo {} //<- this makes us assume an empty parameter list
    //  (...) {} // <- lambda, only ok if not top_level
    //
    // NOT allowed are:
    //  foo bar (...) {}
    //  {} foo (...)
    //  {} (...)
    //  (...) (...) baz {}
    //  foo (...) (...) {}
    
    SourceRange range = current_token.range;
    std::vector<Statement::Ptr> data;

    TokenKind last = TokenKind::Undef;
    while(!peek(TokenKind::LBrace))
    {
      if(peek(TokenKind::Id) && last != TokenKind::Id)
      {
        // parse id
        data.push_back(std::make_shared<Identifier>(range, Symbol(reinterpret_cast<const char*>(current_token.data))));
        expect(TokenKind::Id);
        last = TokenKind::Id;
      }
      else if(peek(TokenKind::LParen) && last != TokenKind::RParen)
      {
        expect(TokenKind::LParen);
        while(!peek(TokenKind::RParen))
          data.push_back(parse_parameter());
        expect(TokenKind::RParen);
        last = TokenKind::RParen;
      }
      else
      {
        // emit error
        if(last == TokenKind::Id)
        {
          // two consecutive identifiers
        }
        else if(last == TokenKind::RParen)
        {
          // two consecutive parameter lists
        }
        else
        {
          // different error
        }
        last = TokenKind::Undef;
      }
    }
    if(is_top_level && data.size() < 2) // cant have a parameter list
    {
      // TODO: emit error
    }
    data.push_back(parse_block());
    return std::make_shared<Function>(range, data);
  }

  Statement::Ptr parse_block()
  {
    // TODO
    return nullptr;
  }

  Statement::Ptr parse_parameter()
  {
    // TODO
    return nullptr;
  }
};

std::vector<Statement::Ptr> parse(Tokenizer& tokenizer)
{
  return Parser(tokenizer).parse();
}

