#include <parser.hpp>
#include <tokenizer.hpp>
#include <scope.hpp>
#include <log.hpp>

#include <tsl/bhopscotch_set.h>

#include <iostream>
#include <cassert>

struct TrieNode : public std::enable_shared_from_this<TrieNode>
{
private:
  std::uint_fast32_t calc_hash(Symbol symb)
  { return (1U << 31U) ^ (~(1U << 31U) & (symb.get_hash() + 1)); } 
  std::uint_fast32_t calc_hash(Type::Ptr typ)
  { return ~(1U << 31U) & (typ->shared_id() + 1); }
public:
  using Ptr = std::shared_ptr<TrieNode>;

  TrieNode(Symbol name)
    : dat(name), hash(calc_hash(name))
  {  }

  TrieNode(Type::Ptr typ)
    : dat(typ), hash(calc_hash(typ))
  {  }

  TrieNode()
    : dat(std::monostate{}), hash(0), current_node()
  {  }

  TrieNode::Ptr add_node(TrieNode::Ptr n)
  {
    if(nodes.find(n->hash) == nodes.end())
      nodes[n->hash] = n;
    return nodes[n->hash];
  }

  bool lookup_step(std::variant<Type::Ptr, Symbol> typ_symb)
  {
    if(!current_node)
      current_node = shared_from_this();
    const std::uint_fast32_t hash = (std::holds_alternative<Type::Ptr>(typ_symb)
                                   ? calc_hash(std::get<Type::Ptr>(typ_symb))
                                   : calc_hash(std::get<Symbol>(typ_symb)));
    auto it = current_node->nodes.find(hash);
    if(it == current_node->nodes.end())
      return false;
    current_node = it->second->shared_from_this();
    return true;
  }

  bool is_parsable()
  {
    if(!current_node)
      current_node = shared_from_this();
    return current_node->nodes.find(0) != current_node->nodes.end();
  }

  bool is_end()
  {
    if(!current_node)
      current_node = shared_from_this();
    return (current_node->nodes.size() == 1)
        && (current_node->nodes.find(0) != current_node->nodes.end());
  }

  void reset()
  {
    current_node = nullptr;
  }

  std::variant<std::monostate, Symbol, Type::Ptr> dat;
  std::uint_fast64_t hash;
  std::unordered_map<std::uint_fast64_t, TrieNode::Ptr> nodes;

  TrieNode::Ptr current_node;
};

struct Parser
{
public:
  Parser(Tokenizer& tokenizer)
    : tokenizer(tokenizer), ast()
  {
    next_token(); next_token(); next_token();

    for(std::size_t i = 0; i < prim_types_len; ++i)
      type_identifiers.emplace(prim_types[i]);
  }

  Parser& preprocess() &&
  {
    TrieNode::Ptr root = std::make_shared<TrieNode>();
    while(!peek(TokenKind::EndOfFile))
    {                // read top-level functions and ignore blocks
      auto st = parse_function(true, true); 

      TrieNode::Ptr cpy = root;
      // ignore error statements
      if(std::dynamic_pointer_cast<ErrorStatement>(st))
        continue;
      auto fn = std::static_pointer_cast<Function>(st);
      auto sig = fn->signature();

      for(auto& var : sig)
      {
        if(std::holds_alternative<Identifier::Ptr>(var))
        {
          auto id = std::get<Identifier::Ptr>(var)->id();
          cpy = cpy->add_node(std::make_shared<TrieNode>(id));
        }
        else if(std::holds_alternative<Type::Ptr>(var))
        {
          auto typ = std::get<Type::Ptr>(var);
          cpy = cpy->add_node(std::make_shared<TrieNode>(typ));
        }
      }
      cpy->add_node(std::make_shared<TrieNode>());
    }
    expr_parse_tree = root;
    tokenizer.reset();
    next_token(); next_token(); next_token();
    return *this;
  }
  
