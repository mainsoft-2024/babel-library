/**
 * @file easy_cbor.c
 * @brief Easy CBOR Converter - Implementation
 * @author IoT SDK Team
 * @date 2025-10-29
 *
 * Implementation of Easy CBOR initialization and conversion functions.
 */

#include "easy_cbor.h"
#include <cbor.h>
#include <inttypes.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static bool s_easy_cbor_initialized = false;
static uint8_t s_easy_cbor_max_recursion_depth = 5U;

/**
 * @brief Convert TinyCBOR error code to IoT SDK error code
 *
 * @param cbor_err TinyCBOR error code
 * @return Corresponding IoT SDK error code
 */
static int cbor_error_to_easy_cbor_error(CborError cbor_err);

/**
 * @brief Find field schema by name
 *
 * @param schema Struct schema
 * @param name Field name to search for
 * @return Pointer to field schema or NULL if not found
 */
static const easy_cbor_field_schema_t *
find_field_by_name(const easy_cbor_struct_schema_t *schema, const char *name);

/**
 * @brief Encode a single field value to CBOR
 *
 * @param encoder CBOR encoder
 * @param struct_ptr Pointer to struct containing the field
 * @param field Field schema
 * @param depth Current recursion depth
 * @return IoT SDK error code
 */
static int encode_field_single(CborEncoder *encoder, const void *struct_ptr,
                               const easy_cbor_field_schema_t *field,
                               int depth);

/**
 * @brief Encode an array field to CBOR
 *
 * @param encoder CBOR encoder
 * @param struct_ptr Pointer to struct containing the field
 * @param field Field schema
 * @param depth Current recursion depth
 * @return IoT SDK error code
 */
static int encode_field_array(CborEncoder *encoder, const void *struct_ptr,
                              const easy_cbor_field_schema_t *field, int depth);

/**
 * @brief Decode a single field value from CBOR
 *
 * @param it CBOR value iterator
 * @param struct_ptr Pointer to struct to populate
 * @param field Field schema
 * @param depth Current recursion depth
 * @return IoT SDK error code
 */
static int decode_field_single(CborValue *it, void *struct_ptr,
                               const easy_cbor_field_schema_t *field,
                               int depth);

/**
 * @brief Decode an array field from CBOR
 *
 * @param it CBOR value iterator
 * @param struct_ptr Pointer to struct to populate
 * @param field Field schema
 * @param depth Current recursion depth
 * @return IoT SDK error code
 */
static int decode_field_array(CborValue *it, void *struct_ptr,
                              const easy_cbor_field_schema_t *field, int depth);

/**
 * @brief Initialize the Easy CBOR module
 *
 * Initializes the Easy CBOR module with the provided configuration.
 * This function must be called before any other conversion operations.
 *
 * @param config Configuration structure (must not be NULL)
 * @return EASY_CBOR_OK on success
 *         EASY_CBOR_ERR_NULL_POINTER if config is NULL
 *
 * @note This is a placeholder implementation. When AES encryption and other
 *       features are added, this function will perform actual initialization
 *       such as:
 *       - Setting up AES context if encryption is enabled
 *       - Allocating internal buffers
 *       - Initializing CBOR/JSON parser contexts
 *
 * @example
 * @code
 * easy_cbor_config_t config = {0};
 * int ret = easy_cbor_init(&config);
 * if (ret != EASY_CBOR_OK) {
 *     printf("Init failed: %s\n", iot_error_message(ret));
 * }
 * @endcode
 */
int easy_cbor_init(const easy_cbor_config_t *config) {
  // Check if already initialized
  if (s_easy_cbor_initialized) {
    return EASY_CBOR_ERR_ALREADY_INITIALIZED;
  }

  // Parameter validation
  if (config == NULL) {
    return EASY_CBOR_ERR_INVALID_CONFIG;
  }

  if (config->max_recursion_depth == 0U) {
    return EASY_CBOR_ERR_INVALID_CONFIG;
  }

  s_easy_cbor_max_recursion_depth = config->max_recursion_depth;

  /*
   * Future implementation will include:
   * - AES context initialization if encryption is enabled
   * - Buffer pool allocation for conversion operations
   * - CBOR/JSON library initialization
   * - State tracking setup
   */

  s_easy_cbor_initialized = true;

  return EASY_CBOR_OK;
}

/* ========================================================================== */
/* Internal Helper Functions                                                  */
/* ========================================================================== */

static int cbor_error_to_easy_cbor_error(CborError cbor_err) {
  switch (cbor_err) {
  case CborNoError:
    return EASY_CBOR_OK;
  case CborErrorOutOfMemory:
    return EASY_CBOR_ERR_NO_MEMORY;
  case CborErrorDataTooLarge:
    return EASY_CBOR_ERR_INVALID_FORMAT;
  case CborErrorUnknownLength:
  case CborErrorAdvancePastEOF:
  case CborErrorIO:
    return EASY_CBOR_ERR_DECODE;
  default:
    return EASY_CBOR_ERR_ENCODE;
  }
}

static const easy_cbor_field_schema_t *
find_field_by_name(const easy_cbor_struct_schema_t *schema, const char *name) {
  if (schema == NULL || name == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < schema->field_count; i++) {
    if (strcmp(schema->fields[i].name, name) == 0) {
      return &schema->fields[i];
    }
  }
  return NULL;
}

