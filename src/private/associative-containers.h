/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 *
 * When interpreting associative containers like:
 *   [ {'key': <key>, 'value': <value>}, ...]
 *
 * It is sometimes preferable to change under what name the
 * key and value will be stored, as members of the object
 * (i.e., 'key' and 'value' in the above example).
 *
 * Define the LLDC_REFLECTION_ASSOCIATIVE_CONTAINER_KEY and/or
 * LLDC_REFLECTION_ASSOCIATIVE_CONTAINER_VALUE when compiling
 * to change behavior of the library.
 */
#pragma once

namespace lldc::reflection::associative_containers {
  extern const char* const KEY;
  extern const char* const VALUE;
}; // lldc::reflection::associative_container