  std::vector<Statement::Ptr> parse() &
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
    bool another_fn = (peek(TokenKind::Id)     && n1_peek(TokenKind::LParen))
                   || (peek(TokenKind::LParen) && (n1_peek(TokenKind::RParen) || (n1_peek(TokenKind::Id) && n2_peek(TokenKind::DoubleColon))));
    while(!another_fn && !peek(TokenKind::EndOfFile))
    {
      while(!peek(TokenKind::RBrace) && !peek(TokenKind::EndOfFile))
      {
        next_token();
      }
      next_token();

      another_fn = (peek(TokenKind::Id)     && n1_peek(TokenKind::LParen))
                || (peek(TokenKind::LParen) && (n1_peek(TokenKind::RParen) || (n1_peek(TokenKind::Id) && n2_peek(TokenKind::DoubleColon))));
    }
  }

  ErrorExpression::Ptr error_expr(SourceRange range)
  { return std::make_shared<ErrorExpression>(range); } // TODO: Add more error context info
  ErrorExpression::Ptr error_expr()
  { return std::make_shared<ErrorExpression>(current_token.range); } // TODO: Add more error context info

  ErrorStatement::Ptr error_stmt()
  { return std::make_shared<ErrorStatement>(current_token.range); } // TODO: Add more error context info
  ErrorStatement::Ptr error_stmt(SourceRange range)
  { range.widen(prev_tok_loc); return std::make_shared<ErrorStatement>(range); } // TODO: Add more error context info

  ErrorType::Ptr error_type()
  { return std::make_shared<ErrorType>(); } // TODO: Add more error context info
