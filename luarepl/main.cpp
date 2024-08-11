#include <format>
//
#include <replxx.hxx>
//
#include <sol/sol.hpp>
//
#include "lexer.hpp"

using namespace std;

using czstring = const char*;
using Replxx = replxx::Replxx;

Replxx repl{};
sol::state lua{};

void init_repl() {
  repl.set_indent_multiline(true);
}

auto input_from_repl() -> czstring {
  return repl.input("\nluarepl> ");
}

void log(const string& str) {
  repl.print(str.c_str());
}

void init_lua() {
  lua.open_libraries(sol::lib::base,       //
                     sol::lib::coroutine,  //
                     sol::lib::string,     //
                     sol::lib::io);
}

void eval_lua(czstring input) {
  auto result = lua.safe_script(input, sol::script_pass_on_error);
  if (!result.valid()) {
    sol::error e = result;
    log(format("ERROR: {}\n", e.what()));
  }
}

int main() {
  init_repl();
  init_lua();

  czstring input{};
  while (input = input_from_repl()) {
    repl.history_add(input);
    log("\n");
    eval_lua(input);
  }
}
