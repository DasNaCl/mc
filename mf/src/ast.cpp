#include <ast.hpp>

std::uint_fast64_t GIDTag::gid_counter = 0;

std::uint_fast64_t GIDTag::gid() const
{ return gid_val; }


Statement::Statement(SourceRange loc)
  : loc(loc)
{  }

Expression::Expression(SourceRange loc)
  : loc(loc)
{  }

SourceRange Expression::source_range()
{ return loc; }

PrimitiveType::PrimitiveType(Symbol name)
  : Type(), name(name)
{  }

FunctionType::FunctionType(Type::Ptr arg_typ, Type::Ptr ret_typ)
  : Type(), arg_typ(arg_typ), ret_typ(ret_typ)
{  }

TupleType::TupleType(const std::vector<Type::Ptr>& types)
  : Type(), types(types)
{  }

ArgsType::ArgsType(const std::vector<Type::Ptr>& types)
  : Type(), types(types)
{  }


Identifier::Identifier(SourceRange loc, Symbol symbol)
  : Statement(loc), symbol(symbol), typ(std::make_shared<TemplateType>()) // TODO: fix type
{  }

Symbol Identifier::id() const
{ return symbol; }

Type::Ptr Identifier::type()
{ return typ; }

Declaration::Declaration(SourceRange loc, Identifier::Ptr identifier, Type::Ptr type)
  : Statement(loc), identifier(identifier), typ(type)
{  }

Type::Ptr Declaration::type()
{ return typ; }

Parameter::Parameter(SourceRange range, Identifier::Ptr identifier, Type::Ptr type)
  : Declaration(range, identifier, type)
{  }

Type::Ptr Parameter::type()
{ return typ; }

Parameters::Parameters(SourceRange loc, const std::vector<Parameter::Ptr>& list)
  : Statement(loc), list(list)
{  }

Type::Ptr Parameters::type()
{
  std::vector<Type::Ptr> types;
  for(auto& p : list)
    types.emplace_back(p->type());
  return std::make_shared<TupleType>(types);
}

Block::Block(SourceRange loc, const std::vector<Statement::Ptr>& statements)
  : Statement(loc), statements(statements)
{  }

Type::Ptr Block::type()
{ return std::make_shared<Unit>(); }

Function::Function(SourceRange loc, const std::vector<Statement::Ptr>& data, Type::Ptr ret_typ)
  : Statement(loc), data(data), ret_typ(ret_typ)
{  }

Type::Ptr Function::type()
{
  std::vector<Type::Ptr> types;
  for(auto& stmt : data)
  {
    if(std::dynamic_pointer_cast<Parameters>(stmt))
      types.emplace_back(stmt->type());
  }
  return std::make_shared<FunctionType>(std::make_shared<ArgsType>(types), ret_typ);
}

ExpressionStatement::ExpressionStatement(Expression::Ptr expr)
  : Statement(expr->source_range()), expr(expr)
{  }

Type::Ptr ExpressionStatement::type()
{ return expr->type(); }

LiteralExpression::LiteralExpression(Token tok)
  : Expression(tok.range), kind(tok.kind), data(tok.data)
{  }

Type::Ptr LiteralExpression::type()
{
  switch(kind)
  {
  default: return std::make_shared<Unit>();

  case TokenKind::Number:     return std::make_shared<PrimitiveType>(Symbol("long"));
  case TokenKind::Character:  return std::make_shared<PrimitiveType>(Symbol("char"));
  case TokenKind::String:     assert(false && "Strings are currently unsupported"); return std::make_shared<Unit>(); // TODO
  }
}

BinaryExpression::BinaryExpression(Expression::Ptr left, Expression::Ptr right)
  : Expression(left->source_range() + right->source_range()), left(left), right(right)
{  }

Type::Ptr BinaryExpression::type()
{
  // TODO
  auto lt = left->type();
  auto rt = right->type();

  return lt;
}


void ASTVisitor::visit_all(const std::vector<Statement::Ptr>& ast)
{
  for(auto& stmt : ast)
  {
    stmt->visit(*this);
  }
}


void Statement::enter(ASTVisitor& visitor)
{
  visitor.enter(shared_from_this());
}
void Statement::visit(ASTVisitor& visitor)
{
  Statement::enter(visitor);
  visitor.visit(shared_from_this());
  Statement::leave(visitor);
}
void Statement::leave(ASTVisitor& visitor)
{
  visitor.leave(shared_from_this());
}

void Identifier::enter(ASTVisitor& visitor)
{
  Statement::enter(visitor);
  visitor.enter(std::static_pointer_cast<Identifier>(shared_from_this()));
}
void Identifier::visit(ASTVisitor& visitor)
{
  Identifier::enter(visitor);
  Statement::visit(visitor);
  visitor.visit(std::static_pointer_cast<Identifier>(shared_from_this()));
  Identifier::leave(visitor);
}
void Identifier::leave(ASTVisitor& visitor)
{
  visitor.leave(std::static_pointer_cast<Identifier>(shared_from_this()));
  Statement::leave(visitor);
}

void Declaration::enter(ASTVisitor& visitor)
{
  Statement::enter(visitor);
  visitor.enter(std::static_pointer_cast<Declaration>(shared_from_this()));
}
void Declaration::visit(ASTVisitor& visitor)
{
  Declaration::enter(visitor);
  Statement::visit(visitor);
  visitor.visit(std::static_pointer_cast<Declaration>(shared_from_this()));
  Declaration::leave(visitor);
}
void Declaration::leave(ASTVisitor& visitor)
{
  visitor.leave(std::static_pointer_cast<Declaration>(shared_from_this()));
  Statement::leave(visitor);
}

