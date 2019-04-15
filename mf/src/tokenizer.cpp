#include <tokenizer.hpp>

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
      if(('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'))
      {
        std::uint_fast32_t hash = ch;
        std::string name;
        name.push_back(ch);
        while(col < linebuf.size())
        {
          ch = linebuf[col++];
          if(!(('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ch == '_'))
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
      }
      else if('0' <= ch && ch <= '9')
      {
        while(col < linebuf.size())
        {
          ch = linebuf[col++];
          if(!('0' <= ch && ch <= '9'))
          {
            col--;
            break;
          }
        }
        kind = TokenKind::Number;
      }
      else
        return Token(SourceRange(module.c_str(), beg_col + 1, beg_row, col + 1, row), TokenKind::Undef);
    break;
  case '\"':
      while(ch != '\"')
      {
        ch = linebuf[col++];
        if(col >= linebuf.size())
        {
          return Token(SourceRange(module.c_str(), beg_col + 1, beg_row, col, row), TokenKind::Undef);
        }
      }
      kind = TokenKind::String;
    break;
  case '\'':
      ch = linebuf[col++];
      if(col + 1 >= linebuf.size() || linebuf[col + 1] != '\'')
      {
        return Token(SourceRange(module.c_str(), beg_col + 1, beg_row, col + 1, row), TokenKind::Undef);
      }
      kind = TokenKind::Character;
    break;

  case '+':
  case '-':
  case '*':
  case '/':
  case '(':
  case ')':
  case '{':
  case '}':
  case '[':
  case ']':
  case '=':
  case ';':
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


Token::operator std::string() const
{
  switch(kind)
  {
  default:
  case TokenKind::Undef: return "Undef";

  case TokenKind::EndOfFile: return "EndOfFile";

  case TokenKind::Id: return std::string("Id(") + std::string(reinterpret_cast<char*>(data)) + ")";

  case TokenKind::Number: return "Number";
  case TokenKind::String: return "String";
  case TokenKind::Character: return "Character";

  case TokenKind::Semicolon: return "Semicolon";

  case TokenKind::LParen: return "LParen";
  case TokenKind::RParen: return "RParen";
  case TokenKind::LBrace: return "LBrace";
  case TokenKind::RBrace: return "RBrace";
  case TokenKind::LBracket: return "RBracket";
  case TokenKind::RBracket: return "RBracket";

  case TokenKind::Equal: return "Equal";
  case TokenKind::Plus: return "Plus";
  case TokenKind::Minus: return "Minus";
  case TokenKind::Star: return "Star";
  case TokenKind::Slash: return "Slash";
  }
}

std::ostream& operator<<(std::ostream& os, const Token& tok)
{
  os << tok.range << " Token: " << static_cast<std::string>(tok);
  return os;
}

