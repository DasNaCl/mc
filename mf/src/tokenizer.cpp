#include <tokenizer.hpp>

Tokenizer::Tokenizer(std::istream& handle)
  : handle(handle), linebuf(), row(1), col(0)
{  }

Token Tokenizer::get()
{
  void* data = nullptr;
  TokenKind kind = TokenKind::Undef;
  char ch = read();
redo:
  switch(ch)
  {
  default:
      if(('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'))
      {
        while(col < linebuf.size())
        {
          ch = linebuf[col++];
          if(!(('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ch == '_'))
          {
            col--;
            break;
          }
        }
        kind = TokenKind::Id;
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
      else if(std::isspace(ch))
      {
        ch = linebuf[col++];
        goto redo;
      }
      else
        return { TokenKind::Undef };
    break;
  case '\"':
      while(ch != '\"')
      {
        ch = linebuf[col++];
        if(col >= linebuf.size())
        {
          return { TokenKind::Undef };
        }
      }
      kind = TokenKind::String;
    break;
  case '\'':
      ch = linebuf[col++];
      if(col + 1 >= linebuf.size() || linebuf[col + 1] != '\'')
      {
        return { TokenKind::Undef };
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
  return Token(kind, data);
}

char Tokenizer::read()
{
  if(col >= linebuf.size())
  {
    if(!(std::getline(handle, linebuf)))
      return EOF;
    col = 0;
    row++;
  }
  return linebuf[col++];
}


Token::operator std::string() const
{
  switch(kind)
  {
  default:
  case TokenKind::Undef: return "Undef";

  case TokenKind::EndOfFile: return "EndOfFile";

  case TokenKind::Id: return "Id";

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

