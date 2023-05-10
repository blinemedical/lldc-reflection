/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 *
 * This was modelled after the RapidJSON -based to_json example
 * found in the RTTR library.
 */

#include <string_view>
#include <json-glib/json-glib.h>

#include <lldc-rttr/converters/json-glib.h>
#include <lldc-rttr/metadata/metadata.h>
#include <lldc-rttr/exceptions/exceptions.h>

#include "private/associative-containers.h"

namespace AC = lldc::rttr::associative_containers;
namespace METADATA = lldc::rttr::metadata;

namespace lldc::json_utils {

static bool to_json_recursive(const ::rttr::instance &obj2, JsonObject *object);
static bool write_variant (const ::rttr::variant &var, JsonNode *node, bool optional = false);
static bool attempt_write_fundamental_type (const ::rttr::type &t, const ::rttr::variant &var, JsonNode *node, bool optional = false);
static bool write_array (const ::rttr::variant_sequential_view &view, JsonNode *node, bool optional = false);
static bool write_associative_container (const ::rttr::variant_associative_view &view, JsonNode *node, bool optional = false);

#define IS_OPTIONAL(p) (p.get_metadata(METADATA::OPTIONAL).is_valid())
#define TREAT_STRING_AS_OPTIONAL(s, optional) ((s.length() == 0 && optional))

#define IS_FUNDAMENTAL_TYPE(t) ((t.is_arithmetic() || t.is_enumeration() || (t == ::rttr::type::get<std::string>()) ))

static bool
attempt_write_fundamental_type (
  const ::rttr::type &t,
  const ::rttr::variant &var,
  JsonNode *node,
  bool optional)
{
  bool did_write = false;

  // Json Number
  if (t.is_arithmetic()) {
    if (t == ::rttr::type::get<bool>()) {
      json_node_init_boolean(node, var.to_bool());
      did_write = true;
    }
    else if (t == ::rttr::type::get<char>()) {
      // This char->bool is from the original and seems odd, to
      // munge the type like this.  Keeping it though
      json_node_init_boolean(node, var.to_bool());
      did_write = true;
    }
    else if (t == ::rttr::type::get<int>()) {
      json_node_init_int(node, var.to_int());
      did_write = true;
    }
    else if (t == ::rttr::type::get<uint64_t>()) {
      json_node_init_int(node, var.to_uint64());
      did_write = true;
    }
    else if (t == ::rttr::type::get<float>() || t == ::rttr::type::get<double>()) {
      json_node_init_double(node, var.to_double());
      did_write = true;
    }
  }
  // Enumeration as string
  else if (t.is_enumeration()) {
    // Attempt to serialize it as a string
    bool ok = false;
    auto result = var.to_string(&ok);

    if (ok && !TREAT_STRING_AS_OPTIONAL(result, optional)) {
      json_node_init_string (node, var.to_string().c_str());
      did_write = true;
    }
    else {
      // Attempt treating as a number
      auto value = var.to_uint64(&ok);
      if (ok)
        json_node_init_int(node, value);
      else
        json_node_init_object(node, nullptr);
    }
    did_write = true;
  }
  else if (t == ::rttr::type::get<std::string>()) {
    auto result = var.to_string();

    if (!TREAT_STRING_AS_OPTIONAL(result, optional)) {
      if (t.get_metadata(METADATA::BLOB)) {
        // Treat the string as JSON; store the serialized object into this node.
        GError* error = NULL;
        auto parsed = json_from_string(result.c_str(), &error);
        if (!error) {
          // get+init -> +1 ref count so when 'parsed' is unreffed, the count
          // goes to 1, not zero.
          auto parsed_obj = json_node_get_object(parsed);
          json_node_init_object(node, parsed_obj);
          json_node_unref(parsed);
          did_write = true;
        }
      }
      else {
        json_node_init_string(node, var.to_string().c_str());
        did_write = true;
      }
    }
  }

  return did_write;
}

static bool
write_array (const ::rttr::variant_sequential_view &view, JsonNode *node, bool optional)
{
  if (optional && view.get_size() == 0)
    return false; // Don't bother serializing.

  JsonArray *arr = json_array_new();

  for (const auto& item : view) {
    JsonNode *element = json_node_alloc();
    if (write_variant(item, element, optional)) {
      json_array_add_element(arr, element);
    }
  }

  json_node_init_array(node, arr);

  // Already filtered out "empty" + "optional" = skip; creating an
  // empty, required array is permissible.
  return true;
}

static bool
write_associative_container (const ::rttr::variant_associative_view &view, JsonNode *node, bool optional)
{
  if (optional && view.get_size() == 0)
    return false; // Don't bother serializing.

  JsonArray *arr = json_array_new();

  // From the original source code comments:
  //   "Dealing with keys = values containers like sets"
  // However the RTTR docs say this method returns TRUE if the
  // associative container is like a set, where it only contains
  // keys.
  if (view.is_key_only_type()) {
    for (auto& item : view) {
      JsonNode *element = json_node_alloc();
      if (write_variant(item.first, element)) {
        json_array_add_element(arr, element);
      }
    }
  }
  else {
    // Original is pretty clearly doing this:
    //   vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // [ {'key': <key>, 'value': <value>}, ... ]
    for (auto& item : view) {
      JsonNode *first = json_node_alloc();
      JsonNode *second = json_node_alloc();

      if (write_variant(item.first, first) && write_variant(item.second, second)) {
        JsonNode* element = json_node_alloc();
        JsonObject* element_obj = json_object_new();

        json_node_init_object(element, element_obj);

        json_object_set_member(element_obj, AC::KEY,
          first);
        json_object_set_member(element_obj, AC::VALUE,
          second);

        json_array_add_element(arr, element);
      }
      else {
        json_node_unref(first);
        json_node_unref(second);
      }
    }
  }

  // Inserting an empty associative container is okay because
  // the container itself is !optional.
  json_node_init_array(node, arr);
  return true;
}

static bool
write_variant (const ::rttr::variant &var, JsonNode *node, bool optional)
{
  bool did_write = false;

  // Deal with wrapped type.
  ::rttr::variant localVar = var;
  ::rttr::type varType = var.get_type();

  if (varType.is_wrapper()) {
    varType = varType.get_wrapped_type();
    localVar = localVar.extract_wrapped_value();
  }

  if (IS_FUNDAMENTAL_TYPE(varType)) {
    did_write = attempt_write_fundamental_type(varType, localVar, node, optional);
  }
  else if (var.is_sequential_container()) {
    did_write = write_array(var.create_sequential_view(), node, optional);
  }
  else if (var.is_associative_container()) {
    did_write = write_associative_container(var.create_associative_view(), node, optional);
  }
  else {
    // Not fundamental or a container -- treat as object.
    auto child_props = varType.get_properties();
    if (!child_props.empty()) {
      auto json_object = json_object_new();
      if (to_json_recursive(var, json_object)) {
        json_node_set_object(node, json_object);
        did_write = true;
      }
      else {
        json_object_unref(json_object);
      }
    }
    else {
      // Edge case encountered
      g_assert("Unknown edge case encountered");
    }
  }

  return did_write;
}

static bool
to_json_recursive(const ::rttr::instance &obj2, JsonObject *json_object)
{
  bool did_write = false;
  ::rttr::instance obj = obj2.get_type().get_raw_type().is_wrapper() ? obj2.get_wrapped_instance() : obj2;

  auto prop_list = obj.get_derived_type().get_properties();
  for (auto prop : prop_list)
  {
    const auto name = prop.get_name();
    ::rttr::variant prop_value = prop.get_value(obj);
    bool optional = IS_OPTIONAL(prop);

    if (!prop_value)
      continue; // cannot serialize, unable to retrieve value.
    if (prop.get_metadata(METADATA::NO_SERIALIZE))
      continue; // skip it.

    JsonNode *prop_node = json_node_alloc();
    if (write_variant(prop_value, prop_node, optional)) {
      did_write = true;
      json_object_set_member(json_object, name.data(), prop_node);
    }
    else {
      if (!optional)
        g_error ("failed to serialize %s\n", name.data());
      json_node_unref(prop_node);
    }
  }

  return did_write;
}

JsonNode*
to_json_node_glib (::rttr::instance rttr_obj) {
  JsonNode* root = NULL;

  if (rttr_obj.is_valid()) {
    auto json_object = json_object_new();
    if (to_json_recursive(rttr_obj, json_object)) {
      root = json_node_new(JSON_NODE_OBJECT);
      json_node_set_object(root, json_object);
    }
    else {
      json_object_unref(json_object);
    }
  }

  return root;
}

namespace json_glib {
  std::string
  to_json(::rttr::instance obj)
  {
    JsonNode* root = to_json_node_glib(obj);
    auto s = json_to_string (root, TRUE);
    std::string out(s);
    g_free(s);
    json_node_unref(root);
    return out;
  }
}; // json_glib

}; // lldc::rttr::converters