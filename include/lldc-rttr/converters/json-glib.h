/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 */
#pragma once

#include <lldc-rttr/api.h>
#include <lldc-rttr/exceptions/exceptions.h>
#include <json-glib/json-glib.h>
#include <rttr/registration>

namespace lldc::rttr::converters {
  LLDC_RTTR_API
  JsonNode* to_json_glib (::rttr::instance obj);

  LLDC_RTTR_API
  bool from_json_glib (JsonNode *node, ::rttr::instance obj);

  namespace json_glib {
    LLDC_RTTR_API
    std::string to_json (::rttr::instance obj);

    LLDC_RTTR_API
    bool from_json (const std::string &json_str, ::rttr::instance obj);
  };

}; // lldc::json_utils