#include <parser.hpp>
#include <tokenizer.hpp>
#include <log.hpp>

#include <tsl/bhopscotch_set.h>

#include <cassert>

static constexpr std::uint_fast32_t type_ids[] = { hash_string("byte"),
                                                   hash_string("char"),  hash_string("uchar"),
                                                   hash_string("short"), hash_string("ushort"),
                                                   hash_string("int"),   hash_string("uint"),
                                                   hash_string("long"),  hash_string("ulong") };
static constexpr std::size_t type_ids_len = sizeof(type_ids) / sizeof(type_ids[0]);

struct Parser
{
public:
  Parser(Tokenizer& tokenizer)
    : tokenizer(tokenizer), ast()
  {
    next_token(); next_token(); next_token();

    for(std::size_t i = 0; i < type_ids_len; ++i)
      type_identifiers.emplace(type_ids[i]);
  }
  
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

  MessageCollector emit_error()
  {
    return ::emit_error(tokenizer.module_name(), current_token.range.column_beg, current_token.range.row_beg);
  }

  void skip_to_next_toplevel()
  {
    bool another_fn = true;
    do
    {
      while(!peek(TokenKind::RBrace) && !peek(TokenKind::EndOfFile))
      {
        next_token();
      }
      next_token();

      another_fn = (n1_peek(TokenKind::Id) && n2_peek(TokenKind::LParen))
                || (n1_peek(TokenKind::LParen) && n2_peek(TokenKind::Id));
    } while(!another_fn && !peek(TokenKind::EndOfFile));
  }

  ErrorExpression::Ptr error_expr(SourceRange range)
  { return std::make_shared<ErrorExpression>(range); } // TODO: Add more error context info
  ErrorExpression::Ptr error_expr()
  { return std::make_shared<ErrorExpression>(current_token.range); } // TODO: Add more error context info

  ErrorStatement::Ptr error_stmt(SourceRange range)
  { return std::make_shared<ErrorStatement>(range); } // TODO: Add more error context info
  ErrorStatement::Ptr error_stmt()
  { return std::make_shared<ErrorStatement>(current_token.range); } // TODO: Add more error context info

  ErrorType::Ptr error_type()
  { return std::make_shared<ErrorType>(); } // TODO: Add more error context info
private:
  void next_token()
  {
    current_token = lookahead.first;
    lookahead.first = lookahead.second;
    lookahead.second = tokenizer.get();
  }
  
  bool peek(TokenKind kind) const
  { return current_token.kind == kind; }

  bool n1_peek(TokenKind kind) const
  { return lookahead.first.kind == kind; }

  bool n2_peek(TokenKind kind) const
  { return lookahead.second.kind == kind; }

  bool accept(TokenKind kind)
  {
    if(peek(kind))
    {
      next_token();
      return true;
    }
    return false;
  }

  bool expect(TokenKind kind)
  {
    if(!accept(kind))
    {
      emit_error() << "expected token \"" << to_string(kind) << "\" but got \"" << to_string(current_token.kind) << "\".";

      return false;
    }
    return true;
  }

  bool lookup_type_id(const char* identifier)
  {
    return type_identifiers.find(hash_string(identifier)) != type_identifiers.end();
  }
private:
  Tokenizer& tokenizer;
  std::vector<Statement::Ptr> ast;

  Token current_token;
  std::pair<Token, Token> lookahead;

  tsl::bhopscotch_set<std::uint_fast32_t> type_identifiers;

private:
  // parsers
  
  Statement::Ptr parse_root()
  { return parse_function(true); }

  Statement::Ptr parse_function(bool is_top_level = false)
  {
    SourceRange range = current_token.range;
    std::vector<Statement::Ptr> data;

    TokenKind last = TokenKind::Undef;
    std::string last_ids;
    while(!peek(TokenKind::Arrow))
    {
      if(peek(TokenKind::Id) && last != TokenKind::Id)
      {
        if(!last_ids.empty())
          last_ids += "-";
        last_ids += reinterpret_cast<const char*>(current_token.data);

        data.push_back(parse_identifier());
        last = TokenKind::Id;
      }
      else if((peek(TokenKind::LParen) && last != TokenKind::RParen)
           || (peek(TokenKind::Id) && lookup_type_id(reinterpret_cast<const char*>(current_token.data))))
      {
        std::optional<std::vector<Parameter::Ptr>> params = parse_parameters();
        if(!params.has_value())
        {
          skip_to_next_toplevel();
          return error_stmt();
        }

        data.push_back(std::make_shared<Parameters>(range, params.value()));
        last = TokenKind::RParen;
      }
      else
      {
        if(last == TokenKind::Id)
        {
          // two consecutive identifiers
          emit_error() << "Function was already declared with identifier \"" << last_ids << "\".";
        }
        else if(last == TokenKind::RParen)
        {
          // two consecutive parameter lists
          if(last_ids.empty())
            emit_error() << "Anonymous function already has a parameter list.";
          else
            emit_error() << "Function \"" << last_ids << "\" already has a parameter list.";
        }
        else
        {
          // different error
          emit_error() << "Could not parse function.";
        }
        skip_to_next_toplevel();
        return error_stmt();
      }
    }
    if(is_top_level && data.size() < 2)
    {
      if(last_ids.empty())
        emit_error() << "Top-level functions must not be anonymous.";
      else
        emit_error() << "Top-level function \"" << last_ids << "\" has no parameter list.";
      skip_to_next_toplevel();
      return error_stmt();
    }
    if(!expect(TokenKind::Arrow))
    {
      return error_stmt();
    }
    auto ret_typ = parse_type();
    data.push_back(parse_block());
    return std::make_shared<Function>(range, data, ret_typ);
  }

