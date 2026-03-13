/**
 * @file easy_cbor.h
 * @brief Easy CBOR Converter - Type definitions and conversion API
 * @author IoT SDK Team
 * @date 2025-10-29
 *
 * This module provides data type definitions and conversion functionality
 * for the SDK. It supports conversion between structs and CBOR format with
 * optional AES encryption.
 */

#ifndef EASY_CBOR_H
#define EASY_CBOR_H

#include "easy_cbor_error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Easy CBOR Type Enumeration
 *
 * Defines supported data types for schema field definitions.
 * Used by the schema system to describe struct field types.
 */
typedef enum {
  EASY_CBOR_TYPE_INT8,   /**< 8-bit signed integer */
  EASY_CBOR_TYPE_INT16,  /**< 16-bit signed integer */
  EASY_CBOR_TYPE_INT32,  /**< 32-bit signed integer */
  EASY_CBOR_TYPE_UINT8,  /**< 8-bit unsigned integer */
  EASY_CBOR_TYPE_UINT16, /**< 16-bit unsigned integer */
  EASY_CBOR_TYPE_UINT32, /**< 32-bit unsigned integer */
  EASY_CBOR_TYPE_FLOAT,  /**< 32-bit floating point (single precision) */
  EASY_CBOR_TYPE_DOUBLE, /**< 64-bit floating point (double precision) */
  EASY_CBOR_TYPE_BOOL,   /**< Boolean type */
  EASY_CBOR_TYPE_STRING, /**< String (char array) */
  EASY_CBOR_TYPE_BYTES,  /**< Byte array */
  EASY_CBOR_TYPE_ENUM,   /**< Enum scalar (encoded as signed integer) */
  EASY_CBOR_TYPE_STRUCT  /**< Nested structure */
} easy_cbor_type_t;

/**
 * @brief Forward declaration of struct schema
 *
 * Allows field schema to reference nested struct schemas
 */
struct easy_cbor_struct_schema;

/**
 * @brief Easy CBOR Field Schema Definition
 *
 * Describes a single field within a structure for schema-based
 * serialization and deserialization operations.
 *
 * The schema supports both single values and multi-dimensional arrays.
 * For backward compatibility, the X-macro helpers in
 * `easy_cbor_macro.h` always generate 1D metadata:
 * - dimensions = 1
 * - dim0       = array_size
 * - dim1       = 1
 *
 * For 2D arrays (for example, uint8_t id[ROWS][COLS]), applications
 * can define the schema manually:
 * - type        = EASY_CBOR_TYPE_UINT8 (base element type)
 * - size        = sizeof(uint8_t) (base element size)
 * - dimensions  = 2
 * - dim0        = ROWS  (outer dimension)
 * - dim1        = COLS  (inner dimension)
 * - array_size  = dim0 * dim1 (total element count, used for bounds)
 *
 * The CBOR representation of a 2D primitive array is a nested array:
 * - C array: uint8_t id[2][3] = {{1,2,3},{4,5,6}};
 * - CBOR:    [[1, 2, 3], [4, 5, 6]]
 */
typedef struct {
  const char *name;      /**< Field name (must not be NULL) */
  easy_cbor_type_t type; /**< Field data type (element type for arrays) */
  size_t offset;         /**< Byte offset within parent struct */
  size_t size; /**< Element size in bytes (for arrays and single values) */
  size_t array_size; /**< Total element count for arrays (1 for non-arrays) */
  const struct easy_cbor_struct_schema
      *nested_schema; /**< Schema for nested structs (NULL if not applicable) */
  uint8_t dimensions; /**< Number of dimensions: 1 (default), 2 for 2D arrays */
  size_t
      dim0; /**< Size of outer dimension (rows) for multi-dimensional arrays */
  size_t dim1; /**< Size of inner dimension (columns) for 2D arrays */
} easy_cbor_field_schema_t;

/**
 * @brief Easy CBOR Struct Schema Definition
 *
 * Describes the complete structure layout for schema-based operations.
 * Contains an array of field schemas and metadata about the structure.
 */
typedef struct easy_cbor_struct_schema {
  const easy_cbor_field_schema_t *fields; /**< Array of field schemas */
  size_t field_count;                     /**< Number of fields in the array */
  size_t struct_size; /**< Total size of the structure in bytes */
} easy_cbor_struct_schema_t;

/**
 * @brief Easy CBOR Configuration
 *
 * Configuration structure for Easy CBOR initialization.
 */
typedef struct {
  uint8_t max_recursion_depth; /**< Maximum nesting depth for struct
                                  encode/decode */
} easy_cbor_config_t;

/**
 * @brief Initialize the Easy CBOR module
 *
 * Initializes the Easy CBOR module with the provided configuration.
 * This function must be called before any other conversion operations.
 *
 * @param config Configuration structure (must not be NULL)
 * @return IOT_OK on success
 *         IOT_ERR_NULL_POINTER if config is NULL
 *
 * @note This is a placeholder implementation; actual initialization logic
 *       will be added when AES encryption and other features are implemented.
 *
 * @example
 * @code
 * easy_cbor_config_t config = {
 *     .max_recursion_depth = 5,
 * };
 * int ret = easy_cbor_init(&config);
 * if (ret != IOT_OK) {
 *     printf("Init failed: %s\n", easy_cbor_error_message(ret));
 * }
 * @endcode
 */
