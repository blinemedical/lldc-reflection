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

  const uint64_t OptionalMemberMessage::DEFAULT_U64_VALUE = 86;

  const long MaybeEmpty::DEFAULT_VALUE = 32;
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
    ::rttr::value("second-message", T::Subject::second_message),
    ::rttr::value("optional-member-message", T::Subject::optional_member_message)
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
    .constructor<>()(::rttr::policy::ctor::as_object)
    .constructor<>()(::rttr::policy::ctor::as_raw_ptr)
    .constructor<>()(::rttr::policy::ctor::as_std_shared_ptr)
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
    .property("some_double", &T::SecondMessage::some_double)
    ;

  ::rttr::registration::class_<T::OptionalMemberMessage>("optional-member-message")
    .property("required_string", &T::OptionalMemberMessage::required_string)
    .property("optional_string", &T::OptionalMemberMessage::optional_string)
      (::lldc::reflection::metadata::set_is_optional())
    .property("optional_vector", &T::OptionalMemberMessage::optional_vector)
      (
        ::lldc::reflection::metadata::set_is_optional(),
        // Good idea for big data types, per rttr manual
        ::rttr::policy::prop::as_reference_wrapper
      )
    .property("required_vector", &T::OptionalMemberMessage::required_vector)
      (::rttr::policy::prop::as_reference_wrapper)
    .property("optional_map", &T::OptionalMemberMessage::optional_map)
      (
        ::lldc::reflection::metadata::set_is_optional(),
        // Good idea for big data types, per rttr manual
        ::rttr::policy::prop::as_reference_wrapper
      )
    .property("required_map", &T::OptionalMemberMessage::required_map)
      (::rttr::policy::prop::as_reference_wrapper)
    .property("optional_defaulted_uint64", &T::OptionalMemberMessage::optional_defaulted_uint64)
      (::lldc::reflection::metadata::set_is_optional_with_default(T::OptionalMemberMessage::DEFAULT_U64_VALUE))
    .property("optional_uint64", &T::OptionalMemberMessage::optional_uint64)
      (::lldc::reflection::metadata::set_is_optional())
    .property("required_uint64", &T::OptionalMemberMessage::required_uint64)
    .property("optional_sptr", &T::OptionalMemberMessage::optional_sptr)
      (::lldc::reflection::metadata::set_is_optional())
    .property("required_sptr", &T::OptionalMemberMessage::required_sptr)
    .property("optional_rawptr", &T::OptionalMemberMessage::optional_rawptr)
      (::lldc::reflection::metadata::set_is_optional())
    .property("required_rawptr", &T::OptionalMemberMessage::required_rawptr)

    .property("optional_obj", &T::OptionalMemberMessage::optional_obj)
      (::lldc::reflection::metadata::set_is_optional())
    .property("required_obj", &T::OptionalMemberMessage::required_obj)
    ;

  ::rttr::registration::class_<T::OptionalMemberMessage::Payload>("optional-member-message::payload")
    .constructor<>() (::rttr::policy::ctor::as_std_shared_ptr)
    .constructor<>() (::rttr::policy::ctor::as_raw_ptr)
    .property("value", &T::OptionalMemberMessage::Payload::value)
    ;

  ::rttr::registration::class_<T::SimpleMessage>("simple-message")
    .constructor<>() (::rttr::policy::ctor::as_object)
    .constructor<>() (::rttr::policy::ctor::as_raw_ptr)
    .constructor<>() (::rttr::policy::ctor::as_std_shared_ptr)
    .property("name", &T::SimpleMessage::name)
    .property("payload", &T::SimpleMessage::payload)
    ;

  ::rttr::registration::class_<T::SimpleMessage::Payload>("simple-message::payload")
    .constructor<>() (::rttr::policy::ctor::as_object)
    .constructor<>() (::rttr::policy::ctor::as_std_shared_ptr)
    .constructor<>() (::rttr::policy::ctor::as_raw_ptr)
    // NOTE: 'member' is intentionally not registered, so this object will only ever have no
    //       property members according to RTTR.
    ;

  ::rttr::registration::class_<T::MessageWithAnys>("message-with-anys")
    .property("properties", &T::MessageWithAnys::properties)
    ;

  ::rttr::registration::class_<T::MessageWithVectors>("message-with-vectors")
    .property("v-int", &T::MessageWithVectors::v_int)
    .property("vv-int", &T::MessageWithVectors::vv_int)
    .property("v-sptr", &T::MessageWithVectors::v_sptr)
    .property("v-obj", &T::MessageWithVectors::v_obj)
    ;

  ::rttr::registration::class_<T::MaybeEmpty>("maybe-empty")
    .property("value", &T::MaybeEmpty::value)
      (::lldc::reflection::metadata::set_is_optional_with_default(T::MaybeEmpty::DEFAULT_VALUE))
    ;
};
