#include <assert.h>
#include <inttypes.h>

#define LFDS710_PAL_INLINE                   inline

static LFDS710_PAL_INLINE void lfds710_pal_barrier_compiler( void )
{
  __asm__ __volatile__ ( "" : : : "memory" );
}

  // TRD : GCC >= 4.7.3 compiler barriers are built into the intrinsics
  #define LF_PAL_COMPILER_BARRIERS_MISSING_PRESUMED_HAVING_A_GOOD_TIME

  #define LF_PAL_BARRIER_PROCESSOR_LOAD   __atomic_thread_fence( __ATOMIC_ACQUIRE )
  #define LF_PAL_BARRIER_PROCESSOR_STORE  __atomic_thread_fence( __ATOMIC_RELEASE )
  #define LF_PAL_BARRIER_PROCESSOR_FULL   __atomic_thread_fence( __ATOMIC_ACQ_REL )

  #define LF_PAL_ATOMIC_ADD( pointer_to_target, value, result, result_type )                   \
  {                                                                                                 \
    (result) = (result_type) __atomic_add_fetch( (pointer_to_target), (value), __ATOMIC_RELAXED );  \
  }

  #define LF_PAL_ATOMIC_CAS( pointer_to_destination, pointer_to_compare, new_destination, cas_strength, result )                                                       \
  {                                                                                                                                                                         \
    result = (char unsigned) __atomic_compare_exchange_n( pointer_to_destination, pointer_to_compare, new_destination, cas_strength, __ATOMIC_RELAXED, __ATOMIC_RELAXED );  \
  }

#define LF_PAL_ATOMIC_DWCAS( pointer_to_destination, pointer_to_compare, pointer_to_new_destination, cas_strength, result ) \
  {                                                                     \
    (result) = 0;                                                       \
                                                                        \
    __asm__ __volatile__                                                \
      (                                                                 \
        "lock;"           /* make cmpxchg16b atomic        */           \
        "cmpxchg16b %0;"  /* cmpxchg16b sets ZF on success */           \
        "setz       %4;"  /* if ZF set, set result to 1    */           \
                                                                        \
        /* output */                                                    \
        : "+m" ((pointer_to_destination)[0]), "+m" ((pointer_to_destination)[1]), "+a" ((pointer_to_compare)[0]), "+d" ((pointer_to_compare)[1]), "=q" (result) \
                                                                        \
          /* input */                                                   \
        : "b" ((pointer_to_new_destination)[0]), "c" ((pointer_to_new_destination)[1]) \
                                                                        \
          /* clobbered */                                               \
        :                                                               \
        );                                                              \
  }

  #define LF_PAL_ATOMIC_EXCHANGE( pointer_to_destination, exchange, exchange_type )                         \
  {                                                                                                              \
    (exchange) = (exchange_type) __atomic_exchange_n( (pointer_to_destination), (exchange), __ATOMIC_RELAXED );  \
  }

  #define LF_PAL_ATOMIC_SET( pointer_to_destination, new_value )                       \
  {                                                                                         \
    (void) __atomic_exchange_n( (pointer_to_destination), (new_value), __ATOMIC_RELAXED );  \
  }


/***** defines *****/
#define LF_MISC_VERSION_STRING "7.1.0"
#define LF_MISC_VERSION_INTEGER 710

#ifndef NULL
#define NULL ((void *)0)
#endif

#define POINTER 0
#define COUNTER 1
#define PAC_SIZE 2

#define LF_MISC_DELIBERATELY_CRASH                                             \
  {                                                                            \
    char *c = 0;                                                               \
    *c = 0;                                                                    \
  }

#define LF_PAL_PROCESSOR

typedef int long long lf_pal_int_t;
typedef int long long unsigned lf_pal_uint_t;

#define LF_PAL_PROCESSOR_STRING "x64"

#define LF_PAL_ALIGN_SINGLE_POINTER 8
#define LF_PAL_ALIGN_DOUBLE_POINTER 16

