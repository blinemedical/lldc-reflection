/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 */

#pragma once

#include <exception>

namespace lldc::reflection::exceptions {

/**
 * @brief This exception type represents when an object has a member
 * whose value is read-only after the creation of the object, but for
 * the sake of serialization, we can leverage the 'setter' to reject
 * seriliazing to an incompatible type based on the constructor-set
 * value.
 */
struct ReferenceValueComparisonMismatch : public std::exception {
  const char* what() const throw () {
    return "unable to set value";
  }
};

struct RequiredMemberSerializationFailure : public std::exception {
  const char* what() const throw () {
    return "required member could not be serialized";
  }
};

}; //lldc::reflection::exceptions
