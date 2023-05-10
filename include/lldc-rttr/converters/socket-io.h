/**
 * Copyright 2023 Laerdal Labs, DC
 *   Author: Thomas Goodwin <thomas.goodwin@laerdal.com>
 */
#pragma once

#include <lldc-rttr/api.h>
#include <lldc-rttr/exceptions/exceptions.h>
#include <sio_message.h>
#include <rttr/registration>

namespace lldc::rttr::converters {

/**
 * @brief Convert the #object to an sio::message::ptr instance
 *
 * @param object the registered reference object
 * @return sio::message::ptr the resulting message, if successfully converted
 */
LLDC_RTTR_API
::sio::message::ptr to_socket_io (::rttr::instance object);

/**
 * @brief Convert the #message to its RTTR registered #object
 *
 * @param message the reference message
 * @param object the resulting parsed object
 * @return true if parsing was successful
 * @return false if prasing wass unsuccessful
 */
LLDC_RTTR_API
bool from_socket_io (const ::sio::message::ptr message, ::rttr::instance object);

}; // lldc::rttr::converters