// TRD : Intel bring over two cache lines at once, always, unless disabled in
// BIOS
#define LF_PAL_ATOMIC_ISOLATION_IN_BYTES 128

#define LF_PAL_ALIGN(alignment) __attribute__((aligned(alignment)))
#define LF_PAL_INLINE inline

#if (!defined LF_PAL_ATOMIC_ADD)
#define LF_PAL_NO_ATOMIC_ADD
#define LF_MISC_ATOMIC_SUPPORT_ADD 0
#define LF_PAL_ATOMIC_ADD(pointer_to_target, value, result, result_type)       \
  {                                                                            \
    assert(!"LF_PAL_ATOMIC_ADD not implemented for this platform.");           \
    LF_MISC_DELIBERATELY_CRASH;                                                \
  }
#else
#define LF_MISC_ATOMIC_SUPPORT_ADD 1
#endif

#if (!defined LF_PAL_ATOMIC_CAS)
#define LF_PAL_NO_ATOMIC_CAS
#define LF_MISC_ATOMIC_SUPPORT_CAS 0
#define LF_PAL_ATOMIC_CAS(pointer_to_destination, pointer_to_compare,          \
                          new_destination, cas_strength, result)               \
  {                                                                            \
    assert(!"LF_PAL_ATOMIC_CAS not implemented for this platform.");           \
    (result) = 0;                                                              \
    LF_MISC_DELIBERATELY_CRASH;                                                \
  }
#else
#define LF_MISC_ATOMIC_SUPPORT_CAS 1
#endif

#if (!defined LF_PAL_ATOMIC_DWCAS)
#define LF_PAL_NO_ATOMIC_DWCAS
#define LF_MISC_ATOMIC_SUPPORT_DWCAS 0
#define LF_PAL_ATOMIC_DWCAS(pointer_to_destination, pointer_to_compare,        \
                            pointer_to_new_destination, cas_strength, result)  \
  {                                                                            \
    assert(!"LF_PAL_ATOMIC_DWCAS not implemented for this platform.");         \
    (result) = 0;                                                              \
    LF_MISC_DELIBERATELY_CRASH;                                                \
  }
#else
#define LF_MISC_ATOMIC_SUPPORT_DWCAS 1
#endif

#if (!defined LF_PAL_ATOMIC_EXCHANGE)
#define LF_PAL_NO_ATOMIC_EXCHANGE
#define LF_MISC_ATOMIC_SUPPORT_EXCHANGE 0
#define LF_PAL_ATOMIC_EXCHANGE(pointer_to_destination, new_value,              \
                               original_value, value_type)                     \
  {                                                                            \
    assert(!"LF_PAL_ATOMIC_EXCHANGE not implemented for this platform.");      \
    LF_MISC_DELIBERATELY_CRASH;                                                \
  }
#else
#define LF_MISC_ATOMIC_SUPPORT_EXCHANGE 1
#endif

#if (!defined LF_PAL_ATOMIC_SET)
#define LF_PAL_NO_ATOMIC_SET
#define LF_MISC_ATOMIC_SUPPORT_SET 0
#define LF_PAL_ATOMIC_SET(pointer_to_destination, new_value)                   \
  {                                                                            \
    assert(!"LF_PAL_ATOMIC_SET not implemented for this platform.");           \
    LF_MISC_DELIBERATELY_CRASH;                                                \
  }
#else
#define LF__MISC_ATOMIC_SUPPORT_SET 1
#endif

#if (defined LF_PAL_BARRIER_COMPILER_LOAD &&                                   \
     defined LF_PAL_BARRIER_PROCESSOR_LOAD)
#define LF_MISC_BARRIER_LOAD                                                   \
  (LF_PAL_BARRIER_COMPILER_LOAD, LF_PAL_BARRIER_PROCESSOR_LOAD,                \
   LF_PAL_BARRIER_COMPILER_LOAD)
#endif

#if ((!defined LF_PAL_BARRIER_COMPILER_LOAD ||                                 \
      defined LF_PAL_COMPILER_BARRIERS_MISSING_PRESUMED_HAVING_A_GOOD_TIME) && \
     defined LF_PAL_BARRIER_PROCESSOR_LOAD)
