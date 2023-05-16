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
set_is_do_not_serialize() {
  return ::rttr::metadata(NO_SERIALIZE, true);
}

::rttr::detail::metadata
set_is_blob() {
  return ::rttr::metadata(BLOB, true);
}

bool
is_optional(const ::rttr::property &property) {
  return property.get_metadata(metadata::OPTIONAL).is_valid();
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