private:
  void next_token()
  {
    prev_tok_loc = current_token.range;

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
      emit_error() << "Expected token \"" << to_string(kind) << "\" but got \"" << to_string(current_token.kind) << "\".";

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

  SourceRange prev_tok_loc;

  tsl::bhopscotch_set<std::uint_fast32_t> type_identifiers;
  TrieNode::Ptr expr_parse_tree;

  Scope::Ptr current_scope;
private:
  // parsers
  
  Statement::Ptr parse_root()
  { return parse_function(true); }

  Statement::Ptr parse_function(bool is_top_level = false, bool ignore_body = false)
  {
    SourceRange range = current_token.range;
    std::vector<Statement::Ptr> data;
    Scope::Ptr scope = std::make_shared<Scope>();

    TokenKind last = TokenKind::Undef;
    std::string last_ids;
    while(!peek(TokenKind::Arrow))
    {
      if(peek(TokenKind::Id) && last != TokenKind::Id)
      {
        if(!last_ids.empty())
          last_ids += "$";
        last_ids += reinterpret_cast<const char*>(current_token.data);

        data.push_back(parse_identifier());
        last = TokenKind::Id;
      }
      else if((peek(TokenKind::LParen) && last != TokenKind::RParen)
           || (peek(TokenKind::Id) && lookup_type_id(reinterpret_cast<const char*>(current_token.data))))
      {
        SourceRange pars_range = current_token.range;
        std::optional<std::vector<Parameter::Ptr>> params = parse_parameters();
        if(!params.has_value())
        {
          skip_to_next_toplevel();
          return error_stmt(pars_range);
        }
        data.push_back(std::make_shared<Parameters>(pars_range, params.value()));
        for(auto& p : params.value())
        {
          scope->add(p->identifier()->id(), p->type());
        }
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
        return error_stmt(range);
      }
    }
    if(is_top_level && data.size() < 2)
    {
      if(last_ids.empty())
        emit_error() << "Top-level functions must not be anonymous.";
      else
        emit_error() << "Top-level function \"" << last_ids << "\" has no parameter list.";
      skip_to_next_toplevel();
      return error_stmt(range);
    }
    if(!expect(TokenKind::Arrow))
    {
      skip_to_next_toplevel();
      return error_stmt(range);
    }
    auto ret_typ = parse_type();

    // TODO: add function's name to scope of block to allow for recursive calls
    current_scope = scope;
    if(!ignore_body)
      data.push_back(parse_block()); // last token was '}'
    else
      skip_to_next_toplevel();
    range.widen(prev_tok_loc);
    return std::make_shared<Function>(range, scope, data, ret_typ);
  }

  Statement::Ptr parse_block()
  {
    SourceRange range = current_token.range;

    Scope::Ptr scope = std::make_shared<Scope>();
    current_scope->make_parent(*scope);

    if(!expect(TokenKind::LBrace))
      return error_stmt(range);

    std::vector<Statement::Ptr> statements;
    while(!peek(TokenKind::RBrace))
    {
      auto stmt = parse_expression_statement();
      statements.emplace_back(stmt);
    }
    if(!expect(TokenKind::RBrace))
      return error_stmt(range);
    range.widen(current_token.range);

    return std::make_shared<Block>(range, scope, statements);
  }

  std::optional<std::vector<Parameter::Ptr>> parse_parameters()
  {
    auto range = current_token.range;
    if(!expect(TokenKind::LParen))
      return std::nullopt;
    std::vector<Parameter::Ptr> pars;
    if(peek(TokenKind::RParen))
    {
      expect(TokenKind::RParen);
      range.widen(prev_tok_loc);

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
    auto range = current_token.range;
    auto id = parse_identifier();
    if(!expect(TokenKind::DoubleColon))
      return std::nullopt;
    auto typ = parse_type();
    range.widen(prev_tok_loc);
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
    auto expr = parse_expression();
    
    return std::make_shared<ExpressionStatement>(expr);
  }

  Expression::Ptr parse_literal()
  {
    auto tok = current_token;
    next_token();
    switch(tok.kind)
    {
    default: return error_expr();

    case TokenKind::Id:
    case TokenKind::Number:
    case TokenKind::String:
    case TokenKind::Character:
       return std::make_shared<LiteralExpression>(tok);
    }
  }

  Expression::Ptr parse_call()
  {
    expr_parse_tree->reset();

    auto range = current_token.range;
    bool keep_on_parsing = true;
    std::vector<Expression::Ptr> args;
    std::string fn_name;
    do
    {
      bool modified = false;
      switch(current_token.kind)
      {
        default: emit_error() << "Invalid function call syntax."; return error_expr();

        case TokenKind::RParen: modified = false; break; // <- TODO: remove in future upon parentheses elimination
        case TokenKind::Id:
           {
             std::string name(reinterpret_cast<const char*>(current_token.data));
             modified = expr_parse_tree->lookup_step(Symbol(name));

             if(modified)
             {
               if(!fn_name.empty())
                 fn_name += "$";
               fn_name += name;
             }
             else // must be an arg
             {
               args.emplace_back(std::make_shared<LiteralExpression>(current_token));
               auto type = current_scope->lookup(Symbol(name));

               // -> redo lookup step with arg's type
               modified = expr_parse_tree->lookup_step(type);
             }
           } break;
        case TokenKind::LParen:
           {
             auto inner = parse_expression();
             modified = expr_parse_tree->lookup_step(inner->type());

             if(modified)
               args.emplace_back(inner);
           } break;
      }
      if(!modified)
      {
        assert(expr_parse_tree->is_end());

        if(expr_parse_tree->is_parsable())
          keep_on_parsing = false;
        else
        {
          emit_error() << "No matching function to call to.";
          return error_expr(range);
        }
      } 
      else
        next_token();
    } while(keep_on_parsing);

    range.widen(prev_tok_loc);

    return std::make_shared<FunctionCall>(range, args, Symbol(fn_name));
  }

  Expression::Ptr parse_expression()
  {
    SourceRange range = current_token.range;

    // For now, expressions must be parenthesized: (...)
    //    Note: We ignore parentheses in source range except for the unit expression `()`
    expect(TokenKind::LParen);
    Expression::Ptr inner;
    if(n1_peek(TokenKind::RParen))
      inner = parse_literal();
    else if(accept(TokenKind::RParen))
    {
      range.widen(prev_tok_loc);
      inner = std::make_shared<UnitExpression>(range);
    }
    else
      inner = parse_call();
    expect(TokenKind::RParen);
    return inner;
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
  return Parser(tokenizer).preprocess().parse();
}

