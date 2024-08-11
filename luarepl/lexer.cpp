#include "lexer.hpp"

namespace luarepl::lexer {

auto scan(string_view text) -> context {
  context ctx{text};

  auto it = ctx.current();

  while (!ctx.done()) {
    if (match(it, is_whitespace)) {      // whitespace
    } else if (scan_comment(ctx, it)) {  // comments
    } else if (match_identifier(it)) {   // identifier
      string_view name{ctx.current().pointer(), it.pointer()};
      const auto is_keyword =
          for_each_until(keyword_types{}, [name, &ctx]<typename type> {
            if (name != type::string()) return false;
            ctx.tokens.emplace_back(type{name});
            return true;
          });
      if (!is_keyword) ctx.tokens.emplace_back(identifier{name});
    } else if (match_string(it)) {  // string
      ctx.tokens.emplace_back(
          string_literal{string_view{ctx.current().pointer(), it.pointer()}});
    } else if (match_numeral(it)) {  // number
      ctx.tokens.emplace_back(
          numeral{string_view{ctx.current().pointer(), it.pointer()}});
    } else {
      const auto is_other =
          traverse(static_radix_tree_from(operators_and_punctuation{}),
                   it.pointer(), [&ctx, &it]<auto str>(auto tail) {
                     ctx.tokens.emplace_back(
                         terminal<str>{string_view{it.pointer(), tail}});
                     it.ptr = tail;
                   });

      if (!is_other)
        throw runtime_error(
            "Failed to scan given string. Unexpected codepoint.");
    }

    ctx.advance(it);
  }

  return ctx;
}

bool scan_comment(context& ctx, iterator& it) {
  if (!match<"--">(it)) return false;

  ctx.comments.push_back({});
  auto& entry = ctx.comments.back();
  entry.base = ctx.current().pointer();
  entry.prefix_end = it.pointer();

  if (match_open_long_bracket(it)) {
    entry.open_end = it.pointer();
    const auto level = entry.open_bracket().size() - 2;
    for (; *it; ++it) {
      const auto tmp = it;
      if (match_close_long_bracket(it, level)) {
        entry.text_end = tmp.pointer();
        entry.close_end = it.pointer();
        return true;
      }
    }
    entry.text_end = it.pointer();
    entry.close_end = it.pointer();
    ctx.error(unfinished_long_comment{});
    return true;
  }

  entry.open_end = it.pointer();
  auto tmp = it;
  for (; *it; ++it) {
    tmp = it;
    if (match_end_of_line(it)) break;
  }
  entry.text_end = tmp.pointer();
  entry.close_end = tmp.pointer();
  return true;
}

bool scan_identifier_or_keyword(context& ctx) {
  auto it = ctx.current();
  if (!match_identifier(it)) return false;
  string_view name{ctx.current().pointer(), it.pointer()};
  const auto is_keyword =
      for_each_until(keyword_types{}, [name, &ctx]<typename type> {
        if (name != type::string()) return false;
        ctx.tokens.emplace_back(type{name});
        return true;
      });
  if (!is_keyword) ctx.tokens.emplace_back(identifier{name});
  return true;
}

}  // namespace luarepl::lexer
