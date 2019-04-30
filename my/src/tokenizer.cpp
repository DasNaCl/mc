#include <tokenizer.hpp>

constexpr static const char lambda_blob[] = "λ";

Tokenizer::Tokenizer(const char* module, std::istream& handle)
  : module(module), handle(handle), linebuf(), row(1), col(0), token_data_table()
{  }

const std::string& Tokenizer::module_name() const
{ return module; }

Token Tokenizer::get()
{
  void* data = nullptr;
  TokenKind kind = TokenKind::Undef;

  char ch = read();
  std::size_t beg_row = row;
  std::size_t beg_col = col - 1;

  switch(ch)
  {
  default:
    {
      // Optimistically allow any kind of identifier to allow for unicode
      std::uint_fast32_t hash = ch;
      std::string name;
      name.push_back(ch);
      while(col < linebuf.size())
      {
        ch = linebuf[col++];

        // break if we hit whitespace (or other control chars) or any other token char
        if(std::iscntrl(ch) || std::isspace(ch)
#define TOK_CONTROL(x, v) || ch == v
#define TOK_COMPOUND(x, v) || ch == v[0]
#define TOK_EXPR_OP(x, v) || ch == v
#include <tokens.def>
            )
        {
          col--;
          break;
        }
        name.push_back(ch);
        hash ^= (hash * 31) + ch;
      }
      kind = TokenKind::Id;
      auto& v = token_data_table[hash];
      v.resize(name.size() + 1, 0);
      std::copy(name.begin(), name.end(), v.begin());
      data = &v[0];
    } break;

  case lambda_blob[0]:
    if(accept(std::string(std::begin(lambda_blob) + 1, std::end(lambda_blob) - 1)))
    {
      kind = TokenKind::Lambda;
      break;
    }
  case '(':
  case ')':
  case '=':
  case ';':
  case '.':
      kind = static_cast<TokenKind>(ch);
    break;
  case EOF:
      kind = TokenKind::EndOfFile;
    break;
  }
  return Token(SourceRange(module.c_str(), beg_col + 1, beg_row, col + 1, row), kind, data);
}

char Tokenizer::read()
{
  char ch = 0;
  bool skipped_line = false;
  do
  {
    if(col >= linebuf.size())
    {
      if(!linebuf.empty())
        row++;
      if(!(std::getline(handle, linebuf)))
      {
        col = 1;
        linebuf = "";
        return EOF;
      }
      else
      {
        while(linebuf.empty())
        {
          row++;
          if(!(std::getline(handle, linebuf)))
          {
            col = 1;
            linebuf = "";
            return EOF;
          }
        }
      }
      col = 0;
    }
    ch = linebuf[col++];
    if(std::isspace(ch))
    {
      skipped_line = true;
      while(col < linebuf.size())
      {
        ch = linebuf[col++];
        if(!(std::isspace(ch)))
        {
          skipped_line = false;
          break;
        }
      }
    }
  } while(skipped_line);

  return ch;
}

bool Tokenizer::accept(std::string substr)
{
  if(col + substr.size() < linebuf.size())
  {
    for(std::size_t c = col; c < col + substr.size(); ++c)
      if(linebuf[c] != substr[c - col])
        return false;
    col += substr.size();
    return true;
  }
  return false;
}

std::string to_string(TokenKind kind)
{
  switch(kind)
  {
  default:
  case TokenKind::Undef: return "Undef";

  case TokenKind::EndOfFile: return "EndOfFile";

  case TokenKind::Semicolon: return "Semicolon";

  case TokenKind::Dot: return "Dot";

  case TokenKind::Id: return "Id";
  case TokenKind::Equal: return "Equal";

  case TokenKind::LParen: return "LParen";
  case TokenKind::RParen: return "RParen";

  case TokenKind::Lambda: return "λ";
  }
}

void Tokenizer::reset()
{
  handle.clear();
  handle.seekg(0, std::ios::beg);
}

Token::operator std::string() const
{
  switch(kind)
  {
  default: return to_string(kind);

  case TokenKind::Id: return to_string(kind) + "(" + this->data_as_text() + ")";
  }
}

std::ostream& operator<<(std::ostream& os, const Token& tok)
{
  os << tok.range << " Token: " << static_cast<std::string>(tok);
  return os;
}

