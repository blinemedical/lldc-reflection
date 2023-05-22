/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 *
 * This was modelled after the RapidJSON -based from_json example found
 * in the RTTR library.
 */

#include <lldc-reflection/converters/json-glib.h>
#include <lldc-reflection/metadata/metadata.h>
#include <lldc-reflection/exceptions/exceptions.h>

#include "private/associative-containers.h"
#include "private/metadata/metadata.h"

namespace AC = lldc::reflection::associative_containers;
namespace METADATA = lldc::reflection::metadata;
namespace EXCEPTIONS = lldc::reflection::exceptions;

namespace lldc::reflection::converters {

static void from_json_recursively (JsonObject *json_obj, ::rttr::instance obj2);
static void write_array_recursively (JsonArray *arr, ::rttr::variant_sequential_view &view);
static void write_associative_view_recursively (JsonArray *arr, ::rttr::variant_associative_view &view);
static ::rttr::variant extract_basic_types (JsonNode *json_value, const ::rttr::type &t);
static ::rttr::variant extract_value (JsonNode *json_value, const ::rttr::type &t);


static void
write_array_recursively (JsonArray *json_array, ::rttr::variant_sequential_view &view)
{
  guint json_array_size = json_array_get_length(json_array);
  const ::rttr::type array_value_type = view.get_rank_type(1);

  view.set_size(json_array_size);
  for (guint i = 0; i < json_array_size; i++) {
    auto element = json_array_get_element(json_array, i);
    if (JSON_NODE_HOLDS_ARRAY(element)) {
      auto sub_array_view = view.get_value(i).create_sequential_view();
      write_array_recursively(json_node_get_array(element), sub_array_view);
    }
    else if (JSON_NODE_HOLDS_OBJECT(element)) {
      ::rttr::variant var_tmp = view.get_value(i);
      ::rttr::variant wrapped_var = var_tmp.extract_wrapped_value();
      from_json_recursively (json_node_get_object(element), wrapped_var);
      view.set_value (i, wrapped_var);
    }
    else if (JSON_NODE_HOLDS_VALUE (element)) {
      ::rttr::variant extracted_value = extract_basic_types (element, array_value_type);
      if (extracted_value.convert(array_value_type))
        view.set_value(i, extracted_value);
    }
  }
}

static void
write_associative_view_recursively (JsonArray *json_array, ::rttr::variant_associative_view &view)
{
  guint json_array_size = json_array_get_length(json_array);
  for (guint i = 0; i < json_array_size; i++) {
    auto element = json_array_get_element (json_array, i);

    if (JSON_NODE_HOLDS_OBJECT(element)) {
      // Treat as: { 'key': <key>, 'value': <value>} view.
      auto element_obj = json_node_dup_object (element);
      auto key_mbr = json_object_get_member (element_obj, AC::KEY);
      auto value_mbr = json_object_get_member (element_obj, AC::VALUE);

      if (key_mbr && value_mbr) {
        auto key_var = extract_value (key_mbr, view.get_key_type());
        auto value_var = extract_value (value_mbr, view.get_value_type());

        if (key_var && value_var)
          view.insert(key_var, value_var);
      }

      json_object_unref(element_obj);
    }
    else {
      // a "key-only" associative view (??)
      ::rttr::variant extracted_value = extract_basic_types (element, view.get_key_type());
      if (extracted_value && extracted_value.convert(view.get_key_type()))
        view.insert(extracted_value);
    }
  }
}

static ::rttr::variant
extract_basic_types (JsonNode *json_value, const ::rttr::type &t)
{
  switch (json_node_get_value_type (json_value)) {
    case G_TYPE_CHAR:
      return (char) *json_node_get_string(json_value);

    case G_TYPE_STRING:
      return std::string (json_node_get_string(json_value));

    case G_TYPE_BOOLEAN:
      // Because the returned 'boolean' is typedef'd to int, we'll assert it
      // to boolean here just for the sake of it.
      return (json_node_get_boolean (json_value));

    // JsonGLIB does not store these types.  Keep this commented
    // for future reference.  They're stored as int/int64
    // case G_TYPE_UINT:
    // case G_TYPE_UINT64:

    case G_TYPE_INT:
    case G_TYPE_INT64: {
      auto temp = json_node_get_int(json_value);
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
      return temp;
    }

    case G_TYPE_FLOAT:
    case G_TYPE_DOUBLE:
      return json_node_get_double (json_value);

    default:
      break;
  }
  return ::rttr::variant();
}

static ::rttr::variant
extract_value (JsonNode *json_value, const ::rttr::type &t)
{
  ::rttr::variant extracted_value = extract_basic_types (json_value, t);
  const bool could_convert = extracted_value.can_convert(t);

  if (!could_convert) {
    if (JSON_NODE_HOLDS_OBJECT(json_value)) {
      ::rttr::constructor ctor = t.get_constructor();
      for (auto &item : t.get_constructors()) {
        if (item.get_instantiated_type() == t)
          ctor = item;
      }
      extracted_value = ctor.invoke();
      from_json_recursively (json_node_get_object(json_value), extracted_value);
    }
  }

  return extracted_value;
}

static void
from_json_recursively (JsonObject *json_obj, ::rttr::instance obj2)
{
  ::rttr::instance obj = obj2.get_type().get_raw_type().is_wrapper() ? obj2.get_wrapped_instance() : obj2;
  const auto prop_list = obj.get_derived_type().get_properties();

  for (auto prop : prop_list) {
    auto name = prop.get_name().data();
    auto optional = METADATA::is_optional(prop, nullptr);
    JsonNode *member = json_object_get_member(json_obj, name);
    if (!member) {
      if (optional)
        continue; // not found, okay to skip
      throw EXCEPTIONS::RequiredMemberSerializationFailure(name);
    }

    auto const value_t = prop.get_type();
    ::rttr::variant var;
    switch (json_node_get_node_type(member)) {
      case JSON_NODE_ARRAY:
      {
        if (value_t.is_sequential_container()) {
          auto json_array = json_node_get_array(member);
          var = prop.get_value(obj);
          auto view = var.create_sequential_view();
          write_array_recursively(json_array, view);
        }
        else if (value_t.is_associative_container()) {
          auto json_array = json_node_get_array(member);
          var = prop.get_value(obj);
          auto view = var.create_associative_view();
          write_associative_view_recursively(json_array, view);
        }
        else if (METADATA::is_blob(value_t)) {
          auto json_str = json_to_string(member, TRUE);
          if (json_str) {
            var = std::string(json_str);
            g_free(json_str);
          }
        }

        prop.set_value(obj, var);
        break;
      }
      case JSON_NODE_OBJECT:
      {
        if (METADATA::is_blob(value_t)) {
          auto json_str = json_to_string(member, TRUE);
          if (json_str) {
            var = std::string(json_str);
            g_free(json_str);
          }
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

          from_json_recursively(json_node_get_object(member), var);
          prop.set_value(obj, var);
        }
        break;
      }
      case JSON_NODE_NULL:
      {
        prop.set_value(obj, nullptr);
        break;
      }
      default:
      {
        var = extract_basic_types (member, value_t);
        // REMARK: conversion only works with "const type".
        if (var.convert(value_t))
          prop.set_value(obj, var);
        break;
      }
    }
  }
}

bool
from_json_glib (JsonNode *node, ::rttr::instance obj)
{
  // similar to to_json, we only assume the top-level
  // node contains a root object with properties in it:
  // {
  //   "first_property": ...,
  //   "second_property": ...,
  //   etc.
  // }
  bool success = false;

  if (node && JSON_NODE_HOLDS_OBJECT(node)) {
    json_node_ref(node);
    JsonObject *root = json_node_dup_object(node);
    try {
      from_json_recursively(root, obj);
      success = true;
    }
    catch (...) {
      // do nothing here; returning false
      success = false;
    }
    json_object_unref(root);
    json_node_unref(node);
  }

  return success;
}

namespace json_glib {
  bool
  from_json (const std::string &json_str, ::rttr::instance obj)
  {
    GError* error = NULL;
    auto node = json_from_string(json_str.c_str(), &error);

    if (!error)
      return from_json_glib(node, obj);
    return false;
  }
}; // json_glib

}; // lldc::reflection::converters