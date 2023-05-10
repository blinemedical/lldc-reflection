/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 */

#include "private/associative-containers.h"

#ifndef LLDC_RTTR_ASSOCIATIVE_CONTAINERS_KEY
#define LLDC_RTTR_ASSOCIATIVE_CONTAINERS_KEY "key"
#endif

#ifndef LLDC_RTTR_ASSOCIATIVE_CONTAINERS_VALUE
#define LLDC_RTTR_ASSOCIATIVE_CONTAINERS_VALUE "value"
#endif

namespace lldc::rttr::associative_containers {
const char* const KEY = LLDC_RTTR_ASSOCIATIVE_CONTAINERS_KEY;
const char* const VALUE = LLDC_RTTR_ASSOCIATIVE_CONTAINERS_VALUE;
}; // lldc::rttr::associative_containers
