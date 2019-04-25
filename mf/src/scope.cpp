#include <scope.hpp>
#include <ast.hpp>


Scope::Ptr Scope::parent()
{
  return _parent;
}

void Scope::make_parent(Scope& of)
{
  of._parent = shared_from_this();
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

  if(_parent)
    return _parent->lookup(symb);

  return nullptr;
}

