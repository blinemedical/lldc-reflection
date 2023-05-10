/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 */
#pragma once

#include <lldc-reflection/api.h>
#include <lldc-reflection/registration.h>
#include <json-glib/json-glib.h>

namespace lldc::reflection::converters {
  LLDC_REFLECTION_API
  JsonNode* to_json_glib (::rttr::instance obj);

  LLDC_REFLECTION_API
  bool from_json_glib (JsonNode *node, ::rttr::instance obj);

  namespace json_glib {
    LLDC_REFLECTION_API
    std::string to_json (::rttr::instance obj);

    LLDC_REFLECTION_API
    bool from_json (const std::string &json_str, ::rttr::instance obj);
  };

}; // lldc::reflection::converters