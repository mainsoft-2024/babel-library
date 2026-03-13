/**
 * @file easy_cbor_error.h
 * @brief Easy CBOR Converter - Error code definitions and message conversion
 * @author IoT SDK Team
 * @date 2026-01-18
 *
 * This module provides Easy CBOR specific error codes and message conversion.
 */

#ifndef EASY_CBOR_ERROR_H
#define EASY_CBOR_ERROR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Easy CBOR Error Codes - X-Macro Definition Table
 *
 * Each entry format: X(name, code, message)
 */
#define EASY_CBOR_ERROR_TABLE(X)                                                    \
    /* 1. General Errors (0x0001 - 0x00FF) */                                       \
    X(EASY_CBOR_OK, 0x0001, "Easy CBOR: Success")                                   \
    X(EASY_CBOR_ERR_NO_MEMORY, 0x0002, "Easy CBOR: Out of memory")                  \
    X(EASY_CBOR_ERR_ENCODE, 0x0003, "Easy CBOR: Encoding failed")                   \
    X(EASY_CBOR_ERR_DECODE, 0x0004, "Easy CBOR: Decoding failed")                   \
    X(EASY_CBOR_ERR_INVALID_FORMAT, 0x0005, "Easy CBOR: Invalid format")            \
    X(EASY_CBOR_ERR_NOT_INITIALIZED, 0x0006, "Easy CBOR: Not initialized")          \
    X(EASY_CBOR_ERR_ALREADY_INITIALIZED, 0x0007, "Easy CBOR: Already initialized")  \
    X(EASY_CBOR_ERR_INVALID_CONFIG, 0x0008, "Easy CBOR: Invalid configuration")     \
                                                                                    \
    /* 2. NULL Pointer Errors (0x0100 - 0x01FF) */                                  \
    X(EASY_CBOR_ERR_NULL_STRUCT, 0x0100, "Easy CBOR: NULL struct pointer")          \
    X(EASY_CBOR_ERR_NULL_SCHEMA, 0x0101, "Easy CBOR: NULL schema pointer")          \
    X(EASY_CBOR_ERR_NULL_BUFFER, 0x0102, "Easy CBOR: NULL buffer pointer")          \
    X(EASY_CBOR_ERR_NULL_SIZE, 0x0103, "Easy CBOR: NULL size pointer")              \
                                                                                    \
    /* 3. Schema Errors (0x0200 - 0x02FF) */                                        \
    X(EASY_CBOR_ERR_INVALID_SCHEMA, 0x0200, "Easy CBOR: Invalid schema structure")  \
    X(EASY_CBOR_ERR_NESTED_SCHEMA_NULL, 0x0201, "Easy CBOR: Nested schema is NULL") \
    X(EASY_CBOR_ERR_MAX_RECURSION, 0x0202, "Easy CBOR: Maximum recursion depth exceeded") \
                                                                                    \
    /* 4. Conversion Errors (0x0300 - 0x03FF) */                                    \
    X(EASY_CBOR_ERR_BUFFER_TOO_SMALL, 0x0300, "Easy CBOR: Output buffer too small")

    /**
     * @brief Easy CBOR Error Codes
     *
     * Enum generated from EASY_CBOR_ERROR_TABLE using X-Macro pattern.
     */
    typedef enum
    {
#define X(name, code, msg) name = code,
        EASY_CBOR_ERROR_TABLE(X)
#undef X
    } easy_cbor_error_code_t;

    /**
     * @brief Convert Easy CBOR error code to message
     *
     * @param error_code Error code to convert
     * @return const char* Pointer to error message string
     */
    const char *easy_cbor_error_message(uint16_t error_code);

#ifdef __cplusplus
}
#endif

#endif /* EASY_CBOR_ERROR_H */
