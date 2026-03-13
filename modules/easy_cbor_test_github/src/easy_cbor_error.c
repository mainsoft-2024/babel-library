/**
 * @file easy_cbor_error.c
 * @brief Easy CBOR Converter - Error message conversion
 * @author IoT SDK Team
 * @date 2026-01-18
 */

#include "easy_cbor_error.h"

const char *easy_cbor_error_message(uint16_t error_code)
{
    switch (error_code)
    {
#define X(name, code, msg) \
    case name:             \
        return msg;
        EASY_CBOR_ERROR_TABLE(X)
#undef X
    default:
        return "Unknown Easy CBOR error";
    }
}
