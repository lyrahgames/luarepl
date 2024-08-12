#pragma once
#include <variant>
//
#include "utility.hpp"

namespace luarepl {

struct token_type_base {
  friend constexpr auto operator<=>(const token_type_base&,
                                    const token_type_base&) noexcept = default;

  constexpr auto source() const noexcept { return src; }
  string_view src{};
};

struct identifier : token_type_base {};

struct numeral : token_type_base {};

struct string_literal : token_type_base {};

template <static_zstring str>
struct terminal : token_type_base {
  static constexpr auto string() noexcept -> czstring {
    // return {str.begin(), str.end()};
    return str.data();
  }
};

using value_keywords = static_zstring_list<"false", "nil", "true">;
using operator_keywords = static_zstring_list<"and", "not", "or">;
using control_keywords = static_zstring_list<"break",
                                             "do",
                                             "else",
                                             "elseif",
                                             "end",
                                             "for",
                                             "function",
                                             "goto",
                                             "if",
                                             "in",
                                             "local",
                                             "repeat",
                                             "return",
                                             "then",
                                             "until",
                                             "while">;
using keywords =
    decltype(value_keywords{} + operator_keywords{} + control_keywords{});

using operators_and_punctuation = static_zstring_list<  //
    "+",
    "-",
    "*",
    "/",
    "%",
    "^",
    "#",
    "&",
    "~",
    "|",
    "<<",
    ">>",
    "//",
    "==",
    "~=",
    "<=",
    ">=",
    "<",
    ">",
    "=",
    "(",
    ")",
    "{",
    "}",
    "[",
    "]",
    "::",
    ";",
    ":",
    ",",
    ".",
    "..",
    "...">;

namespace detail {
template <typename type>
struct is_control_keyword : std::false_type {};
template <static_zstring str>
struct is_control_keyword<terminal<str>> {
  static constexpr bool value = contains<str>(control_keywords{});
};
template <typename type>
struct is_operator_keyword : std::false_type {};
template <static_zstring str>
struct is_operator_keyword<terminal<str>> {
  static constexpr bool value = contains<str>(operator_keywords{});
};
template <typename type>
struct is_value_keyword : std::false_type {};
template <static_zstring str>
struct is_value_keyword<terminal<str>> {
  static constexpr bool value = contains<str>(value_keywords{});
};
template <typename type>
struct is_operator_or_punctuation : std::false_type {};
template <static_zstring str>
struct is_operator_or_punctuation<terminal<str>> {
  static constexpr bool value = contains<str>(operators_and_punctuation{});
};
}  // namespace detail

template <typename type>
concept control_keyword = detail::is_control_keyword<type>::value;
template <typename type>
concept operator_keyword = detail::is_operator_keyword<type>::value;
template <typename type>
concept value_keyword = detail::is_value_keyword<type>::value;
template <typename type>
concept operator_or_punctuation =
    detail::is_operator_or_punctuation<type>::value;

// Use transform of `value_list` to `type_list` with predicate.
template <static_zstring... str>
consteval auto terminal_list_from(static_zstring_list<str...>) {
  return type_list<terminal<str>...>{};
}

using keyword_types = decltype(terminal_list_from(keywords{}));
using operators_and_punctuation_types =
    decltype(terminal_list_from(operators_and_punctuation{}));

using terminals = decltype(keyword_types{} + operators_and_punctuation_types{});

using token_types =
    decltype(type_list<identifier, numeral, string_literal>{} + terminals{});

template <typename... types>
consteval auto variant_from(type_list<types...>) -> variant<types...>;

using token_base = decltype(variant_from(token_types{}));
struct token : token_base {
  using token_base::token_base;
};

}  // namespace luarepl

template <>
struct std::formatter<luarepl::identifier, char> {
  template <class context>
  constexpr auto parse(context& ctx) -> context::iterator {
    return ctx.begin();
  }
  template <class context>
  auto format(luarepl::identifier t, context& ctx) const -> context::iterator {
    return ranges::copy(std::format("<id: {}>", t.source()), ctx.out()).out;
  }
};

template <>
struct std::formatter<luarepl::numeral, char> {
  template <class context>
  constexpr auto parse(context& ctx) -> context::iterator {
    return ctx.begin();
  }
  template <class context>
  auto format(luarepl::numeral t, context& ctx) const -> context::iterator {
    return ranges::copy(std::format("<num: {}>", t.source()), ctx.out()).out;
  }
};

template <>
struct std::formatter<luarepl::string_literal, char> {
  template <class context>
  constexpr auto parse(context& ctx) -> context::iterator {
    return ctx.begin();
  }
  template <class context>
  auto format(luarepl::string_literal t,
              context& ctx) const -> context::iterator {
    return ranges::copy(std::format("<str: {}>", t.source()), ctx.out()).out;
  }
};

template <luarepl::static_zstring str>
struct std::formatter<luarepl::terminal<str>, char> {
  template <class context>
  constexpr auto parse(context& ctx) -> context::iterator {
    return ctx.begin();
  }
  template <class context>
  auto format(luarepl::terminal<str> t,
              context& ctx) const -> context::iterator {
    return ranges::copy(std::format("<{}>", str.data()), ctx.out()).out;
  }
};

template <>
struct std::formatter<luarepl::token, char> {
  template <class context>
  constexpr auto parse(context& ctx) -> context::iterator {
    return ctx.begin();
  }
  template <class context>
  auto format(luarepl::token t, context& ctx) const -> context::iterator {
    const auto str =
        std::visit([](auto&& x) { return std::format("{}", x); }, t);
    return ranges::copy(str, ctx.out()).out;
  }
};