#define LF_MISC_BARRIER_LOAD LF_PAL_BARRIER_PROCESSOR_LOAD
#endif

#if (defined LF_PAL_BARRIER_COMPILER_LOAD &&                                   \
     !defined LF_PAL_BARRIER_PROCESSOR_LOAD)
#define LF_MISC_BARRIER_LOAD LF_PAL_BARRIER_COMPILER_LOAD
#endif

#if (!defined LF_PAL_BARRIER_COMPILER_LOAD &&                                  \
     !defined LF_PAL_BARRIER_PROCESSOR_LOAD)
#define LF_MISC_BARRIER_LOAD
#endif

#if (defined LF_PAL_BARRIER_COMPILER_STORE &&                                  \
     defined LF_PAL_BARRIER_PROCESSOR_STORE)
#define LF_MISC_BARRIER_STORE                                                  \
  (LF_PAL_BARRIER_COMPILER_STORE, LF_PAL_BARRIER_PROCESSOR_STORE,              \
   LF_PAL_BARRIER_COMPILER_STORE)
#endif

#if ((!defined LF_PAL_BARRIER_COMPILER_STORE ||                                \
      defined LF_PAL_COMPILER_BARRIERS_MISSING_PRESUMED_HAVING_A_GOOD_TIME) && \
     defined LF_PAL_BARRIER_PROCESSOR_STORE)
#define LF_MISC_BARRIER_STORE LF_PAL_BARRIER_PROCESSOR_STORE
#endif

#if (defined LF_PAL_BARRIER_COMPILER_STORE &&                                  \
     !defined LF_PAL_BARRIER_PROCESSOR_STORE)
#define LF_MISC_BARRIER_STORE LF_PAL_BARRIER_COMPILER_STORE
#endif

#if (!defined LF_PAL_BARRIER_COMPILER_STORE &&                                 \
     !defined LF_PAL_BARRIER_PROCESSOR_STORE)
#define LF_MISC_BARRIER_STORE
#endif

#if (defined LF_PAL_BARRIER_COMPILER_FULL &&                                   \
     defined LF_PAL_BARRIER_PROCESSOR_FULL)
#define LF_MISC_BARRIER_FULL                                                   \
  (LF_PAL_BARRIER_COMPILER_FULL, LF_PAL_BARRIER_PROCESSOR_FULL,                \
   LF_PAL_BARRIER_COMPILER_FULL)
#endif

#if ((!defined LF_PAL_BARRIER_COMPILER_FULL ||                                 \
      defined LF_PAL_COMPILER_BARRIERS_MISSING_PRESUMED_HAVING_A_GOOD_TIME) && \
     defined LF_PAL_BARRIER_PROCESSOR_FULL)
#define LF_MISC_BARRIER_FULL LF_PAL_BARRIER_PROCESSOR_FULL
#endif

#if (defined LF_PAL_BARRIER_COMPILER_FULL &&                                   \
     !defined LF_PAL_BARRIER_PROCESSOR_FULL)
#define LF_MISC_BARRIER_FULL LF_PAL_BARRIER_COMPILER_FULL
#endif

#if (!defined LF_PAL_BARRIER_COMPILER_FULL &&                                  \
     !defined LF_PAL_BARRIER_PROCESSOR_FULL)
#define LF_MISC_BARRIER_FULL
#endif

#if ((defined LF_PAL_BARRIER_COMPILER_LOAD &&                                  \
      defined LF_PAL_BARRIER_COMPILER_STORE &&                                 \
      defined LF_PAL_BARRIER_COMPILER_FULL) ||                                 \
     (defined LF_PAL_COMPILER_BARRIERS_MISSING_PRESUMED_HAVING_A_GOOD_TIME))
#define LF_MISC_ATOMIC_SUPPORT_COMPILER_BARRIERS 1
#else
#define LF_MISC_ATOMIC_SUPPORT_COMPILER_BARRIERS 0
#endif

