#include "Setup.hpp"
#include <cstdint>
#include <iostream>
#include <sstream>
#include <memory>
#include <limits>
#include <boost/format.hpp>
#include <boost/date_time/time_duration.hpp>
#include <gmpxx.h>
#include "text/SyntaxException.hpp"

static IValueToken* numberConverter(const std::string& value) { return new DefaultValueType(std::stod(value) != 0.0); }

static void addUnaryOperator(const UnaryOperatorToken::CallbackType& callback,
                             char identifier,
                             int precedence,
                             Associativity associativity,
                             const std::string& title       = "",
                             const std::string& description = "")
{
  auto tmpNew                           = std::make_unique<UnaryOperatorToken>(callback, identifier, precedence, associativity);
  auto tmp                              = tmpNew.get();
  defaultUnaryOperatorCache[identifier] = std::move(tmpNew);
  defaultUnaryOperators[identifier]     = tmp;

  defaultUnaryOperatorInfoMap.push_back(std::make_tuple(tmp, title, description));
}

static void addBinaryOperator(const BinaryOperatorToken::CallbackType& callback,
                              const std::string& identifier,
                              int precedence,
                              Associativity associativity,
                              const std::string& title       = "",
                              const std::string& description = "")
{
  auto tmpNew                            = std::make_unique<BinaryOperatorToken>(callback, identifier, precedence, associativity);
  auto tmp                               = tmpNew.get();
  defaultBinaryOperatorCache[identifier] = std::move(tmpNew);
  defaultBinaryOperators[identifier]     = tmp;

  defaultBinaryOperatorInfoMap.push_back(std::make_tuple(tmp, title, description));
}

static void addFunction(const FunctionToken::CallbackType& callback,
                        const std::string& identifier,
                        std::size_t minArgs            = 0u,
                        std::size_t maxArgs            = FunctionToken::GetArgumentCountMaxLimit(),
                        const std::string& title       = "",
                        const std::string& description = "")
{
  auto tmpNew                      = std::make_unique<FunctionToken>(callback, identifier, minArgs, maxArgs);
  auto tmp                         = tmpNew.get();
  defaultFunctionCache[identifier] = std::move(tmpNew);
  defaultFunctions[identifier]     = tmp;

  defaultFunctionInfoMap.push_back(std::make_tuple(tmp, title, description));
}

template<class T>
static void addVariable(const T& value, const std::string& identifier, const std::string& title = "", const std::string& description = "")
{
  auto tmpNew                                 = std::make_unique<DefaultVariableType>(identifier, value);
  auto tmp                                    = tmpNew.get();
  defaultInitializedVariableCache[identifier] = std::move(tmpNew);
  defaultVariables[identifier]                = tmp;

  defaultVariableInfoMap.push_back(std::make_tuple(tmp, title, description));
}

static DefaultValueType* addNewVariable(const std::string& identifier)
{
  auto tmpNew = std::make_unique<DefaultVariableType>(identifier);
  auto result = tmpNew.get();
  defaultUninitializedVariableCache.push_back(std::move(tmpNew));
  defaultVariables[identifier] = result;
  return result;
}

#ifndef __REGION__UNOPS
#ifndef __REGION__UNOPS__BITWISE
static IValueToken* UnaryOperator_Not(IValueToken* rhs) { return new DefaultValueType(!rhs->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>()); }
#endif // __REGION__UNOPS__BITWISE
#endif // __REGION__UNOPS

