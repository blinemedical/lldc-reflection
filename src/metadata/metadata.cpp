/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 */

#include <lldc-reflection/metadata/metadata.h>
#include "private/metadata/metadata.h"

namespace lldc::reflection::metadata {
const char* const OPTIONAL = "OPTIONAL";
const char* const OPTIONAL_DEFAULT = "OPTIONAL_DEFAULT";
const char* const NO_SERIALIZE = "NO_SERIALIZE";
const char* const BLOB = "BLOB";

::rttr::detail::metadata
set_is_optional() {
  return ::rttr::metadata(OPTIONAL, true);
}

::rttr::detail::metadata
set_is_optional_with_default(::rttr::variant value) {
  return ::rttr::metadata(OPTIONAL_DEFAULT, value);
}

::rttr::detail::metadata
set_is_do_not_serialize() {
  return ::rttr::metadata(NO_SERIALIZE, true);
}

::rttr::detail::metadata
set_is_blob() {
  return ::rttr::metadata(BLOB, true);
}

bool
is_optional(const ::rttr::property &property, bool *with_default) {
  bool result = false;
  bool temp_with_default = false;
  auto md_optional = property.get_metadata(metadata::OPTIONAL);
  auto md_optional_default = property.get_metadata(metadata::OPTIONAL_DEFAULT);

  if (md_optional.is_valid()) {
    result = md_optional.to_bool();
  }
  if (md_optional_default.is_valid()) {
    temp_with_default = true;
    result = (md_optional_default.get_type() == property.get_type());
  }

  if (with_default) {
    *with_default = temp_with_default;
  }

  return result;
}

bool
is_optional(const ::rttr::property& property, const ::rttr::variant& reference, bool *matched_reference)
{
  bool has_default = false;
  bool temp_matches_reference = false;
  bool result = is_optional(property, &has_default);

  if (result && has_default) {
    auto md_optional_default = property.get_metadata(metadata::OPTIONAL_DEFAULT);
    temp_matches_reference = (md_optional_default == reference);
  }

  if (matched_reference)
    *matched_reference = temp_matches_reference;

  return result;
}
bool
is_no_serialize(const ::rttr::property &property) {
  return property.get_metadata(metadata::NO_SERIALIZE).is_valid();
}

template <typename T>
bool is_blob(const T &rttr_t) {
  return rttr_t.get_metadata(metadata::BLOB).is_valid();
}

}; // lldc::reflection::metadata
