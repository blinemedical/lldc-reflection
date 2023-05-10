/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 */
#pragma once

namespace lldc::reflection::metadata {
  /**
   * @brief Properties marked as optional will have this metadata set in their RTTR registration.
   */
  extern const char* const OPTIONAL;

  /**
   * @brief Properties that should not be serialized will have this metadata set in their RTTR
   * registrations.
   */
  extern const char* const NO_SERIALIZE;

  /**
   * @brief Property will be stored/recovered as a character string.
   */
  extern const char* const BLOB;
};