int easy_cbor_init(const easy_cbor_config_t *config);

/**
 * @brief Convert struct to CBOR format
 *
 * Serializes a C struct to CBOR binary format based on the provided schema.
 * Supports nested structs and arrays with a maximum recursion depth of 5.
 *
 * @param struct_ptr Pointer to the struct to serialize (must not be NULL)
 * @param schema Pointer to the struct schema (must not be NULL)
 * @param cbor_buffer Output buffer for CBOR data (must not be NULL)
 * @param cbor_size Pointer to buffer size (input: max size, output: actual
 * size)
 * @return IOT_OK on success
 *         IOT_ERR_NULL_POINTER if any pointer parameter is NULL
 *         IOT_ERR_BUFFER_TOO_SMALL if cbor_buffer is too small
 *         IOT_ERR_ENCODE if CBOR encoding fails
 *         IOT_ERR_INVALID_FORMAT if data type is unsupported
 *
 * @note Maximum recursion depth is 5 levels for nested structs
 *
 * @example
 * @code
 * sensor_data_t data = {.temp_celsius = 25, .humidity = 60};
 * uint8_t cbor_buf[128];
 * size_t cbor_len = sizeof(cbor_buf);
 * int ret = easy_cbor_encode_cbor(&data, &sensor_data_schema, cbor_buf,
 * &cbor_len); if (ret == IOT_OK) { printf("CBOR size: %zu bytes\n", cbor_len);
 * }
 * @endcode
 */
int easy_cbor_encode_cbor(const void *struct_ptr,
                          const easy_cbor_struct_schema_t *schema,
                          uint8_t *cbor_buffer, size_t *cbor_size);

/**
 * @brief Convert CBOR data to struct
 *
 * Deserializes CBOR binary format to a C struct based on the provided schema.
 * Supports nested structs and arrays with a maximum recursion depth of 5.
 * Unknown fields in CBOR data are silently ignored.
 *
 * @param cbor_buffer Input buffer containing CBOR data (must not be NULL)
 * @param cbor_size Size of CBOR data in bytes
 * @param struct_ptr Pointer to the struct to populate (must not be NULL)
 * @param schema Pointer to the struct schema (must not be NULL)
 * @return IOT_OK on success
 *         IOT_ERR_NULL_POINTER if any pointer parameter is NULL
 *         IOT_ERR_DECODE if CBOR decoding fails
 *         IOT_ERR_INVALID_FORMAT if CBOR format is invalid or type mismatch
 *
 * @note Maximum recursion depth is 5 levels for nested structs
 * @note Unknown CBOR fields are skipped without error
 *
 * @example
 * @code
 * sensor_data_t data;
 * int ret = easy_cbor_decode_cbor(cbor_buf, cbor_len, &data,
 * &sensor_data_schema); if (ret == IOT_OK) { printf("Temperature: %d C\n",
 * data.temp_celsius);
 * }
 * @endcode
 */
int easy_cbor_decode_cbor(const uint8_t *cbor_buffer, size_t cbor_size,
                          void *struct_ptr,
                          const easy_cbor_struct_schema_t *schema);

/**
 * @brief Convert raw CBOR binary to compact JSON string
 *
 * Converts CBOR encoded data directly to a compact JSON string without
 * requiring a schema. This is useful for debugging and logging CBOR data
 * in human-readable form.
 *
 * @param cbor_buffer Input buffer containing CBOR data (must not be NULL)
 * @param cbor_size Size of CBOR data in bytes
 * @param json_buffer Output buffer for JSON string (must not be NULL)
 * @param json_size Pointer to buffer size (input: max size, output: actual size
 * excluding NUL)
 * @return EASY_CBOR_OK on success
 *         EASY_CBOR_ERR_NULL_BUFFER if cbor_buffer or json_buffer is NULL
 *         EASY_CBOR_ERR_NULL_SIZE if json_size is NULL
 *         EASY_CBOR_ERR_BUFFER_TOO_SMALL if json_buffer is too small
 *         EASY_CBOR_ERR_DECODE if CBOR parsing fails
 *         EASY_CBOR_ERR_INVALID_FORMAT if CBOR contains non-string map keys
 *
 * @note This function does NOT require easy_cbor_init() to be called first.
 * @note Byte strings are encoded as hex strings (e.g., "AABB01").
 * @note NaN and Infinity are represented as JSON null.
 * @note CBOR tags are silently stripped; the tagged value is output directly.
 * @note Output is always NUL-terminated if the buffer has at least 1 byte.
 *
 * @example
 * @code
 * uint8_t cbor_buf[] = { ... };
 * char json_buf[256];
 * size_t json_len = sizeof(json_buf);
 * int ret = easy_cbor_to_json(cbor_buf, sizeof(cbor_buf), json_buf, &json_len);
 * if (ret == EASY_CBOR_OK) {
 *     printf("JSON: %s\n", json_buf);
 * }
 * @endcode
 */
int easy_cbor_to_json(const uint8_t *cbor_buffer, size_t cbor_size,
                      char *json_buffer, size_t *json_size);

#ifdef __cplusplus
}
#endif

#endif /* EASY_CBOR_H */
