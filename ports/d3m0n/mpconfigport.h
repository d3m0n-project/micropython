#include "types.h"
#include <math.h>

#define MICROPY_CONFIG_ROM_LEVEL			(MICROPY_CONFIG_ROM_LEVEL_MINIMUM)

#define MICROPY_ENABLE_COMPILER				(1)



#define MICROPY_ENABLE_GC					(1)
#define MICROPY_LONGINT_IMPL				(MICROPY_LONGINT_IMPL_LONGLONG)
#define MICROPY_HELPER_REPL					(1)
#define MICROPY_ENABLE_EXTERNAL_IMPORT		(1)

// Keep bytecode source locations so runtime exceptions include line numbers.
#define MICROPY_ENABLE_SOURCE_LINE			(1)
#define MICROPY_ERROR_REPORTING				(MICROPY_ERROR_REPORTING_DETAILED)

#define MICROPY_ALLOC_PATH_MAX				(256)

// Use the minimum headroom in the chunk allocator for parse nodes.
#define MICROPY_ALLOC_PARSE_CHUNK_INIT		(16)

// Disable all optional sys module features.
#define MICROPY_PY_SYS_MODULES				(0)
#define MICROPY_PY_SYS_EXIT					(0)
#define MICROPY_PY_SYS_PATH					(0)
#define MICROPY_PY_SYS_ARGV					(0)

//#define MICROPY_OBJ_REPR					MICROPY_OBJ_REPR_C
#define MICROPY_FLOAT_IMPL					MICROPY_FLOAT_IMPL_FLOAT
#define MICROPY_MULTIPLE_INHERITANCE		(1)
#ifndef isnan
#define isnan(x) __builtin_isnan(x)
#endif
#ifndef isinf
#define isinf(x) __builtin_isinf(x)
#endif
#ifndef signbit
#define signbit(x) __builtin_signbit(x)
#endif
#ifndef INFINITY
#define INFINITY __builtin_inff()
#endif
#ifndef NAN
#define NAN __builtin_nanf("")
#endif
#ifndef nanf
#define nanf(tagp) __builtin_nanf(tagp)
#endif
#ifndef copysignf
#define copysignf(x, y) __builtin_copysignf(x, y)
#endif
#ifndef nearbyintf
#define nearbyintf(x) __builtin_nearbyintf(x)
#endif


// type definitions for the specific machine

typedef long mp_off_t;

#define MICROPY_HW_BOARD_NAME			"d3m0n os"
#define MICROPY_HW_MCU_NAME				"H616"
#define MICROPY_MPHALPORT_H				"mphalport.h"

#define MICROPY_MIN_USE_STDOUT			(1)
#define MICROPY_HEAP_SIZE				(25600000) // heap size 25Mb
#define MICROPY_STACK_CHECK				(1)
#define MICROPY_THREAD_MIN_STACK_SIZE	(16 * 1024)

#define MP_STATE_PORT MP_STATE_VM
