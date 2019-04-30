#include <ast.hpp>
#include <set>

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

Statement::Ptr ErrorStatement::reduce_step_normal()
{ return shared_from_this(); }

Statement::Ptr ErrorStatement::reduce_step_callbyname()
{ return shared_from_this(); }

Statement::Ptr ErrorStatement::reduce_step_callbyvalue()
{ return shared_from_this(); }

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

void Identifier::fv(SymbolSet& cur)
{ cur.insert(symbol); }

Expression::Ptr Identifier::reduce_step_normal()
{ return shared_from_this(); }

Expression::Ptr Identifier::reduce_step_callbyname()
{ return shared_from_this(); }

Expression::Ptr Identifier::reduce_step_callbyvalue()
{ return shared_from_this(); }

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

Statement::Ptr Definition::reduce_step_normal()
{ body->reduce_step_normal(); return shared_from_this(); }

Statement::Ptr Definition::reduce_step_callbyname()
{ body->reduce_step_callbyname(); return shared_from_this(); }

Statement::Ptr Definition::reduce_step_callbyvalue()
{ body->reduce_step_callbyvalue(); return shared_from_this(); }

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

Expression::Ptr ErrorExpression::reduce_step_normal()
{ return shared_from_this(); }

Expression::Ptr ErrorExpression::reduce_step_callbyname()
{ return shared_from_this(); }

Expression::Ptr ErrorExpression::reduce_step_callbyvalue()
{ return shared_from_this(); }

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

void FunctionCall::fv(SymbolSet& cur)
{
  fn->fv(cur);
  arg->fv(cur);
}

void FunctionCall::replace(Identifier::Ptr what, Expression::Ptr with)
{
  auto fn_is_var = std::dynamic_pointer_cast<Identifier>(fn);
  if(fn_is_var)
  {
    if(fn_is_var->id() == what->id())
    {
      fn = with->clone();
    }
  }
  else
    fn->replace(what, with);

  auto arg_is_var = std::dynamic_pointer_cast<Identifier>(arg);
  if(arg_is_var)
  {
    if(arg_is_var->id() == what->id())
    {
      arg = with->clone();
    }
  }
  else
    arg->replace(what, with);
}

Expression::Ptr FunctionCall::reduce_step_normal()
{
  if(auto is_fn = std::dynamic_pointer_cast<Lambda>(fn))
  {
    is_fn->replace(arg);
    return is_fn->fn_body();
  }
  else
  {
    fn = fn->reduce_step_normal();
    return shared_from_this();
  }
}

Expression::Ptr FunctionCall::reduce_step_callbyname()
{ return reduce_step_normal(); }

Expression::Ptr FunctionCall::reduce_step_callbyvalue()
{
  auto is_fn = std::dynamic_pointer_cast<Lambda>(arg);
  if(is_fn)
  {   
    // arg is fully evaluated, so we may β-reduce
    return reduce_step_normal();
  }   
  // we first need to fully reduce our argument
  auto new_arg = arg->reduce_step_callbyvalue();
  if(arg == new_arg)
  {   
    is_fn = std::dynamic_pointer_cast<Lambda>(fn);
    if(is_fn)
    {   
      is_fn->replace(arg);
      return is_fn->fn_body();
    }   
    else
    {
      // for stuff like ((λ x. λ y. y x) a b)
      fn = fn->reduce_step_normal();
    }
  }
  return shared_from_this();
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

Expression::Ptr Lambda::fn_body() const
{ return body; }

void Lambda::replace(Expression::Ptr what)
{
  auto is_var = std::dynamic_pointer_cast<Identifier>(body);
  if(is_var)
  {
    if(is_var->id() == binding->id())
      body = what->clone();
    else
      ; // we have something like `λ x.y`
  }
  else
  {
    body->replace(binding, what);
  }
}

void Lambda::fv(SymbolSet& cur)
{
  body->fv(cur);
  if(auto it = cur.find(binding->id()); it != cur.end())
    cur.erase(it);
}

void Lambda::replace(Identifier::Ptr what, Expression::Ptr with)
{
  // only replace if there is no rebinding
  if(what->id() != binding->id())
  {
    SymbolSet FV; with->fv(FV); 
    while(FV.find(binding->id()) != FV.end())
    { 
      // our binding occurs as free variable in what, so we change it!
      auto new_binding = std::make_shared<Identifier>(binding->source_range(), (binding->id().get_string() + "\'").c_str());
      body->replace(binding, new_binding);
      binding = new_binding;
    }
    auto is_var = std::dynamic_pointer_cast<Identifier>(body);
    if(!is_var)
      body->replace(what, with);
    else
    { 
      if(is_var->id() == what->id())
        body = with;
    }
  }
}

Expression::Ptr Lambda::reduce_step_normal()
{
  body = body->reduce_step_normal();
  return shared_from_this();
}

Expression::Ptr Lambda::reduce_step_callbyname()
{ return shared_from_this(); }

Expression::Ptr Lambda::reduce_step_callbyvalue()
{ return shared_from_this(); }

