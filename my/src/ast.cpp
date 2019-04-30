#include <ast.hpp>
#include <numeric>

std::uint_fast64_t GIDTag::gid_counter = 0;

std::uint_fast64_t GIDTag::gid() const
{ return gid_val; }

Statement::Statement(SourceRange loc)
  : loc(loc)
{  }

SourceRange Statement::source_range() const
{ return loc; }

Expression::Expression(SourceRange loc)
  : loc(loc)
{  }

SourceRange Expression::source_range()
{ return loc; }

ErrorStatement::ErrorStatement(SourceRange loc)
  : Statement(loc)
{  }

Statement::Ptr ErrorStatement::clone()
{
  return std::make_shared<ErrorStatement>(source_range());
}

void ErrorStatement::print(std::ostream& os)
{
  os << "?ERR?";
}

Identifier::Identifier(SourceRange loc, Symbol symbol)
  : Expression(loc), symbol(symbol)
{  }

Expression::Ptr Identifier::clone()
{
  return std::make_shared<Identifier>(source_range(), symbol);
}

void Identifier::print(std::ostream& os)
{
  os << symbol;
}

Symbol Identifier::id() const
{ return symbol; }

Definition::Definition(SourceRange loc, Expression::Ptr id, Expression::Ptr body)
  : Statement(loc), id(id), body(body)
{  }

Statement::Ptr Definition::clone()
{
  return std::make_shared<Definition>(source_range(), id->clone(), body->clone());
}

void Definition::print(std::ostream& os)
{
  if(id)
  {
    id->print(os);
    os << " ";
  }
  os << "= ";
  body->print(os);
}

Identifier::Ptr Definition::identifier() const
{
  if(auto is_id = std::dynamic_pointer_cast<Identifier>(id))
    return is_id;
  return nullptr;
}

ErrorExpression::ErrorExpression(SourceRange loc)
  : Expression(loc)
{  }

Expression::Ptr ErrorExpression::clone()
{
  return std::make_shared<ErrorExpression>(source_range());
}

void ErrorExpression::print(std::ostream& os)
{
  os << "?ERR?";
}

FunctionCall::FunctionCall(SourceRange range, Expression::Ptr fn, Expression::Ptr arg)
  : Expression(range), fn(fn), arg(arg)
{  }

Expression::Ptr FunctionCall::clone()
{
  return std::make_shared<FunctionCall>(source_range(), fn->clone(), arg->clone());
}

void FunctionCall::print(std::ostream& os)
{
  bool is_fn0 = !!std::dynamic_pointer_cast<Lambda>(fn);
  if(is_fn0)
    os << "(";
  os << "(";
  fn->print(os);
  if(is_fn0)
    os << ")";
  os << " ";
  bool is_fn = !!std::dynamic_pointer_cast<Lambda>(arg);
  if(is_fn)
    os << "(";
  arg->print(os);
  if(is_fn)
    os << ")";
  os << ")";
}

bool FunctionCall::is_simple() const
{
  const bool  fn_is_var = !!std::dynamic_pointer_cast<Identifier>(fn);
  const bool arg_is_var = !!std::dynamic_pointer_cast<Identifier>(arg);

  return fn_is_var && arg_is_var;
}

Lambda::Lambda(SourceRange loc, Identifier::Ptr binding, Expression::Ptr body)
  : Expression(loc), binding(binding), body(body)
{  }

Expression::Ptr Lambda::clone()
{
  return std::make_shared<Lambda>(source_range(), std::static_pointer_cast<Identifier>(binding->clone()), body->clone());
}

void Lambda::print(std::ostream& os)
{
  os << "λ ";
  binding->print(os);
  os << ". ";

  const bool is_fn = !!std::dynamic_pointer_cast<Lambda>(body);
  if(is_fn)
    os << "(";
  body->print(os);

  if(is_fn)
    os << ")";
}