static int encode_field_single(CborEncoder *encoder, const void *struct_ptr,
                               const easy_cbor_field_schema_t *field,
                               int depth) {
  const uint8_t *field_ptr = (const uint8_t *)struct_ptr + field->offset;
  CborError err = CborNoError;

  switch (field->type) {
  case EASY_CBOR_TYPE_INT8:
    err = cbor_encode_int(encoder, *(int8_t *)field_ptr);
    break;
  case EASY_CBOR_TYPE_INT16:
    err = cbor_encode_int(encoder, *(int16_t *)field_ptr);
    break;
  case EASY_CBOR_TYPE_INT32:
    err = cbor_encode_int(encoder, *(int32_t *)field_ptr);
    break;
  case EASY_CBOR_TYPE_UINT8:
    err = cbor_encode_uint(encoder, *(uint8_t *)field_ptr);
    break;
  case EASY_CBOR_TYPE_UINT16:
    err = cbor_encode_uint(encoder, *(uint16_t *)field_ptr);
    break;
  case EASY_CBOR_TYPE_UINT32:
    err = cbor_encode_uint(encoder, *(uint32_t *)field_ptr);
    break;
  case EASY_CBOR_TYPE_FLOAT:
    err = cbor_encode_float(encoder, *(float *)field_ptr);
    break;
  case EASY_CBOR_TYPE_DOUBLE:
    err = cbor_encode_double(encoder, *(double *)field_ptr);
    break;
  case EASY_CBOR_TYPE_BOOL:
    err = cbor_encode_boolean(encoder, *(bool *)field_ptr);
    break;
  case EASY_CBOR_TYPE_STRING: {
    // Encode string using actual length, not array_size
    size_t str_len = strnlen((const char *)field_ptr, field->array_size);
    err = cbor_encode_text_string(encoder, (const char *)field_ptr, str_len);
    break;
  }
  case EASY_CBOR_TYPE_BYTES:
    err = cbor_encode_byte_string(encoder, field_ptr, field->array_size);
    break;
  case EASY_CBOR_TYPE_ENUM: {
    // Encode enum as signed integer based on underlying storage size.
    int64_t value = 0;

    switch (field->size) {
    case 1: {
      int8_t tmp;
      memcpy(&tmp, field_ptr, sizeof(tmp));
      value = tmp;
      break;
    }
    case 2: {
      int16_t tmp;
      memcpy(&tmp, field_ptr, sizeof(tmp));
      value = tmp;
      break;
    }
    case 4: {
      int32_t tmp;
      memcpy(&tmp, field_ptr, sizeof(tmp));
      value = tmp;
      break;
    }
    default:
      // Unsupported enum storage size.
      return EASY_CBOR_ERR_INVALID_FORMAT;
    }

    err = cbor_encode_int(encoder, value);
    break;
  }
  case EASY_CBOR_TYPE_STRUCT: {
    // Check recursion depth
    if (depth >= s_easy_cbor_max_recursion_depth) {
      return EASY_CBOR_ERR_MAX_RECURSION;
    }
    if (field->nested_schema == NULL) {
      return EASY_CBOR_ERR_NESTED_SCHEMA_NULL;
    }

    // Create nested map
    CborEncoder mapEncoder;
    err = cbor_encoder_create_map(encoder, &mapEncoder,
                                  field->nested_schema->field_count);
    if (err != CborNoError) {
      break;
    }

    // Encode nested struct fields
    for (size_t i = 0; i < field->nested_schema->field_count; i++) {
      const easy_cbor_field_schema_t *nested_field =
          &field->nested_schema->fields[i];

      // Encode field name
      err = cbor_encode_text_string(&mapEncoder, nested_field->name,
                                    strlen(nested_field->name));
      if (err != CborNoError) {
        break;
      }

      // Encode field value (handle single/array)
      int ret;
      // STRING and BYTES types are always encoded as single values
      if (nested_field->array_size == 1 ||
          nested_field->type == EASY_CBOR_TYPE_STRING ||
          nested_field->type == EASY_CBOR_TYPE_BYTES) {
        ret = encode_field_single(&mapEncoder, field_ptr, nested_field,
                                  depth + 1);
      } else {
        ret =
            encode_field_array(&mapEncoder, field_ptr, nested_field, depth + 1);
      }
      if (ret != EASY_CBOR_OK) {
        return ret;
      }
    }

    err = cbor_encoder_close_container(encoder, &mapEncoder);
    break;
  }
  default:
    return EASY_CBOR_ERR_INVALID_FORMAT;
  }

  return cbor_error_to_easy_cbor_error(err);
}