#ifndef __REGION__BINOPS
#ifndef __REGION__BINOPS__COMPARISON
static IValueToken* BinaryOperator_Equals(IValueToken* lhs, IValueToken* rhs)
{
  return new DefaultValueType(DefaultArithmeticType(lhs->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>() ==
                                                    rhs->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* BinaryOperator_NotEquals(IValueToken* lhs, IValueToken* rhs)
{
  return new DefaultValueType(DefaultArithmeticType(lhs->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>() !=
                                                    rhs->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>()));
}
#endif // __REGION__BINOPS__COMPARISON

#ifndef __REGION__BINOPS__BITWISE
static IValueToken* BinaryOperator_BitwiseOr(IValueToken* lhs, IValueToken* rhs)
{
  return new DefaultValueType(DefaultArithmeticType(lhs->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>() |
                                                    rhs->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* BinaryOperator_BitwiseAnd(IValueToken* lhs, IValueToken* rhs)
{
  return new DefaultValueType(DefaultArithmeticType(lhs->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>() &
                                                    rhs->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* BinaryOperator_BitwiseXor(IValueToken* lhs, IValueToken* rhs)
{
  return new DefaultValueType(DefaultArithmeticType(lhs->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>() ^
                                                    rhs->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>()));
}
#endif // __REGION__BINOPS__BITWISE
#endif // __REGION__BINOPS

#ifndef __REGION__FUNCTIONS
#ifndef __REGION__FUNCTIONS__BITWISE
static IValueToken* Function_Not(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(!args[0]->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>());
}

static IValueToken* Function_Or(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(args[0]->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>() |
                              args[1]->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>());
}

static IValueToken* Function_And(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(args[0]->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>() &
                              args[1]->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>());
}

static IValueToken* Function_Xor(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(args[0]->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>() &
                              args[1]->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>());
}

static IValueToken* Function_Nor(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(
      !(args[0]->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>() | args[1]->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Nand(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(
      !(args[0]->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>() & args[1]->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Xnor(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(
      !(args[0]->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>() ^ args[1]->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>()));
}
#endif // __REGION__FUNCTIONS__BITWISE
#endif // __REGION__FUNCTIONS

static std::unique_ptr<BinaryOperatorToken> juxtapositionOperator;

void InitTruthTable(TruthTableExpressionParser& instance)
{
  if(options.jpo_precedence != 0)
  {
    juxtapositionOperator = std::make_unique<BinaryOperatorToken>(BinaryOperator_BitwiseAnd, "&", 2 + options.jpo_precedence, Associativity::Left);
  }

  instance.SetOnParseNumberCallback(numberConverter);
  instance.SetOnUnknownIdentifierCallback(addNewVariable);
  instance.SetJuxtapositionOperator(juxtapositionOperator.get());

  instance.SetUnaryOperators(&defaultUnaryOperators);
  instance.SetBinaryOperators(&defaultBinaryOperators);
  instance.SetFunctions(&defaultFunctions);
  instance.SetVariables(&defaultVariables);

  addUnaryOperator(UnaryOperator_Not, '!', 5, Associativity::Right, "Not", "!x");
  addUnaryOperator(UnaryOperator_Not, '~', 5, Associativity::Right, "Not", "~x");

  addBinaryOperator(BinaryOperator_Equals, "==", 4, Associativity::Left, "Equals", "x == y");
  addBinaryOperator(BinaryOperator_NotEquals, "!=", 4, Associativity::Left, "Not equals", "x != y");
  defaultBinaryOperatorInfoMap.push_back(std::make_tuple(nullptr, "", ""));
  addBinaryOperator(BinaryOperator_BitwiseOr, "|", 2, Associativity::Left, "Bitwise OR", "x | y");
  addBinaryOperator(BinaryOperator_BitwiseAnd, "&", 2, Associativity::Left, "Bitwise AND", "x & y");
  addBinaryOperator(BinaryOperator_BitwiseXor, "^", 2, Associativity::Left, "Bitwise XOR", "x ^ y");
  defaultBinaryOperatorInfoMap.push_back(std::make_tuple(nullptr, "", ""));
  addBinaryOperator(BinaryOperator_BitwiseOr, "+", 2, Associativity::Left, "Bitwise OR", "x + y");
  addBinaryOperator(BinaryOperator_BitwiseAnd, "*", 2, Associativity::Left, "Bitwise AND", "x * y");
  addBinaryOperator(BinaryOperator_BitwiseXor, "/", 2, Associativity::Left, "Bitwise XOR", "x / y");

  addFunction(Function_Not, "NOT", 1u, 1u, "Not", "NOT x");
  defaultFunctionInfoMap.push_back(std::make_tuple(nullptr, "", ""));
  addFunction(Function_Or, "OR", 2u, 2u, "Or", "x OR y");
  addFunction(Function_And, "AND", 2u, 2u, "Or", "x AND y");
  addFunction(Function_Xor, "XOR", 2u, 2u, "Or", "x XOR y");
  defaultFunctionInfoMap.push_back(std::make_tuple(nullptr, "", ""));
  addFunction(Function_Nor, "NOR", 2u, 2u, "Or", "x NOR y");
  addFunction(Function_Nand, "NAND", 2u, 2u, "Or", "x NAND y");
  addFunction(Function_Xnor, "XNOR", 2u, 2u, "Or", "x XNOR y");

  addVariable(true, "true", "True", "Boolean value");
  addVariable(true, "T", "True", "Boolean value");
  addVariable(false, "false", "False", "Boolean value");
  addVariable(false, "F", "False", "Boolean value");
  addVariable(true, "high", "High", "Boolean value");
  addVariable(true, "H", "High", "Boolean value");
  addVariable(false, "low", "Low", "Boolean value");
  addVariable(false, "L", "Low", "Boolean value");
}
