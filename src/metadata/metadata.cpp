/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 */

#include <lldc-reflection/metadata/metadata.h>

#ifndef LLDC_REFLECTION_METADATA_OPTIONAL
#define LLDC_REFLECTION_METADATA_OPTIONAL "OPTIONAL"
#endif

#ifndef LLDC_REFLECTION_METADATA_NO_SERIALIZE
#define LLDC_REFLECTION_METADATA_NO_SERIALIZE "NO_SERIALIZE"
#endif

#ifndef LLDC_REFLECTION_METADATA_BLOB
#define LLDC_REFLECTION_METADATA_BLOB "BLOB"
#endif

namespace lldc::reflection::metadata {
const char* const OPTIONAL = LLDC_REFLECTION_METADATA_OPTIONAL;
const char* const NO_SERIALIZE = LLDC_REFLECTION_METADATA_NO_SERIALIZE;
const char* const BLOB = LLDC_REFLECTION_METADATA_BLOB;
}; // lldc::reflection::metadata
