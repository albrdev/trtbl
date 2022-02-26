#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cerrno>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <memory>
#include <regex>
#include <locale>
#include <exception>
#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/date_time/date_facet.hpp>
#include <boost/date_time/time_facet.hpp>
#include <mpreal.h>
#include "text/SyntaxException.hpp"
#include "TruthTableExpressionParser.hpp"
#include "TrTblSetup.hpp"

static const std::unordered_map<Associativity, std::string> associativityNameMap = {
    {Associativity::Left, "Left"},
    {Associativity::Right, "Right"},
    {Associativity::Any, "Any"},
};

template<class T>
int sgn(T value)
{
  return (value > static_cast<T>(0)) - (value < static_cast<T>(0));
}

template<typename InputIterator, typename T>
bool cartesianProduct(InputIterator begin, InputIterator end, T min, T max)
{
  auto iter = std::find_if_not(std::make_reverse_iterator(end), std::make_reverse_iterator(begin), [&max](auto current) { return max == current; });

  if(iter == std::make_reverse_iterator(begin))
  {
    return false;
  }
  else
  {
    auto boundIter = std::prev(iter.base());
    (*boundIter)++;
    std::fill(std::next(boundIter), end, min);
    return true;
  }
}

static void assignInput(const std::list<unsigned int>& premutations)
{
  auto iter1 = defaultUninitializedVariableCache.begin();
  auto iter2 = premutations.begin();
  for(; iter1 != defaultUninitializedVariableCache.end(); iter1++, iter2++)
  {
    iter1->get()->AsPointer<DefaultVariableType>()->SetValue(static_cast<bool>(*iter2));
  }
}

static void clearVariableCache()
{
  while(!defaultUninitializedVariableCache.empty())
  {
    const auto& top = defaultUninitializedVariableCache.front();
    defaultVariables.erase(top.get()->GetIdentifier());
    defaultUninitializedVariableCache.pop_front();
  }
}

static void evaluate(const std::string& expression, TruthTableExpressionParser& expressionParser)
{
  auto queue = expressionParser.Parse(expression);
  std::list<unsigned int> premutations(defaultUninitializedVariableCache.size(), 0u);

  std::size_t varMaxLen = 0u;
  const auto last       = std::prev(defaultUninitializedVariableCache.cend());
  auto iter             = defaultUninitializedVariableCache.cbegin();
  for(; iter != last; iter++)
  {
    std::cout << iter->get()->GetIdentifier() << std::string(options.isep.length(), ' ');
    varMaxLen = std::max(varMaxLen, iter->get()->GetIdentifier().length());
  }
  std::cout << iter->get()->GetIdentifier() << std::endl;

  do
  {
    assignInput(premutations);
    auto result     = DefaultValueType(expressionParser.Evaluate(queue)->AsPointer<DefaultValueType>()->GetValue<DefaultArithmeticType>());
    const auto last = std::prev(premutations.cend());
    auto iter       = premutations.cbegin();
    for(; iter != last; iter++)
    {
      std::cout << ((*iter != 0u) ? options.tsub : options.fsub) << options.isep;
    }
    std::cout << ((*iter != 0u) ? options.tsub : options.fsub);
    std::cout << options.osep << (result.GetValue<DefaultArithmeticType>() ? options.tsub : options.fsub) << std::endl;
  } while(cartesianProduct(premutations.begin(), premutations.end(), 0u, 1u));

  clearVariableCache();
}