#if (defined LF_PAL_BARRIER_PROCESSOR_LOAD &&                                  \
     defined LF_PAL_BARRIER_PROCESSOR_STORE &&                                 \
     defined LF_PAL_BARRIER_PROCESSOR_FULL)
#define LF_MISC_ATOMIC_SUPPORT_PROCESSOR_BARRIERS 1
#else
#define LF_MISC_ATOMIC_SUPPORT_PROCESSOR_BARRIERS 0
#endif

#define LF_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_LOGICAL_CORE \
  LF_MISC_BARRIER_LOAD
#define LF_MISC_FLUSH                                                          \
  {                                                                            \
    LF_MISC_BARRIER_STORE;                                                     \
    lf_misc_force_store();                                                     \
  }

/***** enums *****/
enum lf_misc_cas_strength {
  // TRD : GCC defined values
  LF_MISC_CAS_STRENGTH_STRONG = 0,
  LF_MISC_CAS_STRENGTH_WEAK = 1,
};

enum lf_misc_validity {
  LF_MISC_VALIDITY_UNKNOWN,
  LF_MISC_VALIDITY_VALID,
  LF_MISC_VALIDITY_INVALID_LOOP,
  LF_MISC_VALIDITY_INVALID_MISSING_ELEMENTS,
  LF_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS,
  LF_MISC_VALIDITY_INVALID_TEST_DATA,
  LF_MISC_VALIDITY_INVALID_ORDER,
  LF_MISC_VALIDITY_INVALID_ATOMIC_FAILED,
  LF_MISC_VALIDITY_INDETERMINATE_NONATOMIC_PASSED,
};

enum lf_misc_flag { LF_MISC_FLAG_LOWERED, LF_MISC_FLAG_RAISED };

enum lf_misc_query { LF_MISC_QUERY_GET_BUILD_AND_VERSION_STRING };

enum lf_misc_data_structure {
  LF_MISC_DATA_STRUCTURE_BTREE_AU,
  LF_MISC_DATA_STRUCTURE_FREELIST,
  LF_MISC_DATA_STRUCTURE_HASH_A,
  LF_MISC_DATA_STRUCTURE_LIST_AOS,
  LF_MISC_DATA_STRUCTURE_LIST_ASU,
  LF_MISC_DATA_STRUCTURE_QUEUE_BMM,
  LF_MISC_DATA_STRUCTURE_QUEUE_BSS,
  LF_MISC_DATA_STRUCTURE_QUEUE_UMM,
  LF_MISC_DATA_STRUCTURE_RINGBUFFER,
  LF_MISC_DATA_STRUCTURE_STACK,
  LF_MISC_DATA_STRUCTURE_COUNT
};

/***** struct *****/
struct lf_prng_state {
  uint64_t volatile LF_PAL_ALIGN(LF_PAL_ATOMIC_ISOLATION_IN_BYTES) entropy;
};

struct lf_misc_backoff_state {
  // LF_PAL_ATOMIC_ISOLATION_IN_BYTES was 128
  uint64_t volatile LF_PAL_ALIGN(128) lock;

  uint64_t backoff_iteration_frequency_counters[2], metric, total_operations;
};

struct lf_misc_globals {
  struct lf_prng_state ps;
};

struct lf_misc_validation_info {
  uint64_t min_elements, max_elements;
};

/***** externs *****/
extern struct lf_misc_globals lf_misc_globals;

/***** public prototypes *****/
static LF_PAL_INLINE void lf_misc_force_store(void);

void lf_misc_query(enum lf_misc_query query_type, void *query_input,
                   void *query_output);

/***** public in-line functions *****/
#pragma prefast(disable : 28112, "blah")

static LF_PAL_INLINE void lf_misc_force_store() {
  // LF_PAL_ATOMIC_ISOLATION_IN_BYTES was 128
  uint64_t volatile LF_PAL_ALIGN(128) destination;

  LF_PAL_ATOMIC_SET(&destination, 0);

  return;
}
