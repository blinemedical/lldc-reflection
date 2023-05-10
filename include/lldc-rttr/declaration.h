/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 *
 * When declaring structs/classes that will use the RTTR_ENABLE
 * macro, include this header, e.g.:
 *
 *   struct Something {
 *     std::string member;
 *
 *     RTTR_ENABLE();
 *   };
 *
 *   struct SomethingChild : public Something {
 *     std::string child_member;
 *
 *     RTTR_ENABLE(Something);
 *   };
 */
#pragma once

#include <rttr/type>
