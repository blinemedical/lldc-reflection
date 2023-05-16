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

bool is_optional(const ::rttr::property &property);

bool is_no_serialize(const ::rttr::property &propety);

template <typename T>
bool is_blob(const T &t);
template bool is_blob(const ::rttr::property &property);
template bool is_blob(const ::rttr::type &_type);

}; // lldc::reflection::metadata
