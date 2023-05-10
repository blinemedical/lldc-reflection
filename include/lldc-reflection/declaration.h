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
 *
 * The 'registration_friend' header is included in the event you
 * need to declare a class that will use RTTR_REGISTRATION_FRIEND
 * to permit registration to access non-public members of the
 * object in question.
 */
#pragma once

#include <rttr/type>
#include <rttr/registration_friend>
