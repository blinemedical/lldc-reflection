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

#include "lldc-reflection/config.h"

#ifndef LLDC_REFLECTION_EXPORT
#define LLDC_REFLECTION_EXPORT extern
#endif

#ifndef LLDC_REFLECTION_IMPORT
#define LLDC_REFLECTION_IMPORT
#endif

#ifdef LLDC_REFLECTION_COMPILATION
#define LLDC_REFLECTION_API LLDC_REFLECTION_EXPORT
#else
#define LLDC_REFLECTION_API LLDC_REFLECTION_IMPORT
#endif