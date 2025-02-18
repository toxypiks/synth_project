#ifndef LF_QUEUE_H_
#define LF_QUEUE_H_

#include <inttypes.h>
#include <stddef.h>

/***** enums *****/
enum lf_queue_query {
  LF_QUEUE_BSS_QUERY_GET_POTENTIALLY_INACCURATE_COUNT,
  LF_QUEUE_BSS_QUERY_VALIDATE
};

/***** structures *****/
typedef struct lf_queue_bss_element {
  void *volatile key, *volatile value;
} lf_queue_bss_element;

typedef struct lf_queue_bss_state {
  uint64_t number_elements, mask;

  uint64_t volatile read_index, write_index;

  lf_queue_bss_element *element_array;
} lf_queue_bss_state;

/***** public prototypes *****/
void lf_queue_init(lf_queue_bss_state *qbsss,
                   lf_queue_bss_element *element_array,
                   uint64_t number_elements);
// TRD : number_elements must be a positive integer power of 2
// TRD : used in conjunction with the #define
// LF_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_LOGICAL_CORE

void lf_queue_cleanup(
    lf_queue_bss_state *qbsss,
    void (*element_cleanup_callback)(lf_queue_bss_state *qbsss,
                                     void *key, void *value));

int lf_queue_push(lf_queue_bss_state *qbsss, void *key, void *value);

int lf_queue_pop(lf_queue_bss_state *qbsss, void **key, void **value);

void lf_queue_query(struct lf_queue_bss_state *qbsss,
                    enum lf_queue_query query_type, void *query_input,
                    void *query_output);

#endif // LF_QUEUE_H_