static int encode_field_array(CborEncoder *encoder, const void *struct_ptr,
                              const easy_cbor_field_schema_t *field,
                              int depth) {
  const uint8_t *field_ptr = (const uint8_t *)struct_ptr + field->offset;

  // Treat dimensions == 0 as 1D for backward compatibility
  uint8_t dims = (field->dimensions == 0U) ? 1U : field->dimensions;

  // 1D array: keep existing behavior
  if (dims == 1U) {
    CborEncoder arrayEncoder;
    CborError err =
        cbor_encoder_create_array(encoder, &arrayEncoder, field->array_size);
    if (err != CborNoError) {
      return cbor_error_to_easy_cbor_error(err);
    }

    // Encode each array element
    for (size_t i = 0; i < field->array_size; i++) {
      const uint8_t *element_ptr = field_ptr + (i * field->size);

      // Create temporary field schema for single element
      easy_cbor_field_schema_t temp_field = *field;
      temp_field.offset = 0;
      temp_field.array_size = 1;
      temp_field.dimensions = 1U;
      temp_field.dim0 = 1U;
      temp_field.dim1 = 1U;

      int ret =
          encode_field_single(&arrayEncoder, element_ptr, &temp_field, depth);
      if (ret != EASY_CBOR_OK) {
        return ret;
      }
    }

    err = cbor_encoder_close_container(encoder, &arrayEncoder);
    return cbor_error_to_easy_cbor_error(err);
  } else if (dims == 2U) {
    // 2D array: encode as array-of-arrays
    size_t rows = field->dim0;
    size_t cols = field->dim1;

    if (rows == 0U || cols == 0U) {
      return EASY_CBOR_ERR_INVALID_FORMAT;
    }

    CborEncoder outerArray;
    CborError err = cbor_encoder_create_array(encoder, &outerArray, rows);
    if (err != CborNoError) {
      return cbor_error_to_easy_cbor_error(err);
    }

    for (size_t r = 0; r < rows; r++) {
      const uint8_t *row_ptr = field_ptr + (r * cols * field->size);

      // Build a temporary 1D schema describing a single row
      easy_cbor_field_schema_t row_field = *field;
      row_field.offset = 0;
      row_field.array_size = cols;
      row_field.dimensions = 1U;
      row_field.dim0 = cols;
      row_field.dim1 = 1U;

      int ret = encode_field_array(&outerArray, row_ptr, &row_field, depth);
      if (ret != EASY_CBOR_OK) {
        return ret;
      }
    }

    err = cbor_encoder_close_container(encoder, &outerArray);
    return cbor_error_to_easy_cbor_error(err);
  } else {
    // Higher dimensions are currently not supported
    return EASY_CBOR_ERR_INVALID_FORMAT;
  }
}