void Parameter::enter(ASTVisitor& visitor)
{
  Statement::enter(visitor);
  Declaration::enter(visitor);
  visitor.enter(std::static_pointer_cast<Parameter>(shared_from_this()));
}
void Parameter::visit(ASTVisitor& visitor)
{
  Parameter::enter(visitor);
  Statement::visit(visitor);
  Declaration::visit(visitor);
  visitor.visit(std::static_pointer_cast<Parameter>(shared_from_this()));
  Parameter::leave(visitor);
}
void Parameter::leave(ASTVisitor& visitor)
{
  visitor.leave(std::static_pointer_cast<Parameter>(shared_from_this()));
  Declaration::leave(visitor);
  Statement::leave(visitor);
}

void Parameters::enter(ASTVisitor& visitor)
{
  Statement::enter(visitor);
  visitor.enter(std::static_pointer_cast<Parameters>(shared_from_this()));
}
void Parameters::visit(ASTVisitor& visitor)
{
  Parameters::enter(visitor);
  Statement::visit(visitor);
  visitor.visit(std::static_pointer_cast<Parameters>(shared_from_this()));
  Parameters::leave(visitor);
}
void Parameters::leave(ASTVisitor& visitor)
{
  visitor.leave(std::static_pointer_cast<Parameters>(shared_from_this()));
  Statement::leave(visitor);
}

void Block::enter(ASTVisitor& visitor)
{
  Statement::enter(visitor);
  visitor.enter(std::static_pointer_cast<Block>(shared_from_this()));
}
void Block::visit(ASTVisitor& visitor)
{
  Block::enter(visitor);
  Statement::visit(visitor);

  visitor.visit(std::static_pointer_cast<Block>(shared_from_this()));
  for(auto& stmt : statements)
    stmt->visit(visitor);

  Block::leave(visitor);
}
void Block::leave(ASTVisitor& visitor)
{
  visitor.leave(std::static_pointer_cast<Block>(shared_from_this()));
  Statement::leave(visitor);
}

void Function::enter(ASTVisitor& visitor)
{
  Statement::enter(visitor);
  visitor.enter(std::static_pointer_cast<Function>(shared_from_this()));
}
void Function::visit(ASTVisitor& visitor)
{
  Function::enter(visitor);

  Statement::visit(visitor);
  visitor.visit(std::static_pointer_cast<Function>(shared_from_this()));

  for(auto& stmt : data)
    stmt->visit(visitor);

  Function::leave(visitor);
}
void Function::leave(ASTVisitor& visitor)
{
  visitor.leave(std::static_pointer_cast<Function>(shared_from_this()));
  Statement::leave(visitor);
}

void ExpressionStatement::enter(ASTVisitor& visitor)
{
  Statement::enter(visitor);
  visitor.enter(std::static_pointer_cast<ExpressionStatement>(shared_from_this()));
}
void ExpressionStatement::visit(ASTVisitor& visitor)
{
  ExpressionStatement::enter(visitor);
  Statement::visit(visitor);

  visitor.visit(std::static_pointer_cast<ExpressionStatement>(shared_from_this()));
  expr->visit(visitor);

  ExpressionStatement::leave(visitor);
}
void ExpressionStatement::leave(ASTVisitor& visitor)
{
  visitor.leave(std::static_pointer_cast<ExpressionStatement>(shared_from_this()));
  Statement::leave(visitor);
}


void Expression::enter(ASTVisitor& visitor)
{
  visitor.enter(shared_from_this());
}
void Expression::visit(ASTVisitor& visitor)
{
  Expression::enter(visitor);
  visitor.visit(shared_from_this());
  Expression::leave(visitor);
}
void Expression::leave(ASTVisitor& visitor)
{
  visitor.leave(shared_from_this());
}

void LiteralExpression::enter(ASTVisitor& visitor)
{
  Expression::enter(visitor);
  visitor.enter(std::static_pointer_cast<LiteralExpression>(shared_from_this()));
}
void LiteralExpression::visit(ASTVisitor& visitor)
{
  LiteralExpression::enter(visitor);
  Expression::visit(visitor);
  visitor.visit(std::static_pointer_cast<LiteralExpression>(shared_from_this()));
  LiteralExpression::leave(visitor);
}
void LiteralExpression::leave(ASTVisitor& visitor)
{
  visitor.leave(std::static_pointer_cast<LiteralExpression>(shared_from_this()));
  Expression::leave(visitor);
}

void BinaryExpression::enter(ASTVisitor& visitor)
{
  Expression::enter(visitor);
  visitor.enter(std::static_pointer_cast<BinaryExpression>(shared_from_this()));
}
void BinaryExpression::visit(ASTVisitor& visitor)
{
  BinaryExpression::enter(visitor);
  Expression::visit(visitor);
  visitor.visit(std::static_pointer_cast<BinaryExpression>(shared_from_this()));

  left->visit(visitor);
  right->visit(visitor);
  BinaryExpression::leave(visitor);
}
void BinaryExpression::leave(ASTVisitor& visitor)
{
  visitor.leave(std::static_pointer_cast<BinaryExpression>(shared_from_this()));
  Expression::leave(visitor);
}


