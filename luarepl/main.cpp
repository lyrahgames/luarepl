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

void hook_color(std::string const& context, Replxx::colors_t& colors);

void init_repl() {
  repl.install_window_change_handler();
  repl.set_indent_multiline(true);
  repl.set_highlighter_callback(hook_color);
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

template <typename... types>
struct overloaded : types... {
  using types::operator()...;
};

void hook_color(std::string const& context, Replxx::colors_t& colors) {
  using cl = Replxx::Color;
  using namespace luarepl;

  const auto base = context.c_str();
  const auto ctx = lexer::scan(context.c_str());

  const auto set_color_for_token = [&](auto& t, cl color) {
    const auto offset = t.source().begin() - base;
    for (size_t i = 0; i < t.source().size(); ++i)
      colors.at(offset + i) = color;
  };

  for (auto& t : ctx.tokens)
    visit(
        overloaded{
            [&](auto t) { /* nothing */ },
            [&](numeral t) { set_color_for_token(t, cl::BROWN); },
            [&](string_literal t) { set_color_for_token(t, cl::GREEN); },
            [&](control_keyword auto t) { set_color_for_token(t, cl::BLUE); },
            [&](operator_keyword auto t) { set_color_for_token(t, cl::CYAN); },
            [&](operator_or_punctuation auto t) {
              set_color_for_token(t, cl::CYAN);
            },
            [&](value_keyword auto t) { set_color_for_token(t, cl::BROWN); },
        },
        t);

  //   auto color = cl::WHITE;

  //   switch (t.type) {
  //     case token::BREAK:
  //     case token::DO:
  //     case token::ELSE:
  //     case token::ELSEIF:
  //     case token::END:
  //     case token::FOR:
  //     case token::FUNCTION:
  //     case token::IF:
  //     case token::IN:
  //     case token::LOCAL:
  //     case token::REPEAT:
  //     case token::RETURN:
  //     case token::THEN:
  //     case token::UNTIL:
  //     case token::WHILE:
  //       color = cl::BLUE;
  //       break;

  //     case token::AND:
  //     case token::OR:
  //     case token::NOT:
  //       color = cl::CYAN;
  //       break;

  //     case token::NIL:
  //     case token::FALSE:
  //     case token::TRUE:
  //       color = cl::BROWN;
  //       break;

  //     case token::PLUS:
  //     case token::MINUS:
  //     case token::ASTERISK:
  //     case token::SLASH:
  //     case token::PERCENT:
  //     case token::CARET:
  //     case token::HASHTAG:
  //     case token::DOUBLE_EQUAL:
  //     case token::SIMILAR_EQUAL:
  //     case token::SMALLER_EQUAL:
  //     case token::GREATER_EQUAL:
  //     case token::SMALLER:
  //     case token::GREATER:
  //     case token::EQUAL:
  //     case token::LPAREN:
  //     case token::RPAREN:
  //     case token::LBRACKET:
  //     case token::RBRACKET:
  //     case token::LCURL:
  //     case token::RCURL:
  //     case token::SEMICOLON:
  //     case token::COLON:
  //     case token::COMMA:
  //     case token::DOT:
  //     case token::DOTDOT:
  //     case token::DOTDOTDOT:
  //       color = cl::CYAN;
  //       break;

  //     case token::STRING:
  //       color = cl::GREEN;
  //       break;

  //     case token::NUMBER:
  //       color = cl::BROWN;
  //       break;
  //   }

  //   if (color == cl::WHITE) continue;
  //   const auto offset = t.source.begin() - base;
  //   for (size_t i = 0; i < t.source.size(); ++i)  //
  //     colors.at(offset + i) = color;
  // }

  // if (l.tokens.back().type == token::END)
  //   repl.bind_key_internal(Replxx::KEY::ENTER, "commit_line");
  // else
  //   repl.bind_key_internal(Replxx::KEY::ENTER, "new_line");
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
