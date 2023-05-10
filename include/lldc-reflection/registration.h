/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 *
 * When defining the properties, metadata, etc. for structures
 * in an object file, include this header and in the top-level
 * namespace, use the macro:
 *
 *   // if using dynamic libraries
 *   RTTR_PLUGIN_REGISTRATION {
 *     ::rttr::registration::class_<type>("type-name")
 *       .property(...) (
 *          // metadata
 *       )
 *       .property(...);
 *   };
 *
 *   // If using static libraries, there can be only one use
 *   // of this macro in the entire binary, and
 *   // RTTR_PLUGIN_REGISTRATION is not to be used, according
 *   // to the RTTR documentation:
 *   RTTR_REGISTRATION {
 *     // same contents as above.
 *   };
 */
#pragma once

#include <rttr/registration>
#include <lldc-reflection/metadata/metadata.h>
#include <lldc-reflection/exceptions/exceptions.h>
