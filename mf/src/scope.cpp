#include <scope.hpp>
#include <ast.hpp>


void Scope::make_parent(Scope& of)
{
  of.parent = shared_from_this();
}

bool Scope::add(Symbol symb, Type::Ptr typ)
{
  if(vars.find(symb) != vars.end())
    return false;

  vars[symb] = typ;

  return true;
}

Type::Ptr Scope::lookup(Symbol symb)
{
  if(auto it = vars.find(symb); it != vars.end())
    return it->second;

  if(parent)
    return parent->lookup(symb);

  return nullptr;
}

