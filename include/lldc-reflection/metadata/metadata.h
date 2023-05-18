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
   */
  LLDC_REFLECTION_API
  ::rttr::detail::metadata set_is_optional();

  /**
   * @brief Treat the property as optional if this value is set.
   */
  LLDC_REFLECTION_API
  ::rttr::detail::metadata set_is_optional_with_default(::rttr::variant value);

  /**
   * @brief Properties that should not be serialized will have this metadata set in their RTTR
   * registrations.
   */
  LLDC_REFLECTION_API
  ::rttr::detail::metadata set_is_do_not_serialize();

  /**
   * @brief Property will be stored/recovered as a character string.
   */
  LLDC_REFLECTION_API
  ::rttr::detail::metadata set_is_blob();
};
