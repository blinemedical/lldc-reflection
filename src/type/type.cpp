/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 */

#include <any>
#include <functional>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "private/type/type.h"
#include <lldc-reflection/exceptions/exceptions.h>

namespace lldc::reflection::type {

template<class T, class F>
static std::pair<const std::type_index, std::function<::rttr::variant (std::any const&)>>
to_any_visitor(F const& f)
{
  return {
      std::type_index(typeid(T)),
      [g = f](std::any const& a)
      {
          if constexpr (std::is_void_v<T>)
              return g();
          else
              return g(std::any_cast<T const&>(a));
      }
  };
}

static std::unordered_map<std::type_index, std::function<::rttr::variant(std::any const&)>>
any_visitor
{
    to_any_visitor<void>([] { return 0; }),
    to_any_visitor<void*>([] (void* x) { return x; }),
    to_any_visitor<int8_t>([](int8_t x) { return x; }),
    to_any_visitor<int16_t>([](int16_t x) { return x; }),
    to_any_visitor<int32_t>([](int32_t x) { return x; }),
    to_any_visitor<int64_t>([](int64_t x) { return x; }),
    to_any_visitor<uint8_t>([](uint8_t x) { return x; }),
    to_any_visitor<uint16_t>([](uint16_t x) { return x; }),
    to_any_visitor<uint32_t>([](uint32_t x) { return x; }),
    to_any_visitor<uint64_t>([](uint64_t x) { return x; }),
    to_any_visitor<float>([](float x) { return x; }),
    to_any_visitor<double>([](double x) { return x; }),
    to_any_visitor<bool>([](bool x) { return x; }),
    to_any_visitor<char>([](char x) { return x; }),
    to_any_visitor<char const*>([](char const* s) { return s; }),
    to_any_visitor<std::string>([](const std::string& s) { return s; }),
};

::rttr::variant
extract_any_value(const ::rttr::variant &in)
{
  if (is_any(in.get_type())) {
    const std::any &a = in.get_value<std::any>();
    if (const auto it = any_visitor.find(std::type_index(a.type())); it != any_visitor.cend())
      return it->second(a);
    throw exceptions::UnhandledAnyConversion();
  }
  return ::rttr::variant();
}

};// lldc::reflection::type