# modules/easy_cbor/GEMINI.md
This module provides **struct <-> CBOR conversion** utilities and **CBOR → JSON string** conversion. Schemas are defined using X-macro helpers or manually when needed, enabling efficient serialization for embedded systems.

## What lives here (key files)

- `module.yml`
  - Declares the module entry point: `easy_cbor_init`. (loop is `null`).
  - Declares external dependencies (tinycbor).
  - Declares parameters such as `max_recursion_depth`.

- `inc/easy_cbor.h`
  - Public API for CBOR encoding, decoding, and JSON conversion.
  - Core conversion logic and type definitions.

- `inc/easy_cbor_macro.h`
  - X-macro helper definitions for simplified schema creation.

- `inc/easy_cbor_error.h` / `src/easy_cbor_error.c`
  - Module-specific error codes and string conversion.

- `src/easy_cbor.c`
  - Implementation of the conversion engine using the vendored tinycbor library.

## Config parameters (module.yml)

Required:

- None (Uses sensible defaults)

Optional:

- `max_recursion_depth` (u32, default: 5): Maximum nesting depth for struct encode/decode.

## Runtime behavior

- `easy_cbor_init()` initializes the module state (currently stateless or minimal init).
- **Schema Definition**: 
  - Prefer X-macro helpers in `easy_cbor_macro.h` for 1D arrays and standard structs.
  - For 2D arrays (e.g., `uint8_t id[ROWS][COLS]`), define schemas manually following the rules documented in `easy_cbor.h`.
- **Deserialization Policy**: Unknown fields are ignored for forward/backward compatibility. Type mismatch errors return `EASY_CBOR_ERR_INVALID_FORMAT`.
- **Recursion Control**: Nesting depth is checked against `max_recursion_depth` to prevent stack overflow.
- **CBOR → JSON Conversion**:
  - `easy_cbor_to_json()` converts raw CBOR binary to compact JSON string without requiring a schema.
  - Does NOT require `easy_cbor_init()` to be called first (standalone utility).
  - Byte strings are encoded as uppercase hex strings (e.g., `"AABB01"`).
  - NaN/Infinity are represented as JSON `null`.
  - CBOR tags are silently stripped; the tagged value is output directly.
  - Non-string map keys return `EASY_CBOR_ERR_INVALID_FORMAT`.

## Integration notes

- Relies on the **tinycbor** external library (vendored under `external/tinycbor`).
- Assumes tight memory limits; avoid large temporary buffers and deep recursion.
- When changing schemas, consider struct layout/padding impacts on the C side.

