/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 *
 * Here the types are registered with RTTR using some of the
 * metadata fields, etc.
 */
#include "common/example_messages.h"

#include <lldc-reflection/registration.h>

namespace lldc::testing {
  /**
   * @brief Define the ApiMessage::SetSubject to throw an exception if
   * the stored value is set to something other than 'not_set' and the
   * set value versus the paramater mismatch.  This will ensure that
   * an exception happens if we were to try to serialize from one type
   * to another based on misconfiguring the 'subject' member defined in
   * this base class.
   *
   * @param subject The incoming subject.
   */
  void
  ApiMessage::SetSubject(Subject subject) {
    if (_subject != Subject::not_set && subject != _subject)
      throw ::lldc::reflection::exceptions::ReferenceValueComparisonMismatch();
    _subject = subject;
  }
};


RTTR_PLUGIN_REGISTRATION {
  namespace T = lldc::testing;

  /**
   * @brief Register the enumerated values against the string
   * names that are used to represent the enumeration.
   */
  ::rttr::registration::enumeration<T::Subject>("subject") (
    ::rttr::value(nullptr, T::Subject::not_set),
    ::rttr::value("first-message", T::Subject::first_message),
    ::rttr::value("second-message", T::Subject::second_message)
  );

  /**
   * @brief This message was declared with registration as a
   * 'friend', so the subject property setter can be reached by
   * serialization, where this base class will perform special
   * handling to prevent derived classes from being serialized
   * to one another while still letting this base class be used
   * to fetch the common 'subject' field for switch statements
   * and the like when doing message routing.
   */
  ::rttr::registration::class_<T::ApiMessage>("api-message")
    .constructor<T::Subject>()
    .property("subject", &T::ApiMessage::GetSubject, &T::ApiMessage::SetSubject)
    ;

  /**
   * @brief Register the 'FirstMessage' type with its registered
   * structure body.
   */
  ::rttr::registration::class_<T::FirstMessage>("first-message")
    .property("body", &T::FirstMessage::body)
    ;

  /**
   * @brief Register the FirstMessage's Body structure.
   * NOTE: The namespacing is only for our benefit here.
   */
  ::rttr::registration::class_<T::FirstMessage::Body>("first-message::body")
    // NOTE: this is to support the by-value member; we get a construction error from rttr
    //       when trying to create this from an intermediate type if we do not specify
    //       this policy.
    .constructor<>()(::rttr::policy::ctor::as_object)
    .property("data", &T::FirstMessage::Body::data)
    ;

  /**
   * @brief Register the 'SecondMessage' type with its body of fields.
   */
  ::rttr::registration::class_<T::SecondMessage>("second-message")
    .property("some_string", &T::SecondMessage::some_string)
    .property("some_char",   &T::SecondMessage::some_char)
    .property("some_bool",   &T::SecondMessage::some_bool)
    .property("some_uint64", &T::SecondMessage::some_uint64)
    .property("some_uint32", &T::SecondMessage::some_uint32)
    .property("some_uint16", &T::SecondMessage::some_uint16)
    .property("some_uint8",  &T::SecondMessage::some_uint8)
    .property("some_int64",  &T::SecondMessage::some_int64)
    .property("some_int32",  &T::SecondMessage::some_int32)
    .property("some_int16",  &T::SecondMessage::some_int16)
    .property("some_int8",   &T::SecondMessage::some_int8)
    .property("some_float",  &T::SecondMessage::some_float)
    .property("some_double",  &T::SecondMessage::some_double)
    ;
};