static int decode_field_single(CborValue *it, void *struct_ptr,
                               const easy_cbor_field_schema_t *field,
                               int depth) {
  uint8_t *field_ptr = (uint8_t *)struct_ptr + field->offset;
  CborError err = CborNoError;

  switch (field->type) {
  case EASY_CBOR_TYPE_INT8: {
    int64_t value;
    err = cbor_value_get_int64(it, &value);
    if (err == CborNoError) {
      *(int8_t *)field_ptr = (int8_t)value;
    }
    break;
  }
  case EASY_CBOR_TYPE_INT16: {
    int64_t value;
    err = cbor_value_get_int64(it, &value);
    if (err == CborNoError) {
      *(int16_t *)field_ptr = (int16_t)value;
    }
    break;
  }
  case EASY_CBOR_TYPE_INT32: {
    int64_t value;
    err = cbor_value_get_int64(it, &value);
    if (err == CborNoError) {
      *(int32_t *)field_ptr = (int32_t)value;
    }
    break;
  }
  case EASY_CBOR_TYPE_UINT8: {
    uint64_t value;
    err = cbor_value_get_uint64(it, &value);
    if (err == CborNoError) {
      *(uint8_t *)field_ptr = (uint8_t)value;
    }
    break;
  }
  case EASY_CBOR_TYPE_UINT16: {
    uint64_t value;
    err = cbor_value_get_uint64(it, &value);
    if (err == CborNoError) {
      *(uint16_t *)field_ptr = (uint16_t)value;
    }
    break;
  }
  case EASY_CBOR_TYPE_UINT32: {
    uint64_t value;
    err = cbor_value_get_uint64(it, &value);
    if (err == CborNoError) {
      *(uint32_t *)field_ptr = (uint32_t)value;
    }
    break;
  }
  case EASY_CBOR_TYPE_FLOAT: {
    float value;
    err = cbor_value_get_float(it, &value);
    if (err == CborNoError) {
      *(float *)field_ptr = value;
    }
    break;
  }
  case EASY_CBOR_TYPE_DOUBLE: {
    double value;
    err = cbor_value_get_double(it, &value);
    if (err == CborNoError) {
      *(double *)field_ptr = value;
    }
    break;
  }
  case EASY_CBOR_TYPE_BOOL: {
    bool value;
    err = cbor_value_get_boolean(it, &value);
    if (err == CborNoError) {
      *(bool *)field_ptr = value;
    }
    break;
  }
  case EASY_CBOR_TYPE_STRING: {
    // Verify it's actually a text string
    if (!cbor_value_is_text_string(it)) {
      return EASY_CBOR_ERR_INVALID_FORMAT;
    }
    size_t len = field->array_size;
    err = cbor_value_copy_text_string(it, (char *)field_ptr, &len, NULL);
    break;
  }
  case EASY_CBOR_TYPE_BYTES: {
    size_t len = field->array_size;
    err = cbor_value_copy_byte_string(it, field_ptr, &len, NULL);
    break;
  }
  case EASY_CBOR_TYPE_ENUM: {
    // Decode enum from signed integer into underlying storage.
    int64_t value = 0;
    err = cbor_value_get_int64(it, &value);
    if (err != CborNoError) {
      break;
    }

    switch (field->size) {
    case 1: {
      int8_t tmp = (int8_t)value;
      memcpy(field_ptr, &tmp, sizeof(tmp));
      break;
    }
    case 2: {
      int16_t tmp = (int16_t)value;
      memcpy(field_ptr, &tmp, sizeof(tmp));
      break;
    }
    case 4: {
      int32_t tmp = (int32_t)value;
      memcpy(field_ptr, &tmp, sizeof(tmp));
      break;
    }
    default:
      return EASY_CBOR_ERR_INVALID_FORMAT;
    }
    break;
  }
  case EASY_CBOR_TYPE_STRUCT: {
    // Check recursion depth
    if (depth >= s_easy_cbor_max_recursion_depth) {
      return EASY_CBOR_ERR_MAX_RECURSION;
    }
    if (field->nested_schema == NULL) {
      return EASY_CBOR_ERR_NESTED_SCHEMA_NULL;
    }

    // Enter nested map
    CborValue nestedMapValue;
    err = cbor_value_enter_container(it, &nestedMapValue);
    if (err != CborNoError) {
      break;
    }

    // Decode nested struct
    while (!cbor_value_at_end(&nestedMapValue)) {
      // Get field name (key)
      char key[64];
      size_t key_len = sizeof(key);
      err = cbor_value_copy_text_string(&nestedMapValue, key, &key_len, NULL);
      if (err != CborNoError) {
        break;
      }

      err = cbor_value_advance(&nestedMapValue);
      if (err != CborNoError) {
        break;
      }

      // Find field in nested schema
      const easy_cbor_field_schema_t *nested_field =
          find_field_by_name(field->nested_schema, key);
      if (nested_field == NULL) {
        // Skip unknown field
        err = cbor_value_advance(&nestedMapValue);
        continue;
      }

      // Decode field value
      int ret;
      // STRING and BYTES types are always decoded as single values
      if (nested_field->array_size == 1 ||
          nested_field->type == EASY_CBOR_TYPE_STRING ||
          nested_field->type == EASY_CBOR_TYPE_BYTES) {
        ret = decode_field_single(&nestedMapValue, field_ptr, nested_field,
                                  depth + 1);
      } else {
        ret = decode_field_array(&nestedMapValue, field_ptr, nested_field,
                                 depth + 1);
      }
      if (ret != EASY_CBOR_OK) {
        return ret;
      }

      // For nested structs and arrays, cbor_value_leave_container already
      // advanced the iterator For primitive types, we need to advance manually
      bool is_nested_container =
          (nested_field->type == EASY_CBOR_TYPE_STRUCT) ||
          (nested_field->array_size > 1 &&
           nested_field->type != EASY_CBOR_TYPE_STRING &&
           nested_field->type != EASY_CBOR_TYPE_BYTES);
      if (!is_nested_container) {
        err = cbor_value_advance(&nestedMapValue);
        if (err != CborNoError) {
          break;
        }
      }
    }

    // Leave container - this advances 'it' past the nested struct
    err = cbor_value_leave_container(it, &nestedMapValue);
    break;
  }
  default:
    return EASY_CBOR_ERR_INVALID_FORMAT;
  }

  return cbor_error_to_easy_cbor_error(err);
}

static int decode_field_array(CborValue *it, void *struct_ptr,
                              const easy_cbor_field_schema_t *field,
                              int depth) {
  uint8_t *field_ptr = (uint8_t *)struct_ptr + field->offset;

  // Treat dimensions == 0 as 1D for backward compatibility
  uint8_t dims = (field->dimensions == 0U) ? 1U : field->dimensions;

  if (dims == 1U) {
    // 1D array: keep existing behavior
    CborValue arrayValue;
    CborError err = cbor_value_enter_container(it, &arrayValue);
    if (err != CborNoError) {
      return cbor_error_to_easy_cbor_error(err);
    }

    // Decode each element
    size_t index = 0;
    while (!cbor_value_at_end(&arrayValue) && index < field->array_size) {
      uint8_t *element_ptr = field_ptr + (index * field->size);

      // Create temporary field schema for single element
      easy_cbor_field_schema_t temp_field = *field;
      temp_field.offset = 0;
      temp_field.array_size = 1;
      temp_field.dimensions = 1U;
      temp_field.dim0 = 1U;
      temp_field.dim1 = 1U;

      int ret =
          decode_field_single(&arrayValue, element_ptr, &temp_field, depth);
      if (ret != EASY_CBOR_OK) {
        return ret;
      }

      err = cbor_value_advance(&arrayValue);
      if (err != CborNoError) {
        return cbor_error_to_easy_cbor_error(err);
      }

      index++;
    }

    err = cbor_value_leave_container(it, &arrayValue);
    return cbor_error_to_easy_cbor_error(err);
  } else if (dims == 2U) {
    // 2D array: decode from array-of-arrays
    size_t rows = field->dim0;
    size_t cols = field->dim1;

    if (rows == 0U || cols == 0U) {
      return EASY_CBOR_ERR_INVALID_FORMAT;
    }

    CborValue outerArray;
    CborError err = cbor_value_enter_container(it, &outerArray);
    if (err != CborNoError) {
      return cbor_error_to_easy_cbor_error(err);
    }

    size_t row_index = 0;
    while (!cbor_value_at_end(&outerArray) && row_index < rows) {
      uint8_t *row_ptr = field_ptr + (row_index * cols * field->size);

      // Describe a single row as a 1D array
      easy_cbor_field_schema_t row_field = *field;
      row_field.offset = 0;
      row_field.array_size = cols;
      row_field.dimensions = 1U;
      row_field.dim0 = cols;
      row_field.dim1 = 1U;

      int ret = decode_field_array(&outerArray, row_ptr, &row_field, depth);
      if (ret != EASY_CBOR_OK) {
        return ret;
      }

      row_index++;
    }

    // Skip any remaining rows in the CBOR array, if present.
    while (!cbor_value_at_end(&outerArray)) {
      err = cbor_value_advance(&outerArray);
      if (err != CborNoError) {
        return cbor_error_to_easy_cbor_error(err);
      }
    }

    err = cbor_value_leave_container(it, &outerArray);
    return cbor_error_to_easy_cbor_error(err);
  } else {
    // Higher dimensions are currently not supported
    return EASY_CBOR_ERR_INVALID_FORMAT;
  }
}

