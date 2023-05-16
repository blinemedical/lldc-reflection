/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 *
 * Private header for dealing with type narrowing, etc.
 */
#pragma once

#include <rttr/registration>

namespace lldc::reflection::type {

inline bool is_fundamental (const ::rttr::type &t) {
  return (t.is_arithmetic() || t.is_enumeration() || (t == ::rttr::type::get<std::string>()));
}

}; // lldc::reflection::metadata
