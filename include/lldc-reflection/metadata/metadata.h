/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 *
 * These methods can be used during object registration to mark
 * properties for alternate handling within the converters.
 */
#pragma once

#include <lldc-reflection/api.h>
#include <rttr/registration>

namespace lldc::reflection::metadata {
  /**
   * @brief Properties marked as optional will have this metadata set in their RTTR registration.
   * "To" Behavior:
   *   Container types (e.g., string, vector) are skipped if marked optional and are empty/zero-length.
   *   Pointer types are skipped if set to null (i.e., empty).
   *   Everything else will be included since without a default value there is no unambiguous way to
   *     know _when_ it is optional.
   *
   * "From" Behavior:
   *   If the property member name is not found, and it has been marked optional, it will be skipped
   *   without trigging a failure of serialization.  The expectation is the target RTTR-registered
   *   type will already have a default set for that member, so serialization should not touch it.
   */
  LLDC_REFLECTION_API
  ::rttr::detail::metadata set_is_optional();

  /**
   * @brief Treat the property as optional if this value is set.
   * "To" Behavior:
   *   When preparing to convert the member, if the source value matches the declared default (via
   *     operator==), the member will be skipped.
   *
   * "From" Behavior:
   *   If the property member name is not found, and it has been marked optional, it will be skipped
   *   without trigging a failure of serialization.  The expectation is the target RTTR-registered
   *   type will already have a default set for that member, so serialization should not touch it.
   */
  LLDC_REFLECTION_API
  ::rttr::detail::metadata set_is_optional_with_default(::rttr::variant value);

  /**
   * @brief Properties that should not be serialized will have this metadata set in their RTTR
   * registrations.
   *
   * NOTE: This is the same as simply not declaring the property/member in the
   *   RTTR[_PLUGIN]_REGISTRATION macro, but provided in the event one is trying to provide some
   *   bookkeeping or otherwise pertaining to the member.
   */
  LLDC_REFLECTION_API
  ::rttr::detail::metadata set_is_do_not_serialize();

  /**
   * @brief Property will be stored/recovered as a character string.  This is a way to provide a
   * generic "payload" member in an object which can then be passed to another to/from routine
   * with other registered types pertianing to payload(s) of the parent class.
   */
  LLDC_REFLECTION_API
  ::rttr::detail::metadata set_is_blob();
};
