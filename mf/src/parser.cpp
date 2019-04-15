#include <parser.hpp>
#include <tokenizer.hpp>

#include <cassert>

struct Parser
{
public:
  Parser(Tokenizer& tokenizer)
    : tokenizer(tokenizer), ast()
  { next_token(); next_token(); next_token(); }
  
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
  { return parse_function(true); }

  Statement::Ptr parse_function(bool is_top_level = false)
  {
    SourceRange range = current_token.range;
    std::vector<Statement::Ptr> data;

    TokenKind last = TokenKind::Undef;
    while(!peek(TokenKind::LBrace))
    {
      if(peek(TokenKind::Id) && last != TokenKind::Id)
      {
        data.push_back(std::make_shared<Identifier>(current_token.range,
                                                    Symbol(reinterpret_cast<const char*>(current_token.data))));
        expect(TokenKind::Id);
        last = TokenKind::Id;
      }
      else if(peek(TokenKind::LParen) && last != TokenKind::RParen)
      {
        std::vector<Parameter::Ptr> params;
        expect(TokenKind::LParen);
        while(!peek(TokenKind::RParen))
          params.push_back(parse_parameter());
        expect(TokenKind::RParen);

        data.push_back(std::make_shared<Parameters>(range, params));
        last = TokenKind::RParen;
      }
      else
      {
        // emit error
        if(last == TokenKind::Id)
        {
          // two consecutive identifiers
          assert(false);
        }
        else if(last == TokenKind::RParen)
        {
          // two consecutive parameter lists
          assert(false);
        }
        else
        {
          // different error
          assert(false);
        }
        last = TokenKind::Undef;
      }
    }
    if(is_top_level && data.size() < 2) // cant have a parameter list
    {
      // TODO: emit error
      assert(false);
    }
    data.push_back(parse_block());
    return std::make_shared<Function>(range, data);
  }

  Statement::Ptr parse_block()
  {
    SourceRange range = current_token.range;
    expect(TokenKind::LBrace);

    std::vector<Statement::Ptr> statements;
    while(!peek(TokenKind::RBrace))
    {
      statements.emplace_back(parse_expression_statement());
    }
    expect(TokenKind::RBrace);
    range.widen(current_token.range);

    return std::make_shared<Block>(range, statements);
  }

  Parameter::Ptr parse_parameter()
  {
    // TODO
    return nullptr;
  }

  Statement::Ptr parse_expression_statement()
  {
    return std::make_shared<ExpressionStatement>(parse_expression(0));
  }

  std::int_fast32_t lbp(TokenKind kind) const
  {
    // TODO: Lookup operator in table
    switch(kind)
    {
    default: return 0;

    case TokenKind::Plus: return 100;
    }
  }

  Expression::Ptr nud(Token tok)
  {
    // TODO: Lookup unary op in table
    switch(tok.kind)
    {
    default: return nullptr;

    case TokenKind::Id:
    case TokenKind::Number:
    case TokenKind::String:
    case TokenKind::Character:
         return std::make_shared<LiteralExpression>(tok);
    }
  }
  
  Expression::Ptr led(TokenKind kind, Expression::Ptr left)
  {
    // TODO: Lookup parser in table with operators
    
    Expression::Ptr right = nullptr;
    switch(kind)
    {
    default: assert(false); return nullptr;
    
    case TokenKind::Plus:
         right = parse_expression(lbp(TokenKind::Plus)); 
         return std::make_shared<BinaryExpression>(left, right);

         // for right-assoc, recurse into expression(lbp(tok) - 1)
    }
  }

  Expression::Ptr parse_expression(std::int_fast32_t rbp)
  {
    auto t = current_token;
    next_token();
    Expression::Ptr left = nud(t);
    while(rbp < lbp(current_token.kind))
    {
      t = current_token;
      next_token();
      left = led(t.kind, left);
    }
    return left;
  }
};

std::vector<Statement::Ptr> parse(Tokenizer& tokenizer)
{
  return Parser(tokenizer).parse();
}