static void list(const std::string& searchPattern)
{
  const std::regex regex(searchPattern);
  bool isPrevEmptyLine = true;

  std::cerr << "Unary operators" << std::endl;
  for(const auto& i : defaultUnaryOperatorInfoMap)
  {
    const auto& entry = std::get<0u>(i);
    if(entry == nullptr)
    {
      if(!isPrevEmptyLine)
      {
        std::cerr << std::endl;
        isPrevEmptyLine = true;
      }

      continue;
    }

    const auto identifier    = std::string(1u, entry->GetIdentifier());
    const auto precedence    = entry->GetPrecedence();
    const auto associativity = associativityNameMap.at(entry->GetAssociativity());
    const auto& title        = std::get<1u>(i);
    const auto& description  = std::get<2u>(i);
    if(std::regex_match(identifier.begin(), identifier.end(), regex) || std::regex_match(title.begin(), title.end(), regex) ||
       std::regex_match(description.begin(), description.end(), regex))
    {
      std::cout << (boost::format("  %|1$-5|%|2$-5|%|3$-9|%|4$-20|%|5$|") % identifier % precedence % associativity % title % description).str() << std::endl;
      isPrevEmptyLine = false;
    }
  }
  std::cerr << std::endl;

  isPrevEmptyLine = true;
  std::cerr << "Binary operators" << std::endl;
  for(const auto& i : defaultBinaryOperatorInfoMap)
  {
    const auto& entry = std::get<0u>(i);
    if(entry == nullptr)
    {
      if(!isPrevEmptyLine)
      {
        std::cerr << std::endl;
        isPrevEmptyLine = true;
      }

      continue;
    }

    const auto& identifier   = entry->GetIdentifier();
    const auto precedence    = entry->GetPrecedence();
    const auto associativity = associativityNameMap.at(entry->GetAssociativity());
    const auto& title        = std::get<1u>(i);
    const auto& description  = std::get<2u>(i);
    if(std::regex_match(identifier.begin(), identifier.end(), regex) || std::regex_match(title.begin(), title.end(), regex) ||
       std::regex_match(description.begin(), description.end(), regex))
    {
      std::cout << (boost::format("  %|1$-6|%|2$-5|%|3$-9|%|4$-25|%|5$|") % identifier % precedence % associativity % title % description).str() << std::endl;
      isPrevEmptyLine = false;
    }
  }
  std::cerr << std::endl;

  isPrevEmptyLine = true;
  std::cerr << "Functions" << std::endl;
  for(const auto& i : defaultFunctionInfoMap)
  {
    const auto& entry = std::get<0u>(i);
    if(entry == nullptr)
    {
      if(!isPrevEmptyLine)
      {
        std::cerr << std::endl;
        isPrevEmptyLine = true;
      }

      continue;
    }

    const auto& identifier = entry->GetIdentifier();
    const auto argMinCount = ((entry->GetMinArgumentCount() != FunctionToken::GetArgumentCountMaxLimit()) ? std::to_string(entry->GetMinArgumentCount()) : "-");
    const auto argMaxCount = ((entry->GetMaxArgumentCount() != FunctionToken::GetArgumentCountMaxLimit()) ? std::to_string(entry->GetMaxArgumentCount()) : "-");
    const auto& title      = std::get<1u>(i);
    const auto& description = std::get<2u>(i);
    if(std::regex_match(identifier.begin(), identifier.end(), regex) || std::regex_match(title.begin(), title.end(), regex) ||
       std::regex_match(description.begin(), description.end(), regex))
    {
      std::cout << (boost::format("  %|1$-15|%|2$-5|%|3$-5|%|4$-27|%|5$|") % identifier % argMinCount % argMaxCount % title % description).str() << std::endl;
      isPrevEmptyLine = false;
    }
  }
  std::cerr << std::endl;

  isPrevEmptyLine = true;
  std::cerr << "Variables" << std::endl;
  for(const auto& i : defaultVariableInfoMap)
  {
    const auto& entry = std::get<0u>(i);
    if(entry == nullptr)
    {
      if(!isPrevEmptyLine)
      {
        std::cerr << std::endl;
        isPrevEmptyLine = true;
      }

      continue;
    }

    const auto& identifier  = entry->GetIdentifier();
    const auto& title       = std::get<1u>(i);
    const auto& description = std::get<2u>(i);
    if(std::regex_match(identifier.begin(), identifier.end(), regex) || std::regex_match(title.begin(), title.end(), regex) ||
       std::regex_match(description.begin(), description.end(), regex))
    {
      std::cout << (boost::format("  %|1$-18|%|2$-31|%|3$|") % identifier % title % description).str() << std::endl;
      isPrevEmptyLine = false;
    }
  }
}

