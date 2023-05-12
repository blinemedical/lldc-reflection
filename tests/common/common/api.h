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

#if !defined(__COMMON_TEST_INSIDE__) && !defined(COMMON_TEST_COMPILATION)
#error "Only <common/common.h> can be included directly."
#endif

#include "common/config.h"

#ifndef COMMON_TEST_EXPORT
#define COMMON_TEST_EXPORT extern
#endif

#ifndef COMMON_TEST_IMPORT
#define COMMON_TEST_IMPORT
#endif

#ifdef COMMON_TEST_COMPILATION
#define COMMON_TEST_API COMMON_TEST_EXPORT
#else
#define COMMON_TEST_API COMMON_TEST_IMPORT
#endif