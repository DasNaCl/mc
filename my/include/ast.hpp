#pragma once

#include <source_range.hpp>
#include <tokenizer.hpp>
#include <symbol.hpp>
#include <variant>
#include <memory>

#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>

struct GIDTag
{
  std::uint_fast64_t gid() const;
private:
  static std::uint_fast64_t gid_counter;
  std::uint_fast64_t gid_val { gid_counter++ };
};
struct GIDTagHasher
{
  std::size_t operator()(GIDTag* gidtag) const
  { if(gidtag) return gidtag->gid(); return 0; } // gid is unique
};
struct GIDTagComparer
{
  bool operator()(GIDTag* lhs, GIDTag* rhs) const
  { if(lhs && rhs) return lhs->gid() == rhs->gid(); return false; }
};

template<class T, 
         unsigned int NeighborhoodSize = 62,
         bool StoreHash = false,
         class GrowthPolicy = tsl::hh::power_of_two_growth_policy<2>>
using ASTNodeMap = tsl::hopscotch_map<GIDTag*, T, GIDTagHasher, GIDTagComparer, std::allocator<std::pair<GIDTag*, T>>,
                                      NeighborhoodSize, StoreHash, GrowthPolicy>;


class Statement : public GIDTag, public std::enable_shared_from_this<Statement>
{
public:
  using Ptr = std::shared_ptr<Statement>;

  Statement(SourceRange loc);

  virtual Statement::Ptr clone() = 0;
  virtual void print(std::ostream& os) = 0;

  SourceRange source_range() const;
private:
  SourceRange loc;
};

class Expression : public GIDTag, public std::enable_shared_from_this<Expression>
{
public:
  using Ptr = std::shared_ptr<Expression>;

  Expression(SourceRange loc);

  virtual Expression::Ptr clone() = 0;
  virtual void print(std::ostream& os) = 0;

  SourceRange source_range();
private:
  SourceRange loc;
};

class Identifier;

class ErrorStatement : public Statement
{
public:
  using Ptr = std::shared_ptr<ErrorStatement>;

  ErrorStatement(SourceRange loc);

  Statement::Ptr clone() override;
  void print(std::ostream& os) override;
};

class Definition : public Statement
{
public:
  using Ptr = std::shared_ptr<Definition>;

  Definition(SourceRange loc, Expression::Ptr id, Expression::Ptr body);

  Statement::Ptr clone() override;
  void print(std::ostream& os) override;

  std::shared_ptr<Identifier> identifier() const;
private:
  Expression::Ptr id;
  Expression::Ptr body;
};

class ErrorExpression : public Expression
{
public:
  using Ptr = std::shared_ptr<ErrorExpression>;

  ErrorExpression(SourceRange loc);

  Expression::Ptr clone() override;
  void print(std::ostream& os) override;
};

class Identifier : public Expression
{
public:
  using Ptr = std::shared_ptr<Identifier>;

  Identifier(SourceRange loc, Symbol symbol);

  Expression::Ptr clone() override;
  void print(std::ostream& os) override;

  Symbol id() const;
private:
  Symbol symbol;
};

class FunctionCall : public Expression
{
public:
  using Ptr = std::shared_ptr<FunctionCall>;

  FunctionCall(SourceRange range, Expression::Ptr fn, Expression::Ptr arg);

  Expression::Ptr clone() override;
  void print(std::ostream& os) override;

  bool is_simple() const;
private:
  Expression::Ptr fn;
  Expression::Ptr arg;
};

class Lambda : public Expression
{
public:
  using Ptr = std::shared_ptr<Lambda>;

  Lambda(SourceRange loc, Identifier::Ptr binding, Expression::Ptr body);
  void print(std::ostream& os) override;

  Expression::Ptr clone() override;
private:
  Identifier::Ptr binding;
  Expression::Ptr body;
};

