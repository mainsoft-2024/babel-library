/**
 * @file easy_cbor_macro.h
 * @brief Easy CBOR Converter - Schema generation macros
 * @author IoT SDK Team
 * @date 2025-10-29
 *
 * This module provides preprocessor macros for automatic schema generation
 * from struct definitions. These macros use C11 _Generic for type inference
 * and X-Macro patterns for maintainable code generation.
 */

#ifndef EASY_CBOR_MACRO_H
#define EASY_CBOR_MACRO_H

#include "easy_cbor.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* ========================================================================== */
/* Preprocessor Utility Macros                                                */
/* ========================================================================== */

/**
 * @brief Token concatenation macro
 * @internal
 *
 * Concatenates two preprocessor tokens with intermediate expansion.
 */
#define EASY_CBOR_PP_CAT(a, b) EASY_CBOR_PP_CAT_I(a, b)
#define EASY_CBOR_PP_CAT_I(a, b) a##b

/**
 * @brief Returns the second argument
 * @internal
 */
#define EASY_CBOR_PP_SECOND(a, b, ...) b

/**
 * @brief Probe pattern for detection macros
 * @internal
 */
#define EASY_CBOR_PP_PROBE(...) __VA_ARGS__, 1
#define EASY_CBOR_PP_IS_PROBE(...) EASY_CBOR_PP_SECOND(__VA_ARGS__, 0, ~)

/**
 * @brief Detects if preprocessor argument is literal 1
 * @internal
 *
 * Uses token pasting and probe pattern to detect the literal 1.
 * - EASY_CBOR_PP_IS_1(1) -> 1
 * - EASY_CBOR_PP_IS_1(other) -> 0
 */
#define EASY_CBOR_PP_PROBE_1 EASY_CBOR_PP_PROBE(~)
#define EASY_CBOR_PP_IS_1(x) EASY_CBOR_PP_IS_PROBE(EASY_CBOR_PP_CAT(EASY_CBOR_PP_PROBE_, x))

/**
 * @brief Conditional selection macro
 * @internal
 *
 * Usage: EASY_CBOR_PP_IF(condition)(true_branch, false_branch)
 * Expands to true_branch if condition is 1, false_branch if 0.
 */
#define EASY_CBOR_PP_IF_1(t, f) t
#define EASY_CBOR_PP_IF_0(t, f) f
#define EASY_CBOR_PP_IF(c) EASY_CBOR_PP_CAT(EASY_CBOR_PP_IF_, c)

/* ========================================================================== */
/* Field Declaration Macros                                                   */
/* ========================================================================== */

/**
 * @brief Declares a single field (non-array)
 * @internal
 *
 * @param type Field C type
 * @param name Field name
 * @param size Size parameter (ignored for single fields)
 */
#define DECLARE_FIELD_SINGLE(type, name, size) type name;

/**
 * @brief Declares an array field
 * @internal
 *
 * @param type Element C type
 * @param name Field name
 * @param size Array size
 */
#define DECLARE_FIELD_ARRAY(type, name, size) type name[size];

/**
 * @brief Declares a 2D array field
 * @internal
 *
 * @param type Element C type
 * @param name Field name
 * @param dim0 Outer dimension (rows)
 * @param dim1 Inner dimension (columns)
 */
#define DECLARE_FIELD_ARRAY_2D(type, name, dim0, dim1) type name[dim0][dim1];

/**
 * @brief Chooses between single and array field declaration
 * @internal
 *
 * Automatically detects if size is 1 (single field) or greater (array).
 * Used internally by struct generation macros.
 */
#define DECLARE_FIELD_HELPER(type, name, size)  \
    EASY_CBOR_PP_IF(EASY_CBOR_PP_IS_1(size))(   \
        DECLARE_FIELD_SINGLE(type, name, size), \
        DECLARE_FIELD_ARRAY(type, name, size))

/* ========================================================================== */
/* Type Inference Macro (C11 _Generic)                                        */
/* ========================================================================== */