/* ========================================================================== */
/* Public API Functions                                                       */
/* ========================================================================== */

int easy_cbor_encode_cbor(const void *struct_ptr,
                          const easy_cbor_struct_schema_t *schema,
                          uint8_t *cbor_buffer, size_t *cbor_size) {
  if (!s_easy_cbor_initialized) {
    return EASY_CBOR_ERR_NOT_INITIALIZED;
  }

  // Validate parameters
  if (struct_ptr == NULL) {
    return EASY_CBOR_ERR_NULL_STRUCT;
  }
  if (schema == NULL) {
    return EASY_CBOR_ERR_NULL_SCHEMA;
  }
  if (cbor_buffer == NULL) {
    return EASY_CBOR_ERR_NULL_BUFFER;
  }
  if (cbor_size == NULL) {
    return EASY_CBOR_ERR_NULL_SIZE;
  }

  // Initialize CBOR encoder
  CborEncoder encoder;
  cbor_encoder_init(&encoder, cbor_buffer, *cbor_size, 0);

  // Create root map
  CborEncoder mapEncoder;
  CborError err =
      cbor_encoder_create_map(&encoder, &mapEncoder, schema->field_count);
  if (err != CborNoError) {
    return cbor_error_to_easy_cbor_error(err);
  }

  // Encode each field
  for (size_t i = 0; i < schema->field_count; i++) {
    const easy_cbor_field_schema_t *field = &schema->fields[i];

    // Encode field name (key)
    err =
        cbor_encode_text_string(&mapEncoder, field->name, strlen(field->name));
    if (err != CborNoError) {
      return cbor_error_to_easy_cbor_error(err);
    }

    // Encode field value
    int ret;
    // STRING and BYTES types are always encoded as single values, even with
    // array_size > 1
    if (field->array_size == 1 || field->type == EASY_CBOR_TYPE_STRING ||
        field->type == EASY_CBOR_TYPE_BYTES) {
      ret = encode_field_single(&mapEncoder, struct_ptr, field, 0);
    } else {
      ret = encode_field_array(&mapEncoder, struct_ptr, field, 0);
    }

    if (ret != EASY_CBOR_OK) {
      return ret;
    }
  }

  // Close root map
  err = cbor_encoder_close_container(&encoder, &mapEncoder);
  if (err != CborNoError) {
    return cbor_error_to_easy_cbor_error(err);
  }

  // Get encoded size
  *cbor_size = cbor_encoder_get_buffer_size(&encoder, cbor_buffer);

  return EASY_CBOR_OK;
}

