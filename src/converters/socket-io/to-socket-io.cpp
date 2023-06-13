/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 *
 * This is derived from the RTTR Json example app which utilized
 * rapidjson for its backend and the lldc_appsync variation which
 * utilized JsonGLIB for its incarnation.
 */

#include <lldc-reflection/converters/socket-io.h>
#include <lldc-reflection/metadata/metadata.h>
#include <lldc-reflection/exceptions/exceptions.h>

#include "private/associative-containers.h"
#include "private/metadata/metadata.h"
#include "private/type/type.h"

namespace AC = lldc::reflection::associative_containers;
namespace METADATA = lldc::reflection::metadata;
namespace TYPE = lldc::reflection::type;

using sio_object = std::map<std::string, ::sio::message::ptr>;
using sio_array = std::vector<::sio::message::ptr>;


namespace lldc::reflection::converters {

static bool to_socket_io_recursive(const ::rttr::instance &rttr_obj, sio_object &object);
static bool write_variant(const ::rttr::variant &var, ::sio::message::ptr &member, bool optional=false);
static bool attempt_write_fundamental_type (const ::rttr::type &t, const ::rttr::variant &var, ::sio::message::ptr &member, bool optional=false);
static bool write_array (const ::rttr::variant_sequential_view &view, ::sio::message::ptr &member, bool optional=false);
static bool write_associative_container (const ::rttr::variant_associative_view &view, ::sio::message::ptr &member, bool optional=false);

static bool
attempt_write_fundamental_type(
  const ::rttr::type &t,
  const ::rttr::variant &var,
  ::sio::message::ptr &member,
  bool optional)
{
  bool did_write = false;

  if (t.is_arithmetic()) {
    // Is a Number
    if (t == ::rttr::type::get<bool>()) {
      member = ::sio::bool_message::create(var.to_bool());
      did_write = true;
    }
    else if (t == ::rttr::type::get<char>()) {
      member = ::sio::string_message::create(var.to_string());
      did_write = true;
    }
    else if (t == ::rttr::type::get<int>()) {
      member = ::sio::int_message::create(var.to_int());
      did_write = true;
    }
    else if (t == ::rttr::type::get<int8_t>()) {
      member = ::sio::int_message::create(var.to_int8());
      did_write = true;
    }
    else if (t == ::rttr::type::get<int16_t>()) {
      member = ::sio::int_message::create(var.to_int16());
      did_write = true;
    }
    else if (t == ::rttr::type::get<int32_t>()) {
      member = ::sio::int_message::create(var.to_int32());
      did_write = true;
    }
    else if (t == ::rttr::type::get<int64_t>()) {
      member = ::sio::int_message::create(var.to_int64());
      did_write = true;
    }
    else if (t == ::rttr::type::get<uint8_t>()) {
      member = ::sio::int_message::create(static_cast<int64_t>(var.to_uint8()));
      did_write = true;
    }
    else if (t == ::rttr::type::get<uint16_t>()) {
      member = ::sio::int_message::create(static_cast<int64_t>(var.to_uint16()));
      did_write = true;
    }
    else if (t == ::rttr::type::get<uint32_t>()) {
      member = ::sio::int_message::create(static_cast<int64_t>(var.to_uint32()));
      did_write = true;
    }
    else if (t == ::rttr::type::get<uint64_t>()) {
      member = ::sio::int_message::create(static_cast<int64_t>(var.to_uint64()));
      did_write = true;
    }
    else if (t == ::rttr::type::get<float>() || t == ::rttr::type::get<double>()) {
      member = ::sio::double_message::create(var.to_double());
      did_write = true;
    }
  }
  else if (t.is_enumeration()) {
    // Enumeration as a string
    // Attempt to serialize it as a string
    bool ok = false;
    auto result = var.to_string(&ok);

    if (ok && !(optional && result.empty())) {
      member = ::sio::string_message::create(var.to_string());
      did_write = true;
    }
    else {
      // Attempt treating as a number
      auto value = var.to_int64(&ok);
      if (ok)
        member = ::sio::int_message::create(value);
      else
        member = ::sio::null_message::create();
    }
    did_write = true;
  }
  else if (t == ::rttr::type::get<std::string>()) {
    auto result = var.to_string();

    if (!(optional && result.empty())) {
      if (METADATA::is_blob(t)) {
        member = ::sio::binary_message::create(std::make_shared<std::string>(result));
      }
      else {
        member = ::sio::string_message::create(result);
      }
      did_write = true;
    }
  }

  return did_write;
}

static bool
write_array (
  const ::rttr::variant_sequential_view &view,
  ::sio::message::ptr &member,
  bool optional)
{
  if (optional && view.get_size() == 0)
    return false; // Don't bother serializing

  member = sio::array_message::create();

  for (const auto& item : view) {
    auto entry = ::sio::message::ptr();
    if (write_variant(item, entry, optional)) {
      member->get_vector().push_back(entry);
    }
  }
  return true;
}

static bool
write_associative_container (
  const ::rttr::variant_associative_view &view,
  ::sio::message::ptr &member,
  bool optional)
{
  if (optional && view.get_size() == 0)
    return false;

  auto array = sio::array_message::create();

  // From the original source code comments:
  //   "Dealing with keys = values containers like sets"
  // However the RTTR docs say this method returns TRUE if the
  // associative container is like a set, where it only contains
  // keys.
  if (view.is_key_only_type()) {
    for (auto& item : view) {
      ::sio::message::ptr element;
      if (write_variant(item.first, element)) {
        array->get_vector().push_back(element);
      }
    }
  }
  else {
    // Original is pretty clearly doing this:
    //   vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // [ {'key': <key>, 'value': <value>}, ... ]
    for (auto& item : view) {
      ::sio::message::ptr key;
      ::sio::message::ptr value;

      if (write_variant(item.first, key) && write_variant(item.second, value)) {
        auto obj = sio::object_message::create();
        obj->get_map()[AC::KEY] = key;
        obj->get_map()[AC::VALUE] = value;

        array->get_vector().push_back(obj);
      }
    }
  }

  // Even if it's empty, it's okay because we've established
  // it's !optional.
  member = array;
  return true;
}

static bool
write_variant(const ::rttr::variant &var, ::sio::message::ptr &member, bool optional)
{
  bool did_write = false;

  // Deal with wrapped types.
  ::rttr::variant localVar = var;
  ::rttr::type varType = var.get_type();
  if (varType.is_wrapper()) {
    varType = varType.get_wrapped_type();
    localVar = localVar.extract_wrapped_value();
  }

  // If the varType is holding a std::any, it needs to be unpacked.
  if (TYPE::is_any(varType)) {
    did_write = write_variant(TYPE::extract_any_value(localVar), member, optional);
  }
  else if (TYPE::is_fundamental(varType)) {
    did_write = attempt_write_fundamental_type(varType, localVar, member, optional);
  }
  else if (localVar.is_sequential_container()) {
    did_write = write_array(localVar.create_sequential_view(), member, optional);
  }
  else if (localVar.is_associative_container()) {
    did_write = write_associative_container(localVar.create_associative_view(), member, optional);
  }
  else {
    // Not fundamental or container -- treat as object.
    auto temp = ::sio::object_message::create();
    if (to_socket_io_recursive(localVar, temp->get_map())) {
      member.swap(temp);
      did_write = true;
    }
    else if (!optional) {
      // Source member is "empty" and required.
      // If it's a pointer-type, represent it as a nullptr
      // If not, represent it as an empty object.
      if (varType.is_pointer())
        temp = ::sio::null_message::create();
      member.swap(temp);
      did_write = true;
    }
  }

  return did_write;
}

static bool
to_socket_io_recursive(const ::rttr::instance &obj2, sio_object &object)
{
  bool did_write = false;
  ::rttr::instance obj = obj2.get_type().get_raw_type().is_wrapper() ? obj2.get_wrapped_instance() : obj2;

  auto prop_list = obj.get_derived_type().get_properties();
  for (auto prop : prop_list)
  {
    const auto name = prop.get_name();
    ::rttr::variant prop_value = prop.get_value(obj);
    bool matches_default = false;
    bool optional = METADATA::is_optional(prop, prop_value, &matches_default);

    if (METADATA::is_no_serialize(prop)) {
      continue; // skip it
    }

    if (optional && matches_default) {
      continue; // By implication, skip it.
    }

    if (optional && !prop_value) {
      continue; // null-like and it's optional; skip it.
    }

    ::sio::message::ptr member;
    if (write_variant(prop_value, member, optional)) {
      did_write = true;
      object[name.to_string()] = member;
    }
    else if (!optional) {
      // Failed write and not optional -> error condition
      throw exceptions::RequiredMemberSerializationFailure(name.to_string());
    }
  }

  return did_write;
}

::sio::message::ptr
to_socket_io (::rttr::instance object)
{
  ::sio::message::ptr out;
  out.reset();

  if (object.is_valid()) {
    auto temp = ::sio::object_message::create();
    if (to_socket_io_recursive(object, temp->get_map())) {
      out = temp;
    }
  }

  return std::move(out);
}

}; // lldc::reflection::converters