/**
 * @brief Maps C types to Easy CBOR type enums
 *
 * Uses C11 _Generic for compile-time type inference. Primitive types map to
 * their corresponding EASY_CBOR_TYPE_* enums.
 *
 * User-defined types are resolved based on the presence of a nested schema:
 * - nested != NULL -> EASY_CBOR_TYPE_STRUCT (nested struct field)
 * - nested == NULL -> EASY_CBOR_TYPE_ENUM  (enum-like scalar encoded as integer)
 *
 * This allows enum types to be represented as simple scalar values while
 * still supporting nested struct fields via the same macro system.
 *
 * @param type   C type to infer
 * @param nested Nested schema pointer (may be NULL)
 * @return Corresponding easy_cbor_type_t enum value
 *
 * Supported primitive mappings:
 * - int8_t, int16_t, int32_t -> EASY_CBOR_TYPE_INT8/16/32
 * - uint8_t, uint16_t, uint32_t -> EASY_CBOR_TYPE_UINT8/16/32
 * - float -> EASY_CBOR_TYPE_FLOAT
 * - double -> EASY_CBOR_TYPE_DOUBLE
 * - _Bool -> EASY_CBOR_TYPE_BOOL
 * - char -> EASY_CBOR_TYPE_STRING
 */
#define GET_TYPE(type, nested)              \
    _Generic(((type){0}),                   \
        int8_t: EASY_CBOR_TYPE_INT8,        \
        int16_t: EASY_CBOR_TYPE_INT16,      \
        int32_t: EASY_CBOR_TYPE_INT32,      \
        uint8_t: EASY_CBOR_TYPE_UINT8,      \
        uint16_t: EASY_CBOR_TYPE_UINT16,    \
        uint32_t: EASY_CBOR_TYPE_UINT32,    \
        float: EASY_CBOR_TYPE_FLOAT,        \
        double: EASY_CBOR_TYPE_DOUBLE,      \
        _Bool: EASY_CBOR_TYPE_BOOL,         \
        char: EASY_CBOR_TYPE_STRING,        \
        default: ((nested) == NULL          \
                      ? EASY_CBOR_TYPE_ENUM \
                      : EASY_CBOR_TYPE_STRUCT))

/* ========================================================================== */
/* X-Macro Based Struct and Schema Generation                                 */
/* ========================================================================== */

/**
 * @brief X-Macro framework for automatic struct and schema generation
 *
 * This macro system uses the X-Macro (higher-order macro) pattern to generate
 * both struct definitions and their corresponding schema metadata from a single
 * field definition list.
 *
 * Usage:
 * 1. Define a field list macro with signature: FIELDS(APPLY, CTX)
 * 2. Each field: APPLY(ctype, fname, array_size, nested_schema_ptr, CTX)
 * 3. Use generation macros to create struct and schema
 *
 * Example:
 * @code
 *   #define SENSOR_DATA_FIELDS(APPLY, CTX)               \
 *       APPLY(int8_t, temp_celsius, 1, NULL, CTX)        \
 *       APPLY(char,   location,      32, NULL, CTX)
 *
 *   EASY_CBOR_DECLARE_STRUCT_AND_SCHEMA(SENSOR_DATA_FIELDS, sensor_data_t, sensor_data_schema);
 * @endcode
 *
 * This generates:
 * - typedef struct sensor_data_t { int8_t temp_celsius; char location[32]; };
 * - static const easy_cbor_struct_schema_t sensor_data_schema = { ... };
 */

/**
 * @brief Field applicator for struct declaration
 * @internal
 */
#define EASY_CBOR__APPLY_DECLARE_FIELD(ctype, fname, array_size, nested, CTX) \
    DECLARE_FIELD_HELPER(ctype, fname, array_size)

/**
 * @brief Field applicator for 2D array declaration
 * @internal
 *
 * This applicator is used via the EASY_CBOR_FIELD_2D helper macro inside
 * a FIELDS definition. It declares a true C 2D array:
 *
 *   EASY_CBOR_FIELD_2D(APPLY, CTX, uint8_t, matrix, 4, 8, NULL)
 *
 * expands (for struct declaration) to:
 *
 *   uint8_t matrix[4][8];
 */
#define EASY_CBOR__APPLY_DECLARE_FIELD_2D(ctype, fname, dim0, dim1, nested, CTX) \
    DECLARE_FIELD_ARRAY_2D(ctype, fname, dim0, dim1)

/**
 * @brief Field applicator for schema generation
 * @internal
 *
 * @param STRUCT_TYPE Passed as CTX parameter to calculate field offsets
 */
#define EASY_CBOR__APPLY_SCHEMA_FIELD(ctype, fname, array_size, nested, STRUCT_TYPE) \
    {                                                                                \
        #fname,                                                                      \
        GET_TYPE(ctype, nested),                                                     \
        offsetof(STRUCT_TYPE, fname),                                                \
        sizeof(ctype),                                                               \
        (size_t)(array_size),                                                        \
        (nested),                                                                    \
        1U,                                                                          \
        (size_t)(array_size),                                                        \
        1U},