int easy_cbor_decode_cbor(const uint8_t *cbor_buffer, size_t cbor_size,
                          void *struct_ptr,
                          const easy_cbor_struct_schema_t *schema) {
  if (!s_easy_cbor_initialized) {
    return EASY_CBOR_ERR_NOT_INITIALIZED;
  }

  // Validate parameters
  if (cbor_buffer == NULL) {
    return EASY_CBOR_ERR_NULL_BUFFER;
  }
  if (struct_ptr == NULL) {
    return EASY_CBOR_ERR_NULL_STRUCT;
  }
  if (schema == NULL) {
    return EASY_CBOR_ERR_NULL_SCHEMA;
  }

  // Initialize CBOR parser
  CborParser parser;
  CborValue it;
  CborError err = cbor_parser_init(cbor_buffer, cbor_size, 0, &parser, &it);
  if (err != CborNoError) {
    return cbor_error_to_easy_cbor_error(err);
  }

  // Verify it's a map
  if (!cbor_value_is_map(&it)) {
    return EASY_CBOR_ERR_INVALID_FORMAT;
  }

  // Enter root map
  CborValue mapValue;
  err = cbor_value_enter_container(&it, &mapValue);
  if (err != CborNoError) {
    return cbor_error_to_easy_cbor_error(err);
  }

  // Decode each field
  while (!cbor_value_at_end(&mapValue)) {
    // Get field name (key)
    char key[64];
    size_t key_len = sizeof(key);
    err = cbor_value_copy_text_string(&mapValue, key, &key_len, NULL);
    if (err != CborNoError) {
      return cbor_error_to_easy_cbor_error(err);
    }

    err = cbor_value_advance(&mapValue);
    if (err != CborNoError) {
      return cbor_error_to_easy_cbor_error(err);
    }

    // Find field in schema
    const easy_cbor_field_schema_t *field = find_field_by_name(schema, key);
    if (field == NULL) {
      // Skip unknown field
      err = cbor_value_advance(&mapValue);
      if (err != CborNoError) {
        return cbor_error_to_easy_cbor_error(err);
      }
      continue;
    }

    // Decode field value
    int ret;
    // STRING and BYTES types are always decoded as single values
    if (field->array_size == 1 || field->type == EASY_CBOR_TYPE_STRING ||
        field->type == EASY_CBOR_TYPE_BYTES) {
      ret = decode_field_single(&mapValue, struct_ptr, field, 0);
    } else {
      ret = decode_field_array(&mapValue, struct_ptr, field, 0);
    }

    if (ret != EASY_CBOR_OK) {
      return ret;
    }

    /* For nested structs and arrays, cbor_value_leave_container already
     * advanced the iterator For primitive types, we need to advance manually
     */
    bool is_container =
        (field->type == EASY_CBOR_TYPE_STRUCT) ||
        (field->array_size > 1 && field->type != EASY_CBOR_TYPE_STRING &&
         field->type != EASY_CBOR_TYPE_BYTES);
    if (!is_container) {
      err = cbor_value_advance(&mapValue);
      if (err != CborNoError) {
        return cbor_error_to_easy_cbor_error(err);
      }
    }
  }

  // Leave root map
  err = cbor_value_leave_container(&it, &mapValue);
  if (err != CborNoError) {
    return cbor_error_to_easy_cbor_error(err);
  }

  return EASY_CBOR_OK;
}

/* ========================================================================== */
/* CBOR to JSON Conversion                                                    */
/* ========================================================================== */

/**
 * @brief JSON writer context for buffer-based output
 */
typedef struct {
  char *buf;       /**< Output buffer */
  size_t capacity; /**< Total buffer capacity */
  size_t offset;   /**< Current write position */
  bool overflow;   /**< Set to true when buffer is exhausted */
} json_writer_t;

/**
 * @brief Write a single character to the JSON buffer
 */
static void json_write_char(json_writer_t *w, char c) {
  if (w->overflow) {
    return;
  }
  /* Reserve 1 byte for NUL terminator */
  if (w->offset + 1 >= w->capacity) {
    w->overflow = true;
    return;
  }
  w->buf[w->offset++] = c;
  w->buf[w->offset] = '\0';
}

/**
 * @brief Write a raw string (no escaping) to the JSON buffer
 */
static void json_write_raw(json_writer_t *w, const char *str, size_t len) {
  if (w->overflow) {
    return;
  }
  if (w->offset + len >= w->capacity) {
    w->overflow = true;
    return;
  }
  memcpy(w->buf + w->offset, str, len);
  w->offset += len;
  w->buf[w->offset] = '\0';
}

/**
 * @brief Write a NUL-terminated string to the JSON buffer
 */
static void json_write_str(json_writer_t *w, const char *str) {
  json_write_raw(w, str, strlen(str));
}

/**
 * @brief Write a JSON-escaped text string (with surrounding quotes)
 */
static void json_write_escaped_string(json_writer_t *w, const char *str,
                                      size_t len) {
  json_write_char(w, '"');
  for (size_t i = 0; i < len && !w->overflow; i++) {
    unsigned char c = (unsigned char)str[i];
    switch (c) {
    case '"':
      json_write_raw(w, "\\\"", 2);
      break;
    case '\\':
      json_write_raw(w, "\\\\", 2);
      break;
    case '\b':
      json_write_raw(w, "\\b", 2);
      break;
    case '\f':
      json_write_raw(w, "\\f", 2);
      break;
    case '\n':
      json_write_raw(w, "\\n", 2);
      break;
    case '\r':
      json_write_raw(w, "\\r", 2);
      break;
    case '\t':
      json_write_raw(w, "\\t", 2);
      break;
    default:
      if (c < 0x20) {
        /* Control characters: \u00XX */
        char esc[7];
        snprintf(esc, sizeof(esc), "\\u%04x", c);
        json_write_raw(w, esc, 6);
      } else {
        json_write_char(w, (char)c);
      }
      break;
    }
  }
  json_write_char(w, '"');
}

/**
 * @brief Write a byte string as a hex-encoded JSON string
 */
static void json_write_hex_string(json_writer_t *w, const uint8_t *data,
                                  size_t len) {
  static const char hex_chars[] = "0123456789ABCDEF";
  json_write_char(w, '"');
  for (size_t i = 0; i < len && !w->overflow; i++) {
    json_write_char(w, hex_chars[(data[i] >> 4) & 0x0F]);
    json_write_char(w, hex_chars[data[i] & 0x0F]);
  }
  json_write_char(w, '"');
}

/**
 * @brief Convert uint64 to decimal string (portable, no PRIu64 dependency)
 * @return Number of characters written (excluding NUL)
 */
