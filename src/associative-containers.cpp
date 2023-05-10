/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 */

#include "private/associative-containers.h"

#ifndef LLDC_REFLECTION_ASSOCIATIVE_CONTAINERS_KEY
#define LLDC_REFLECTION_ASSOCIATIVE_CONTAINERS_KEY "key"
#endif

#ifndef LLDC_REFLECTION_ASSOCIATIVE_CONTAINERS_VALUE
#define LLDC_REFLECTION_ASSOCIATIVE_CONTAINERS_VALUE "value"
#endif

namespace lldc::reflection::associative_containers {
const char* const KEY = LLDC_REFLECTION_ASSOCIATIVE_CONTAINERS_KEY;
const char* const VALUE = LLDC_REFLECTION_ASSOCIATIVE_CONTAINERS_VALUE;
}; // lldc::reflection::associative_containers