/**
 * @brief Field applicator for 2D array schema generation
 * @internal
 *
 * For a C declaration like:
 *   uint8_t matrix[ROWS][COLS];
 *
 * the generated schema metadata is:
 *   - type       : GET_TYPE(uint8_t) == EASY_CBOR_TYPE_UINT8
 *   - size       : sizeof(uint8_t)
 *   - array_size : ROWS * COLS
 *   - dimensions : 2
 *   - dim0       : ROWS
 *   - dim1       : COLS
 */
#define EASY_CBOR__APPLY_SCHEMA_FIELD_2D(ctype, fname, dim0, dim1, nested, STRUCT_TYPE) \
    {                                                                                   \
        #fname,                                                                         \
        GET_TYPE(ctype, nested),                                                        \
        offsetof(STRUCT_TYPE, fname),                                                   \
        sizeof(ctype),                                                                  \
        (size_t)((dim0) * (dim1)),                                                      \
        (nested),                                                                       \
        2U,                                                                             \
        (size_t)(dim0),                                                                 \
        (size_t)(dim1)},

/**
 * @brief Helper macro for 1D fields inside FIELDS definitions
 *
 * Usage example:
 *   #define MY_FIELDS(APPLY, CTX)                                      \
 *       EASY_CBOR_FIELD(uint8_t, flags, 8, NULL, APPLY, CTX)           \
 *       EASY_CBOR_FIELD_2D(uint8_t, matrix, 4, 8, NULL, APPLY, CTX)
 */
#define EASY_CBOR_FIELD(ctype, fname, array_size, nested, APPLY, CTX) \
    APPLY(ctype, fname, array_size, nested, CTX)

/**
 * @brief Helper macro for 2D fields inside FIELDS definitions
 *
 * This macro relies on the applicator providing a corresponding
 * *_2D variant (for example EASY_CBOR__APPLY_DECLARE_FIELD_2D) by using
 * token concatenation on the APPLY parameter.
 */
#define EASY_CBOR_FIELD_2D(ctype, fname, dim0, dim1, nested, APPLY, CTX) \
    EASY_CBOR_PP_CAT(APPLY, _2D)(ctype, fname, dim0, dim1, nested, CTX)

/**
 * @brief Declares only the struct type
 *
 * @param STRUCT_TYPE Name of the struct type to declare
 * @param FIELDS Field list macro (signature: FIELDS(APPLY, CTX))
 */
#define EASY_CBOR_DECLARE_STRUCT(STRUCT_TYPE, FIELDS) \
    typedef struct                                    \
    {                                                 \
        FIELDS(EASY_CBOR__APPLY_DECLARE_FIELD, _)     \
    } STRUCT_TYPE;

/**
 * @brief Defines only the schema variable
 *
 * @param STRUCT_TYPE Name of the struct type (must be already declared)
 * @param FIELDS Field list macro (signature: FIELDS(APPLY, CTX))
 * @param SCHEMA_VAR Name of the schema variable to define
 */
#define EASY_CBOR_DEFINE_SCHEMA(STRUCT_TYPE, FIELDS, SCHEMA_VAR)                         \
    static const easy_cbor_field_schema_t STRUCT_TYPE##_fields[] = {                     \
        FIELDS(EASY_CBOR__APPLY_SCHEMA_FIELD, STRUCT_TYPE)};                             \
    static const easy_cbor_struct_schema_t SCHEMA_VAR = {                                \
        .fields = STRUCT_TYPE##_fields,                                                  \
        .field_count = sizeof(STRUCT_TYPE##_fields) / sizeof((STRUCT_TYPE##_fields)[0]), \
        .struct_size = sizeof(STRUCT_TYPE)};

/**
 * @brief Declares struct and defines schema in one macro
 *
 * @param FIELDS Field list macro (signature: FIELDS(APPLY, CTX))
 * @param STRUCT_TYPE Name of the struct type to declare
 * @param SCHEMA_VAR Name of the schema variable to define
 */
#define EASY_CBOR_DECLARE_STRUCT_AND_SCHEMA(FIELDS, STRUCT_TYPE, SCHEMA_VAR) \
    EASY_CBOR_DECLARE_STRUCT(STRUCT_TYPE, FIELDS)                            \
    EASY_CBOR_DEFINE_SCHEMA(STRUCT_TYPE, FIELDS, SCHEMA_VAR)

#endif /* EASY_CBOR_MACRO_H */