  Statement::Ptr parse_block()
  {
    SourceRange range = current_token.range;
    if(!expect(TokenKind::LBrace))
      return error_stmt();

    std::vector<Statement::Ptr> statements;
    while(!peek(TokenKind::RBrace))
    {
      auto stmt = parse_expression_statement();
      statements.emplace_back(stmt);
    }
    if(!expect(TokenKind::RBrace))
      return error_stmt();
    range.widen(current_token.range);

    return std::make_shared<Block>(range, statements);
  }

  std::optional<std::vector<Parameter::Ptr>> parse_parameters()
  {
    auto range = current_token.range;
    if(!expect(TokenKind::LParen))
      return std::nullopt;
    std::vector<Parameter::Ptr> pars;
    if(peek(TokenKind::RParen))
    {
      range.widen(current_token.range);
      expect(TokenKind::RParen);

      // f () -> (), () f -> (), () -> ()
      pars.emplace_back(std::make_shared<Parameter>(range, nullptr, std::make_shared<Unit>())); 
      return pars;
    }
    do
    {
      auto par = parse_parameter();
      if(!par.has_value())
        return std::nullopt;

      pars.emplace_back(par.value());
    } while(accept(TokenKind::Comma)); 
    if(!expect(TokenKind::RParen))
      return std::nullopt;
    return pars;
  }

  std::optional<Parameter::Ptr> parse_parameter()
  {
    auto range = current_token.range; // TODO: fix range to span along `foo: int`?
    auto id = parse_identifier();
    if(!expect(TokenKind::DoubleColon))
      return std::nullopt;
    auto typ = parse_type();
    return std::make_shared<Parameter>(range, id, typ);
  }

  Statement::Ptr parse_identifier()
  {
    auto range = current_token.range;
    const char* data = reinterpret_cast<const char*>(current_token.data);

    if(lookup_type_id(data))
    {
      emit_error() << "Type \"" << data << "\" used as identifier.";
      return error_stmt();
    }

    if(!expect(TokenKind::Id))
      return error_stmt();
    return std::make_shared<Identifier>(range, Symbol(data));
  }

  Statement::Ptr parse_expression_statement()
  {
    auto expr = parse_expression(0);
    
    return std::make_shared<ExpressionStatement>(expr);
  }

  std::int_fast32_t lbp(TokenKind kind) const
  {
    // TODO: Lookup operator in table
    switch(kind)
    {
      default: return 0; // error: Unknown token

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

  Type::Ptr parse_type(Type::Ptr type = nullptr)
  {
    do
    {
      switch(current_token.kind)
      {
      default: return type;

      case TokenKind::Id: // char, int, uchar, uint, short, ushort
        if(lookup_type_id(reinterpret_cast<const char*>(current_token.data)))
        {
          Symbol name = reinterpret_cast<const char*>(current_token.data);
          expect(TokenKind::Id);
          type = std::make_shared<PrimitiveType>(name);
        }
        else
        {
          emit_error() << "Unknown type \"" << reinterpret_cast<const char*>(current_token.data) << "\".";
          return error_type();
        }
       break;

      case TokenKind::LParen:  // (), (() -> ()), (int -> ()), ((int, int) -> ()) (((int)))
        expect(TokenKind::LParen);
        if(accept(TokenKind::RParen))
        {
          type = std::make_shared<Unit>();
        }
        else
        {
          type = parse_type(type);

          expect(TokenKind::RParen);
        }
       break;

      case TokenKind::Arrow:
        expect(TokenKind::Arrow);
        auto ret = parse_type();
        type = std::make_shared<FunctionType>(type, ret);
      break;
      }
    } while(true);
  }
};

std::vector<Statement::Ptr> parse(Tokenizer& tokenizer)
{
  return Parser(tokenizer).parse();
}

