/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 *
 * This header extends the public headers to include methods for checking
 * the values of optional fields.
 */
#pragma once

#include <rttr/registration>

namespace lldc::reflection::metadata {
extern const char* const OPTIONAL;
extern const char* const OPTIONAL_DEFAULT;
extern const char* const NO_SERIALIZE;
extern const char* const BLOB;

bool is_optional(const ::rttr::property &property, bool *has_default);
bool is_optional(const ::rttr::property& property, const ::rttr::variant& reference, bool *matched_reference);

bool is_no_serialize(const ::rttr::property &propety);

template <typename T>
bool is_blob(const T &t) {
  auto md = t.get_metadata(metadata::BLOB);
  if (md.is_valid())
    return md.to_bool();
  return false;
}
template bool is_blob(const ::rttr::property &property);
template bool is_blob(const ::rttr::type &_type);

}; // lldc::reflection::metadata
