/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 *
 * This header is not to be directly included; it's explicitly for
 * configuring the IMPORT and EXPORT directives relative to the
 * generated header file so that API import/export declarations
 * function in Windows and have no impact in Linux or macOS.
 */
#pragma once

#include "lldc-rttr/config.h"

#if !defined(__LLDC_RTTR_INSIDE__) && !defined(LLDC_RTTR_COMPILATION)
#error "Do not include api.h directly."
#endif

#ifndef LLDC_RTTR_EXPORT
#define LLDC_RTTR_EXPORT extern
#endif

#ifndef LLDC_RTTR_IMPORT
#define LLDC_RTTR_IMPORT
#endif

#ifdef LLDC_RTTR_COMPILATION
#define LLDC_RTTR_API LLDC_RTTR_EXPORT
#else
#define LLDC_RTTR_API LLDC_RTTR_IMPORT
#endif