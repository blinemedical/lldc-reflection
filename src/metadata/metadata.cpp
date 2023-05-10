/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 */

#include <lldc-rttr/metadata/metadata.h>

#ifndef LLDC_RTTR_METADATA_OPTIONAL
#define LLDC_RTTR_METADATA_OPTIONAL "OPTIONAL"
#endif

#ifndef LLDC_RTTR_METADATA_NO_SERIALIZE
#define LLDC_RTTR_METADATA_NO_SERIALIZE "NO_SERIALIZE"
#endif

#ifndef LLDC_RTTR_METADATA_BLOB
#define LLDC_RTTR_METADATA_BLOB "BLOB"
#endif

namespace lldc::rttr::metadata {
const char* const OPTIONAL = LLDC_RTTR_METADATA_OPTIONAL;
const char* const NO_SERIALIZE = LLDC_RTTR_METADATA_NO_SERIALIZE;
const char* const BLOB = LLDC_RTTR_METADATA_BLOB;
}; // lldc::rttr::metadata
