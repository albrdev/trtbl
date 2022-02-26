cmake_minimum_required(VERSION 3.14)

target_sources(${TARGET_TRTBL}
  PUBLIC
    TruthTableExpressionParser.hpp
    TrTblSetup.hpp

  PRIVATE
    TruthTableExpressionParser.cpp
    ExpressionParserDefaultSetup.cpp
)