static int uint64_to_str(uint64_t val, char *buf, size_t buf_size) {
  /* Build digits in reverse, then reverse them */
  char tmp[21]; /* max uint64 = 20 digits + NUL */
  int len = 0;
  if (val == 0) {
    tmp[len++] = '0';
  } else {
    while (val > 0 && len < 20) {
      tmp[len++] = '0' + (char)(val % 10);
      val /= 10;
    }
  }
  if ((size_t)len >= buf_size) {
    len = (int)(buf_size - 1);
  }
  /* Reverse into output buffer */
  for (int i = 0; i < len; i++) {
    buf[i] = tmp[len - 1 - i];
  }
  buf[len] = '\0';
  return len;
}

/**
 * @brief Convert int64 to decimal string (portable, no PRId64 dependency)
 * @return Number of characters written (excluding NUL)
 */
static int int64_to_str(int64_t val, char *buf, size_t buf_size) {
  if (val < 0) {
    if (buf_size < 2) {
      buf[0] = '\0';
      return 0;
    }
    buf[0] = '-';
    /* Handle INT64_MIN safely: cast after negation via unsigned */
    uint64_t uval = (uint64_t)(-(val + 1)) + 1U;
    int n = uint64_to_str(uval, buf + 1, buf_size - 1);
    return n + 1;
  }
  return uint64_to_str((uint64_t)val, buf, buf_size);
}

/**
 * @brief Recursively convert a CborValue to JSON
 */
static int cbor_value_to_json_recursive(json_writer_t *w, CborValue *it,
                                        int depth) {
  if (depth <= 0) {
    return EASY_CBOR_ERR_MAX_RECURSION;
  }

  CborType type = cbor_value_get_type(it);

  switch (type) {
  case CborIntegerType: {
    if (cbor_value_is_unsigned_integer(it)) {
      uint64_t val;
      cbor_value_get_uint64(it, &val);
      char num[21]; /* max uint64 = 20 digits + NUL */
      int n = uint64_to_str(val, num, sizeof(num));
      json_write_raw(w, num, (size_t)n);
    } else {
      int64_t val;
      cbor_value_get_int64(it, &val);
      char num[22]; /* '-' + max int64 = 21 chars + NUL */
      int n = int64_to_str(val, num, sizeof(num));
      json_write_raw(w, num, (size_t)n);
    }
    break;
  }

#ifndef CBOR_NO_FLOATING_POINT
  case CborHalfFloatType:
  case CborFloatType: {
    float fval;
    if (type == CborHalfFloatType) {
      /* Half-float: convert 16-bit half to 32-bit float */
      cbor_value_get_half_float_as_float(it, &fval);
    } else {
      cbor_value_get_float(it, &fval);
    }
    int cls = fpclassify(fval);
    if (cls == FP_NAN || cls == FP_INFINITE) {
      json_write_str(w, "null");
    } else {
      char num[32];
      int n = snprintf(num, sizeof(num), "%g", (double)fval);
      json_write_raw(w, num, (size_t)n);
    }
    break;
  }

  case CborDoubleType: {
    double dval;
    cbor_value_get_double(it, &dval);
    int cls = fpclassify(dval);
    if (cls == FP_NAN || cls == FP_INFINITE) {
      json_write_str(w, "null");
    } else {
      char num[32];
      int n = snprintf(num, sizeof(num), "%.17g", dval);
      json_write_raw(w, num, (size_t)n);
    }
    break;
  }
#else
  case CborHalfFloatType:
  case CborFloatType:
  case CborDoubleType:
    json_write_str(w, "null");
    break;
#endif

  case CborBooleanType: {
    bool val;
    cbor_value_get_boolean(it, &val);
    json_write_str(w, val ? "true" : "false");
    break;
  }

  case CborNullType:
    json_write_str(w, "null");
    break;

  case CborUndefinedType:
    json_write_str(w, "null");
    break;

  case CborSimpleType:
    json_write_str(w, "null");
    break;

  case CborTextStringType: {
    /* Get total string length first */
    size_t len = 0;
    CborError err = cbor_value_calculate_string_length(it, &len);
    if (err != CborNoError) {
      return EASY_CBOR_ERR_DECODE;
    }

    if (len == 0) {
      json_write_raw(w, "\"\"", 2);
    } else {
      /* Allocate temporary buffer on stack for small strings, skip for huge
       * ones */
      char stack_buf[128];
      char *str = stack_buf;
      size_t buf_len = sizeof(stack_buf);

      if (len <= sizeof(stack_buf) - 1) {
        buf_len = sizeof(stack_buf);
      } else {
        /* For large strings, truncate to what we can fit (NUL space) */
        buf_len = sizeof(stack_buf);
        len = sizeof(stack_buf) - 1;
      }

      err = cbor_value_copy_text_string(it, str, &len, NULL);
      if (err != CborNoError) {
        return EASY_CBOR_ERR_DECODE;
      }
      json_write_escaped_string(w, str, len);
    }
    break;
  }

  case CborByteStringType: {
    size_t len = 0;
    CborError err = cbor_value_calculate_string_length(it, &len);
    if (err != CborNoError) {
      return EASY_CBOR_ERR_DECODE;
    }

    if (len == 0) {
      json_write_raw(w, "\"\"", 2);
    } else {
      uint8_t stack_buf[64];
      uint8_t *bytes = stack_buf;
      size_t buf_len = sizeof(stack_buf);

      if (len <= sizeof(stack_buf)) {
        buf_len = sizeof(stack_buf);
      } else {
        /* Truncate to buffer capacity (byte strings don't need NUL) */
        buf_len = sizeof(stack_buf);
        len = sizeof(stack_buf);
      }

      err = cbor_value_copy_byte_string(it, bytes, &len, NULL);
      if (err != CborNoError) {
        return EASY_CBOR_ERR_DECODE;
      }
      json_write_hex_string(w, bytes, len);
    }
    break;
  }

  case CborArrayType: {
    CborValue element;
    CborError err = cbor_value_enter_container(it, &element);
    if (err != CborNoError) {
      return EASY_CBOR_ERR_DECODE;
    }

    json_write_char(w, '[');
    bool first = true;
    int safety = 0;
    while (!cbor_value_at_end(&element)) {
      if (++safety > 4096) {
        return EASY_CBOR_ERR_DECODE;
      }
      if (!first) {
        json_write_char(w, ',');
      }
      first = false;

      int ret = cbor_value_to_json_recursive(w, &element, depth - 1);
      if (ret != EASY_CBOR_OK) {
        return ret;
      }
    }
    json_write_char(w, ']');

    err = cbor_value_leave_container(it, &element);
    if (err != CborNoError) {
      return EASY_CBOR_ERR_DECODE;
    }
    return EASY_CBOR_OK; /* container already advanced */
  }

  case CborMapType: {
    CborValue element;
    CborError err = cbor_value_enter_container(it, &element);
    if (err != CborNoError) {
      return EASY_CBOR_ERR_DECODE;
    }

    json_write_char(w, '{');
    bool first = true;
    int safety = 0;
    while (!cbor_value_at_end(&element)) {
      if (++safety > 4096) {
        return EASY_CBOR_ERR_DECODE;
      }
      if (!first) {
        json_write_char(w, ',');
      }
      first = false;

      /* Key must be a text string */
      if (!cbor_value_is_text_string(&element)) {
        return EASY_CBOR_ERR_INVALID_FORMAT;
      }

      /* Write key */
      char key_buf[64];
      size_t key_len = sizeof(key_buf) - 1;
      err = cbor_value_copy_text_string(&element, key_buf, &key_len, NULL);
      if (err != CborNoError) {
        return EASY_CBOR_ERR_DECODE;
      }
      json_write_escaped_string(w, key_buf, key_len);

      json_write_char(w, ':');

      /* Advance past key */
      err = cbor_value_advance(&element);
      if (err != CborNoError) {
        return EASY_CBOR_ERR_DECODE;
      }

      /* Write value */
      int ret = cbor_value_to_json_recursive(w, &element, depth - 1);
      if (ret != EASY_CBOR_OK) {
        return ret;
      }
    }
    json_write_char(w, '}');

    err = cbor_value_leave_container(it, &element);
    if (err != CborNoError) {
      return EASY_CBOR_ERR_DECODE;
    }
    return EASY_CBOR_OK; /* container already advanced */
  }

  case CborTagType: {
    /* Skip tag and convert the tagged value */
    CborError err = cbor_value_advance_fixed(it);
    if (err != CborNoError) {
      return EASY_CBOR_ERR_DECODE;
    }
    return cbor_value_to_json_recursive(w, it, depth - 1);
  }

  default:
    json_write_str(w, "null");
    break;
  }

  /* Advance past the current (non-container) value.
   * Container types (Array, Map) and Tag already advance and return early. */
  {
    CborError adv_err = cbor_value_advance(it);
    if (adv_err != CborNoError) {
      return EASY_CBOR_ERR_DECODE;
    }
  }
  return EASY_CBOR_OK;
}

