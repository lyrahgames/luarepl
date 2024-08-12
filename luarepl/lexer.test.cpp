#include <print>
//
#include <doctest/doctest.h>
//
#include "lexer.hpp"

using namespace luarepl;

SCENARIO("") {
  // CHECK(lexer::scan("_myVar") == lexer::context{{identifier{"_myVar"sv}}});
  // CHECK(lexer::scan("for") == lexer::context{{terminal<"for">{"for"}}});

  const auto ctx = lexer::scan(
      R"==(
_globalVar = 10
--[=[
This is a longer comment.
]=]
for x =1,10 do
  local y = x * x -- Let us square the value.
  print("y = " .. y);
end
-- EOF comment)==");

  for (const auto& t : ctx.tokens) print("{}", t);
  print("\n");

  for (auto comment : ctx.comments)
    print("{} ({}{}{})\n", comment.text(), comment.prefix(),
          comment.open_bracket(), comment.close_bracket());
  print("\n");

  for (const auto& e : ctx.errors)
    visit([](auto err) { print("ERROR: {}\n", err.what()); }, e);
  print("\n");
}
