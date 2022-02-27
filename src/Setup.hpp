#ifndef __TRTBLSETUP_HPP__
#define __TRTBLSETUP_HPP__

#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <tuple>
#include "text/expression/ExpressionParserBase.hpp"

using DefaultArithmeticType = bool;
using DefaultValueType      = ValueToken<std::nullptr_t, DefaultArithmeticType>;
using DefaultVariableType   = VariableToken<std::nullptr_t, DefaultArithmeticType>;

inline std::unordered_map<char, std::unique_ptr<UnaryOperatorToken>> defaultUnaryOperatorCache;
inline std::unordered_map<char, IUnaryOperatorToken*> defaultUnaryOperators;

inline std::unordered_map<std::string, std::unique_ptr<BinaryOperatorToken>> defaultBinaryOperatorCache;
inline std::unordered_map<std::string, IBinaryOperatorToken*> defaultBinaryOperators;

inline std::unordered_map<std::string, std::unique_ptr<FunctionToken>> defaultFunctionCache;
inline std::unordered_map<std::string, IFunctionToken*> defaultFunctions;

inline std::list<std::unique_ptr<DefaultVariableType>> defaultUninitializedVariableCache;
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
  int jpo_precedence;
};

const inline trtbl_options defaultOptions {"1", "0", ' ', '=', 1u, 1u, 1u, 1u, -1};
inline trtbl_options options {};

void InitTruthTable(ExpressionParserBase& instance);

#endif // __TRTBLSETUP_HPP__
