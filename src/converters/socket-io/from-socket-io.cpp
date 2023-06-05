/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 *
 * This is derived from the RTTR RapidJson example app and
 * the JsonGLIB variation (also inspired by that app) found
 * here in this library.
 */

#include <iostream>
#include <format>

#include <lldc-reflection/converters/socket-io.h>
#include <lldc-reflection/metadata/metadata.h>
#include <lldc-reflection/exceptions/exceptions.h>

#include "private/associative-containers.h"
#include "private/metadata/metadata.h"
#include "private/type/type.h"

namespace AC = lldc::reflection::associative_containers;
namespace METADATA = lldc::reflection::metadata;
namespace EXCEPTIONS = lldc::reflection::exceptions;
namespace TYPE = lldc::reflection::type;

namespace lldc::reflection::converters {

using sio_object = std::map<std::string, ::sio::message::ptr>;
using sio_array = std::vector<::sio::message::ptr>;

static void from_socket_io_recursively (const sio_object &message, ::rttr::instance obj2);
static void write_array_recursively (const sio_array &array, ::rttr::variant_sequential_view &view);
static void write_associative_view_recursively (const sio_array &array, ::rttr::variant_associative_view &view);
static ::rttr::variant extract_basic_types (const ::sio::message &message, const ::rttr::type &t);
static ::rttr::variant extract_value (const ::sio::message &message, const ::rttr::type &t);

static inline bool is_a (const ::sio::message &ref, ::sio::message::flag flag) {
  return (ref.get_flag() == flag);
}

static inline bool is_an_array (const ::sio::message &ref) {
  return is_a(ref, ::sio::message::flag_array);
}

static inline bool is_a_basic_type (const ::sio::message &ref) {
  return (
    is_a(ref, ::sio::message::flag_double) ||
    is_a(ref, ::sio::message::flag_null) ||
    is_a(ref, ::sio::message::flag_integer) ||
    is_a(ref, ::sio::message::flag_string)
  );
}

static inline bool is_an_object (const ::sio::message &ref) {
  return is_a(ref, ::sio::message::flag_object);
}

static void
write_array_recursively (const sio_array &array, ::rttr::variant_sequential_view &view)
{
  const ::rttr::type array_value_type = view.get_rank_type(1);

  view.set_size(array.size());
  for (size_t i = 0; i < array.size(); i++) {
    auto element = array.at(i);

    if (is_an_array(*element)) {
      auto sub_array_view = view.get_value(i).create_sequential_view();
      write_array_recursively(element->get_vector(), sub_array_view);
    }
    else if (is_an_object(*element)) {
      ::rttr::variant var_tmp = view.get_value(i);
      ::rttr::variant wrapped_var = var_tmp.extract_wrapped_value();
      from_socket_io_recursively (element->get_map(), wrapped_var);
      view.set_value (i, wrapped_var);
    }
    else if (is_a_basic_type(*element)) {
      ::rttr::variant extracted_value = extract_basic_types(*element, array_value_type);
      if (extracted_value.convert(array_value_type))
        view.set_value(i, extracted_value);
    }
  }
}

static void
write_associative_view_recursively (const sio_array &array, ::rttr::variant_associative_view &view)
{
  for (size_t i = 0; i < array.size(); i++) {
    auto element = array.at(i);

    if (is_an_object(*element)) {
      // treat as: { 'key': <key>, 'value': <value> } view
      auto element_key = element->get_map()[AC::KEY];
      auto element_value = element->get_map()[AC::VALUE];

      if (element_key.get() && element_value.get()) {
        auto key_var = extract_value(*element_key, view.get_key_type());
        auto value_var = extract_value(*element_value, view.get_value_type());

        if (key_var && value_var)
          view.insert(key_var, value_var);
      }
    }
    else {
      // a "key-only" associative view (??)
      ::rttr::variant extracted_value = extract_basic_types(*element, view.get_key_type());
      if (extracted_value && extracted_value.convert(view.get_key_type()))
        view.insert(extracted_value);
    }
  }
}

static ::rttr::variant
extract_basic_types (const ::sio::message &message, const ::rttr::type &t)
{
  switch (message.get_flag()) {
    // Big fall-through to default for data marshalling
    // basic types.
    case ::sio::message::flag_boolean: {
      auto ref = message.get_bool();
      if (TYPE::is_any(t))
        return std::any(ref);
      return ref;
    }
    case ::sio::message::flag_double: {
      auto ref = message.get_double();
      if (TYPE::is_any(t))
        return std::any(ref);
      return ref;
    }
    case ::sio::message::flag_integer: {
      auto temp = message.get_int();
      if (t == ::rttr::type::get<uint8_t>()) {
        return static_cast<uint8_t> (temp);
      }
      else if (t == ::rttr::type::get<uint16_t>()) {
        return static_cast<uint16_t> (temp);
      }
      else if (t == ::rttr::type::get<uint32_t>()) {
        return static_cast<uint32_t> (temp);
      }
      else if (t == ::rttr::type::get<uint64_t>()) {
        return static_cast<uint64_t> (temp);
      }
      else if (TYPE::is_any(t)) {
        return std::any(temp);
      }
      return temp;
    }
    case ::sio::message::flag_string: {
      auto ref = message.get_string();
      if (TYPE::is_any(t))
        return std::any(ref);
      return ref;
    }
    default:
      break;
  }
  return ::rttr::variant();
}

static ::rttr::variant
extract_value (const ::sio::message &message, const ::rttr::type &t)
{
  ::rttr::variant extracted_value = extract_basic_types(message, t);
  const bool could_convert = extracted_value.can_convert(t);

  if (!could_convert) {
    if (is_an_object(message)) {
      ::rttr::constructor ctor = t.get_constructor();
      for (auto &item : t.get_constructors()) {
        if (item.get_instantiated_type() == t) {
          ctor = item;
          break;
        }
      }

      extracted_value = ctor.invoke();
      from_socket_io_recursively (message.get_map(), extracted_value);
    }
  }

  return extracted_value;
}

static void
from_socket_io_recursively (const sio_object &message, ::rttr::instance obj2)
{
  ::rttr::instance obj = obj2.get_type().get_raw_type().is_wrapper() ? obj2.get_wrapped_instance() : obj2;
  const auto prop_list = obj.get_derived_type().get_properties();

  for (auto prop : prop_list)
  {
    auto name = prop.get_name().to_string();
    auto optional = METADATA::is_optional(prop, nullptr);
    if (message.end() == message.find(name)) {
      if (optional)
        continue; // Okay to skip restoration
      throw EXCEPTIONS::RequiredMemberSerializationFailure(name);
    }
    auto member = message.at(name);
    auto member_flag = (member) ? member->get_flag() : ::sio::message::flag_null;

    auto const value_t = prop.get_type();
    ::rttr::variant var;

    switch (member_flag) {
      case ::sio::message::flag_array: {
        ::rttr::type local_value_t = value_t;
        if (value_t.is_wrapper())
          local_value_t = value_t.get_wrapped_type();

        if (local_value_t.is_sequential_container()) {
          var = prop.get_value(obj);
          auto view = var.create_sequential_view();
          write_array_recursively(member->get_vector(), view);
        }
        else if (local_value_t.is_associative_container()) {
          var = prop.get_value(obj);
          auto view = var.create_associative_view();
          write_associative_view_recursively(member->get_vector(), view);
        }
        else if (METADATA::is_blob(local_value_t)) {
          auto blob = member->get_binary();
          if (blob.get())
            var = std::string(blob.get()->c_str());
        }
        prop.set_value(obj, var);
        break;
      }
      case ::sio::message::flag_object: {
        if (METADATA::is_blob(value_t)) {
          auto blob = member->get_binary();
          if (blob.get())
            var = std::string(blob.get()->c_str());
        }
        else {
          ::rttr::type local_value_t = value_t;
          if (value_t.is_wrapper())
            local_value_t = value_t.get_wrapped_type();

          var = prop.get_value(obj);
          if (local_value_t.is_pointer()) {
            auto ctor = local_value_t.get_constructor();
            for (auto& item : local_value_t.get_raw_type().get_constructors()) {
              if (item.get_instantiated_type() == value_t) {
                ctor = item;
                break;
              }
            }
            if (ctor.is_valid())
              var = ctor.invoke();
          }

          from_socket_io_recursively(member->get_map(), var);
          prop.set_value(obj, var);
        }
        break;
      }
      case ::sio::message::flag_null: {
        prop.set_value(obj, nullptr);
        break;
      }
      default:
        // REMARK: this conversion only works with "const type".
        var = extract_basic_types(*member, value_t);
        if (var.convert(value_t))
          prop.set_value(obj, var);
    }
  }
}


bool
from_socket_io (const ::sio::message::ptr message, ::rttr::instance object)
{
  bool success = false;

  if (message && message->get_flag() == ::sio::message::flag_object) {
    try {
      from_socket_io_recursively (message->get_map(), object);
      success = true;
    }
    catch (...) {
      // do nothing here; returning false.
      success = false;
    }
  }

  return success;
}

}; // lldc::reflection::converters
