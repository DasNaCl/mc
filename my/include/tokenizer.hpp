#pragma once

#include <tsl/hopscotch_map.h>

#include <source_range.hpp>
#include <util.hpp>

#include <cstdint>
#include <istream>
#include <memory>
#include <string>

enum class TokenKind : std::int_fast16_t
{
#define TOK(x) x,
#define TOK_CONTROL(x, v) x = v,
#define TOK_EXPR_OP(x, v) x = v,
#define TOK_COMPOUND(x, v) x = hash_string<const char*, std::int_fast16_t>(v),

#include "tokens.def"
};

std::string to_string(TokenKind tok);

struct Token
{
  Token() = default;
  Token(SourceRange range, TokenKind kind)
    : range(range), kind(kind)
  {  }
  Token(SourceRange range, TokenKind kind, void* data)
    : range(range), kind(kind), data(data)
  {  }

  TokenKind tok_kind() const
  { return kind; }

  const char* data_as_text() const
  { return reinterpret_cast<const char*>(data); }

  const SourceRange& loc() const
  { return range; }

  explicit operator std::string() const;

  friend std::ostream& operator<<(std::ostream& os, const Token& tok);
private:
  SourceRange range;
  TokenKind kind;
  void* data;

};

class Tokenizer
{
public:
  Tokenizer(const char* module, std::istream& handle);

  Token get();
  void reset();

  const std::string& module_name() const;
private:
  char read(); 
  bool accept(std::string substr);
private:
  const std::string module;
  std::istream& handle;
  std::string linebuf;
  std::size_t row;
  std::size_t col; 

  tsl::hopscotch_map<std::uint_fast32_t, std::vector<std::uint_fast8_t>> token_data_table;
};