int easy_cbor_to_json(const uint8_t *cbor_buffer, size_t cbor_size,
                      char *json_buffer, size_t *json_size) {
  /* Validate parameters */
  if (cbor_buffer == NULL || json_buffer == NULL) {
    return EASY_CBOR_ERR_NULL_BUFFER;
  }
  if (json_size == NULL) {
    return EASY_CBOR_ERR_NULL_SIZE;
  }
  if (*json_size == 0) {
    return EASY_CBOR_ERR_BUFFER_TOO_SMALL;
  }

  /* Initialize JSON writer */
  json_writer_t writer = {
      .buf = json_buffer,
      .capacity = *json_size,
      .offset = 0,
      .overflow = false,
  };
  json_buffer[0] = '\0';

  /* Initialize CBOR parser */
  CborParser parser;
  CborValue it;
  CborError err = cbor_parser_init(cbor_buffer, cbor_size, 0, &parser, &it);
  if (err != CborNoError) {
    return EASY_CBOR_ERR_DECODE;
  }

  /* Convert */
  int max_depth = (int)s_easy_cbor_max_recursion_depth;
  if (max_depth == 0) {
    max_depth = 5; /* default if init was not called */
  }

  int ret = cbor_value_to_json_recursive(&writer, &it, max_depth);
  if (ret != EASY_CBOR_OK) {
    return ret;
  }

  if (writer.overflow) {
    /* NUL-terminate at the end of buffer */
    json_buffer[writer.capacity - 1] = '\0';
    return EASY_CBOR_ERR_BUFFER_TOO_SMALL;
  }

  *json_size = writer.offset;
  return EASY_CBOR_OK;
}
