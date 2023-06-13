/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 *
 * This header provides a basic example of a message structure
 * common to public APIs where there is a base API message with
 * some 'subject' field that is usually populated with a string
 * value telling the receiver what other fields to expect from
 * the message.  From that example, we'll show in the tests how
 * to "check" for that type to support serializing to the
 * destination structure in switch statements, etc. by using an
 * integer enumeration derived from that string value.
 */
#pragma once

#include <common/api.h>
#include <lldc-reflection/declaration.h>

#include <map>
#include <string>
#include <vector>
#include <any>

namespace lldc::testing {

enum class COMMON_TEST_API
Subject {
  not_set = -1,
  first_message,
  second_message,
  optional_member_message
};

struct COMMON_TEST_API
ApiMessage {
  RTTR_REGISTRATION_FRIEND;

  ApiMessage(Subject subject=Subject::not_set) : _subject(subject) {}

  virtual ~ApiMessage() = default;

  Subject GetSubject() const { return _subject; }

  friend bool operator==(const ApiMessage &lhs, const ApiMessage &rhs) {
    return (lhs._subject == rhs._subject);
  }

  protected:
  void SetSubject(Subject subject);

  private:
  Subject _subject;

  RTTR_ENABLE();
};

struct COMMON_TEST_API
FirstMessage : public ApiMessage {
  FirstMessage() : ApiMessage(Subject::first_message) {}

  struct Body {
    std::map<std::string, std::string> data;

    friend bool operator==(const Body &lhs, const Body &rhs) {
      return (lhs.data == rhs.data);
    }

    RTTR_ENABLE();
  };

  Body body;

  friend bool operator==(const FirstMessage &lhs, const FirstMessage &rhs) {
    return (lhs.body == rhs.body);
  }

  RTTR_ENABLE(ApiMessage);
};

struct COMMON_TEST_API
SecondMessage : public ApiMessage {
  SecondMessage() : ApiMessage(Subject::second_message) {}

  friend bool operator==(const SecondMessage &lhs, const SecondMessage &rhs) {
    return (
      std::tie(
            lhs.some_string,
            lhs.some_char,
            lhs.some_bool,
            lhs.some_float,
            lhs.some_double,
            lhs.some_uint64,
            lhs.some_uint32,
            lhs.some_uint16,
            lhs.some_uint8,
            lhs.some_int64,
            lhs.some_int32,
            lhs.some_int16,
            lhs.some_int8)
      ==
      std::tie(
            rhs.some_string,
            rhs.some_char,
            rhs.some_bool,
            rhs.some_float,
            rhs.some_double,
            rhs.some_uint64,
            rhs.some_uint32,
            rhs.some_uint16,
            rhs.some_uint8,
            rhs.some_int64,
            rhs.some_int32,
            rhs.some_int16,
            rhs.some_int8)
    );
  }

  std::string some_string;
  char        some_char   = '\0';
  bool        some_bool   = false;
  float       some_float  = 0.0f;
  double      some_double = 0.0;
  uint64_t    some_uint64 = 0;
  uint32_t    some_uint32 = 0;
  uint16_t    some_uint16 = 0;
  uint8_t     some_uint8  = 0;
  int64_t     some_int64  = 0;
  int32_t     some_int32  = 0;
  int16_t     some_int16  = 0;
  int8_t      some_int8   = 0;

  RTTR_ENABLE(ApiMessage);
};

/**
 * @brief This message showcases the behavior of the "optional" metadata.
 * On the from -side of the conversion:
 *   1. A missing member name that is optional/optional_default will not
 *      generate an error/failure to convert exception.
 *   2. Default values are not used in the 'from' processes at this time;
 *      the assumption is the target object would have set that default on
 *      its own.
 *   3. Wrapped objects (shared_ptr) are not  supported at this time.
 * On the to -side of the conversion:
 *   1. An optional container or object pointer member (bool <type>::empty())
 *      that are empty (or nullptr) will be skipped.
 *   2. An optional fundamental value having no default will not be sanity
 *      checked on intent; it will be skipped.
 *   3. An optional+default member will be skipped if the reference object's
 *      value matches the registered default.
 */
struct COMMON_TEST_API
OptionalMemberMessage : public ApiMessage
{
  static const uint64_t DEFAULT_U64_VALUE = 86;

  OptionalMemberMessage() :
    ApiMessage(Subject::optional_member_message),
    optional_string(),
    required_string(),
    optional_vector(),
    required_vector(),
    optional_map(),
    required_map(),
    optional_defaulted_uint64(DEFAULT_U64_VALUE),
    optional_uint64(12345),
    required_uint64(67890),
    optional_sptr(nullptr),
    required_sptr(nullptr),
    optional_rawptr(nullptr),
    required_rawptr(nullptr),
    optional_obj(),
    required_obj()
  { /** intentionally empty */ }

  ~OptionalMemberMessage() {
    if (optional_rawptr)
      delete optional_rawptr;
    if (required_rawptr)
      delete required_rawptr;
  }

  struct Payload {
    int32_t value;

    friend bool operator==(const Payload& lhs, const Payload& rhs) {
      return (lhs.value == rhs.value);
    }

    RTTR_ENABLE();
  };

  // Container types
  std::string                     optional_string;
  std::string                     required_string;
  std::vector<uint64_t>           optional_vector;
  std::vector<uint64_t>           required_vector;
  std::map<std::string, uint64_t> optional_map;
  std::map<std::string, uint64_t> required_map;

  // Value -type member
  uint64_t                        optional_defaulted_uint64;
  uint64_t                        optional_uint64;
  uint64_t                        required_uint64;

  // Unique Pointer
  // NOT SUPPORTED: https://github.com/rttrorg/rttr/issues/39
  // std::unique_ptr<Payload>        optional_uptr;

  // Shared Pointer
  std::shared_ptr<Payload>        optional_sptr;
  std::shared_ptr<Payload>        required_sptr;

  // Raw Pointer
  Payload*                        optional_rawptr;
  Payload*                        required_rawptr;

  // By-value to object
  Payload                         optional_obj;
  Payload                         required_obj;

  friend bool operator==(const OptionalMemberMessage &lhs, const OptionalMemberMessage &rhs) {
    bool containers_and_values_match = (
      std::tie(
        lhs.optional_string, lhs.required_string,
        lhs.optional_vector, lhs.required_vector,
        lhs.optional_map, lhs.required_map,
        lhs.optional_defaulted_uint64, lhs.optional_uint64, lhs.required_uint64,
        lhs.optional_obj, lhs.required_obj)
      ==
      std::tie(
        rhs.optional_string, rhs.required_string,
        rhs.optional_vector, rhs.required_vector,
        rhs.optional_map, rhs.required_map,
        rhs.optional_defaulted_uint64, rhs.optional_uint64, rhs.required_uint64,
        rhs.optional_obj, rhs.required_obj)
    );

    std::vector<std::pair<const OptionalMemberMessage::Payload*, const OptionalMemberMessage::Payload*>> pointers = {
      std::make_pair(lhs.optional_sptr.get(), rhs.optional_sptr.get()),
      std::make_pair(lhs.required_sptr.get(), rhs.required_sptr.get()),
      std::make_pair(lhs.optional_rawptr, rhs.optional_rawptr),
      std::make_pair(lhs.required_rawptr, rhs.required_rawptr)
    };

    bool pointers_match = true;
    for (const auto &item : pointers) {
      if ((item.first && !item.second) || (!item.first && item.second)) {
        // One or the either is not set when it should be
        pointers_match = false;
      }
      else if (item.first && item.second && !(*item.first == *item.second)) {
        // Both are set, but operator== fails on the instances
        pointers_match = false;
      }

      if (!pointers_match)
        break;
    }

    return containers_and_values_match && pointers_match;
  }
};

/**
 * @brief The purpose of this message is to show the serialization behavior of having a
 * struct member that has no RTTR-registered properties.
 */
struct COMMON_TEST_API
SimpleMessage {
  struct Payload {
    std::string member;

    RTTR_ENABLE();
  };

  std::string name;
  Payload payload;

  RTTR_ENABLE();
};

struct COMMON_TEST_API
MessageWithAnys {
  std::map<std::string, std::any> properties;

  RTTR_ENABLE();
};

struct COMMON_TEST_API
  MessageWithVectors {
  std::vector<int> v_int;
  std::vector<std::vector<int>> vv_int;
  std::vector<std::shared_ptr<SimpleMessage>> v_sptr;
  std::vector<SimpleMessage> v_obj;

  RTTR_ENABLE();
};

}; // lldc::testing