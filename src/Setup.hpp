#ifndef __SETUP_HPP__
#define __SETUP_HPP__

#include "text/expression/ExpressionParserBase.hpp"

#include <list>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using DefaultArithmeticType                 = bool;
using DefaultValueType                      = Text::Expression::ValueToken<DefaultArithmeticType>;
using DefaultVariableType                   = Text::Expression::VariableToken<DefaultArithmeticType>;
using DefaultUninitializedVariableCacheType = std::list<std::unique_ptr<DefaultVariableType>>;

using Text::Expression::IValueToken;
using Text::Expression::IVariableToken;
using Text::Expression::IUnaryOperatorToken;
using Text::Expression::IBinaryOperatorToken;
using Text::Expression::IFunctionToken;
using Text::Expression::UnaryOperatorToken;
using Text::Expression::BinaryOperatorToken;
using Text::Expression::FunctionToken;

using Text::Expression::Associativity;

using Text::Expression::ExpressionParserBase;

inline std::unordered_map<char, std::unique_ptr<UnaryOperatorToken>> defaultUnaryOperatorCache;
inline std::unordered_map<char, IUnaryOperatorToken*> defaultUnaryOperators;

inline std::unordered_map<std::string, std::unique_ptr<BinaryOperatorToken>> defaultBinaryOperatorCache;
inline std::unordered_map<std::string, IBinaryOperatorToken*> defaultBinaryOperators;

inline std::unordered_map<std::string, std::unique_ptr<FunctionToken>> defaultFunctionCache;
inline std::unordered_map<std::string, IFunctionToken*> defaultFunctions;

inline DefaultUninitializedVariableCacheType defaultUninitializedVariableCache;
inline std::unordered_map<std::string, std::unique_ptr<DefaultVariableType>> defaultInitializedVariableCache;
inline std::unordered_map<std::string, IVariableToken*> defaultVariables;

inline std::vector<std::tuple<const IUnaryOperatorToken*, std::string, std::string>> defaultUnaryOperatorInfoMap;
inline std::vector<std::tuple<const IBinaryOperatorToken*, std::string, std::string>> defaultBinaryOperatorInfoMap;
inline std::vector<std::tuple<const IFunctionToken*, std::string, std::string>> defaultFunctionInfoMap;
inline std::vector<std::tuple<const IVariableToken*, std::string, std::string>> defaultVariableInfoMap;

struct trtbl_options
{
  std::string tsub;
  std::string fsub;
  char isep;
  char osep;
  std::size_t ipad_a;
  std::size_t ipad_b;
  std::size_t opad_a;
  std::size_t opad_b;
  bool sort;
  int jpo_precedence;
};

const inline trtbl_options defaultOptions {"1", "0", ' ', '=', 1u, 1u, 4u, 1u, false, -1};
inline trtbl_options options {};

void InitTruthTable(ExpressionParserBase& instance);

#endif // __SETUP_HPP__
