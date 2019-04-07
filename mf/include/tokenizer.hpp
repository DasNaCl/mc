#pragma once

#include <SourceRange.hpp>

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
  SourceRange range;
  TokenKind kind;
  void* data;

  Token() = default;
  Token(SourceRange range, TokenKind kind)
    : range(range), kind(kind)
  {  }
  Token(SourceRange range, TokenKind kind, void* data)
    : range(range), kind(kind), data(data)
  {  }

  explicit operator std::string() const;

  friend std::ostream& operator<<(std::ostream& os, const Token& tok);
};

class Tokenizer
{
public:
  Tokenizer(const char* module, std::istream& handle);

  Token get();
private:
  char read(); 

private:
  const char* module;
  std::istream& handle;
  std::string linebuf;
  std::size_t row;
  std::size_t col; 
};

