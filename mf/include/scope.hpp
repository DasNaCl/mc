#pragma once

#include <tsl/hopscotch_map.h>
#include <util.hpp>

#include <memory>

class Type;

class Scope : public std::enable_shared_from_this<Scope>
{
public:
  using Ptr = std::shared_ptr<Scope>;

  Scope::Ptr parent();
  void make_parent(Scope& of);
  bool add(Symbol symb, std::shared_ptr<Type> typ);
  std::shared_ptr<Type> lookup(Symbol symb);
private:
  Scope::Ptr _parent;
  SymbolMap<std::shared_ptr<Type>> vars;
};

