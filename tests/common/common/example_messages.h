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

namespace lldc::testing {

enum class COMMON_TEST_API
Subject {
  not_set = -1,
  first_message,
  second_message
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
  char        some_char;
  bool        some_bool;
  float       some_float;
  double      some_double;
  uint64_t    some_uint64;
  uint32_t    some_uint32;
  uint16_t    some_uint16;
  uint8_t     some_uint8;
  int64_t     some_int64;
  int32_t     some_int32;
  int16_t     some_int16;
  int8_t      some_int8;

  RTTR_ENABLE(ApiMessage);
};

}; // lldc::testing