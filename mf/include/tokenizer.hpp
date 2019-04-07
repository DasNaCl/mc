#pragma once

#include <cstdint>
#include <istream>
#include <string>

enum class TokenKind : std::int_fast16_t
{
#define TOK(x) x,
#define TOK_CONTROL(x, v) x = v,
#define TOK_EXPR_OP(x, v) x = v,

#include "tokens.def"
};

struct Token
{
  TokenKind kind;
  //SourceRange range;
  void* data;

  Token() = default;
  Token(TokenKind kind)
    : kind(kind)
  {  }
  Token(TokenKind kind, void* data)
    : kind(kind), data(data)
  {  }

  explicit operator std::string() const;
};

class Tokenizer
{
public:
  Tokenizer(std::istream& handle);

  Token get();
private:
  char read(); 

private:
  std::istream& handle;
  std::string linebuf;
  std::size_t row;
  std::size_t col; 
};