static void printVersion() { std::cout << (boost::format("%1% v%2%") % PROJECT_NAME % PROJECT_VERSION).str() << std::endl; }

static void printUsage(const boost::program_options::options_description& desc)
{
  std::cerr << (boost::format("%1% -[jlVh] expr...") % PROJECT_EXECUTABLE).str() << std::endl;
  std::cerr << desc << std::endl;
}

int main(int argc, char* argv[])
{
  boost::program_options::options_description namedArgDescs("Options");
  namedArgDescs.add_options()("expr,x", boost::program_options::value<std::vector<std::string>>(), "Add an expression");
  namedArgDescs.add_options()("tsub,t",
                              boost::program_options::value<std::string>(&options.tsub)->default_value(defaultOptions.tsub),
                              "Set \'true\' substitution");
  namedArgDescs.add_options()("fsub,f",
                              boost::program_options::value<std::string>(&options.fsub)->default_value(defaultOptions.fsub),
                              "Set \'false\' substitution");
  namedArgDescs.add_options()("isep,s", boost::program_options::value<std::string>(&options.isep)->default_value(defaultOptions.isep), "Set input separator");
  namedArgDescs.add_options()("osep,S", boost::program_options::value<std::string>(&options.osep)->default_value(defaultOptions.osep), "Set output separator");
  namedArgDescs.add_options()(
      "juxta,j",
      boost::program_options::value<int>()->default_value(defaultOptions.jpo_precedence)->notifier([](int value) { options.jpo_precedence = sgn(value); }),
      "Set juxtaposition operator precedence (-1, 0, 1)");
  namedArgDescs.add_options()("list,l", boost::program_options::value<std::string>()->implicit_value(".*"), "List available operators/functions/variables");
  namedArgDescs.add_options()("verbose,v", "Enable verbose mode");
  namedArgDescs.add_options()("version,V", "Print version");
  namedArgDescs.add_options()("help,h", "Print usage");

  boost::program_options::positional_options_description positionalArgDescs;
  positionalArgDescs.add("expr", -1);

  boost::program_options::variables_map argVariableMap;
  boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(namedArgDescs).positional(positionalArgDescs).run(),
                                argVariableMap);
  boost::program_options::notify(argVariableMap);

  if(argVariableMap.count("help") > 0u)
  {
    printUsage(namedArgDescs);
    std::exit(EXIT_SUCCESS);
  }

  if(argVariableMap.count("version") > 0u)
  {
    printVersion();
    std::exit(EXIT_SUCCESS);
  }

  TruthTableExpressionParser expressionParser;
  InitTruthTable(expressionParser);

  if(argVariableMap.count("list") > 0u)
  {
    list(argVariableMap["list"].as<const std::string&>());
    std::exit(EXIT_SUCCESS);
  }

  bool hasPipedData = std::cin.rdbuf()->in_avail() != -1 && isatty(fileno(stdin)) == 0;
  if(hasPipedData)
  {
    std::string input;
    while(std::getline(std::cin, input))
    {
      evaluate(input, expressionParser);
    }
  }

  if(argVariableMap.count("expr") == 0u && !hasPipedData)
  {
    std::cerr << "*** Error: No expression specified" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  if(argVariableMap.count("expr") > 0u)
  {
    defaultResults.clear();

    const auto& exprs = argVariableMap["expr"].as<const std::vector<std::string>&>();
    for(auto& expr : exprs)
    {
      evaluate(expr, expressionParser);
    }
  }

  std::exit(EXIT_SUCCESS);
}
