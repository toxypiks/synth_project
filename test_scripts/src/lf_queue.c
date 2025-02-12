#include "lf_queue.h"
#include "lf_misc.h"
#include <assert.h>

void lf_queue_init(struct lf_queue_bss_state *qbsss,
                   struct lf_queue_bss_element *element_array,
                   uint64_t number_elements)
{
  assert(qbsss != NULL);
  assert(element_array != NULL);
  assert(number_elements >= 2);
  assert((number_elements & (number_elements - 1)) ==
         0); // TRD : number_elements must be a positive integer power of 2
  /* TRD : the use of mask and the restriction on a power of two
           upon the number of elements bears some remark

           in this queue, there are a fixed number of elements
           we have a read index and a write index
           when we write, and there is space to write, we increment the write
     index (if no space to write, we just return) when we read, and there are
     elements to be read, we after reading increment the read index (if no
     elements to read, we just return) the problem is - how do we handle wrap
     around? e.g. when I write, but my write index is now equal to the number of
     elements the usual solution is to modulus the write index by the nunmber of
     elements problem is modulus is slow there is a better way first, we
     restrict the number of elements to be a power of two so imagine we have a
     64-bit system and we set the number of elements to be 2^64 this gives us a
     bit pattern of 1000 0000 0000 0000 (...etc, lots of zeros) now (just roll
     with this for a bit) subtract one from this this gives us a mask (on a
     two's compliment machine) 0111 1111 1111 1111 (...etc, lots of ones) so
     what we do now, when we increment an index (think of the write index as the
     example) we bitwise and it with the mask now think about thwt happens all
     the numbers up to 2^64 will be unchanged - their MSB is never set, and we
     and with all the other bits but when we finally hit 2^64 and need to roll
     over... bingo! we drop MSB (which we finally have) and have the value 0!
           this is exactly what we want
           bitwise and is much faster than modulus
  */

  qbsss->number_elements = number_elements;
  qbsss->mask = qbsss->number_elements - 1;
  qbsss->read_index = 0;
  qbsss->write_index = 0;
  qbsss->element_array = element_array;

  LF_MISC_BARRIER_STORE;

  lf_misc_force_store();

  return;
}

void lf_queue_cleanup(
    struct lf_queue_bss_state *qbsss,
    void (*element_cleanup_callback)(struct lf_queue_bss_state *qbsss,
                                     void *key, void *value)) {
  uint64_t loop;

  struct lf_queue_bss_element *qbsse;

  assert(qbsss != NULL);
  // TRD : element_cleanup_callback can be NULL

  if (element_cleanup_callback != NULL)
    for (loop = qbsss->read_index;
         loop < qbsss->read_index + qbsss->number_elements; loop++) {
      qbsse = qbsss->element_array + (loop % qbsss->number_elements);
      element_cleanup_callback(qbsss, qbsse->key, qbsse->value);
    }

  return;
}

int lf_queue_pop(struct lf_queue_bss_state *qbsss,
                 void **key,
                 void **value)
{
  struct lf_queue_bss_element *qbsse;

  assert(qbsss != NULL);
  // TRD : key can be NULL
  // TRD : value can be NULL

  LF_MISC_BARRIER_LOAD;

  if (qbsss->read_index != qbsss->write_index) {
    qbsse = qbsss->element_array + qbsss->read_index;

    if (key != NULL)
      *key = qbsse->key;

    if (value != NULL)
      *value = qbsse->value;

    qbsss->read_index = (qbsss->read_index + 1) & qbsss->mask;

    LF_MISC_BARRIER_STORE;

    return 1;
  }

  return 0;
}

int lf_queue_push(struct lf_queue_bss_state *qbsss,
                  void *key,
                  void *value)
{
  struct lf_queue_bss_element *qbsse;

  assert(qbsss != NULL);
  // TRD : key can be NULL
  // TRD : value can be NULL

  LF_MISC_BARRIER_LOAD;

  if (((qbsss->write_index + 1) & qbsss->mask) != qbsss->read_index) {
    qbsse = qbsss->element_array + qbsss->write_index;

    qbsse->key = key;
    qbsse->value = value;

    LF_MISC_BARRIER_STORE;

    qbsss->write_index = (qbsss->write_index + 1) & qbsss->mask;

    return 1;
  }

  return 0;
}

static void lf_queue_bss_internal_validate(struct lf_queue_bss_state *qbsss,
                                           struct lf_misc_validation_info *vi,
                                           enum lf_misc_validity *lf_validity) {
  assert(qbsss != NULL);
  // TRD : vi can be NULL
  assert(lf_validity != NULL);

  *lf_validity = LF_MISC_VALIDITY_VALID;

  if (vi != NULL) {
    uint64_t number_elements;

    lf_queue_query(qbsss, LF_QUEUE_BSS_QUERY_GET_POTENTIALLY_INACCURATE_COUNT,
                   NULL, (void *)&number_elements);

    if (number_elements < vi->min_elements)
      *lf_validity = LF_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if (number_elements > vi->max_elements)
      *lf_validity = LF_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

void lf_queue_query(struct lf_queue_bss_state *qbsss,
                    enum lf_queue_query query_type,
                    void *query_input,
                    void *query_output) {
  assert(qbsss != NULL);
  // TRD : query_type can be any value in its range

  switch (query_type) {
  case LF_QUEUE_BSS_QUERY_GET_POTENTIALLY_INACCURATE_COUNT: {
    uint64_t local_read_index, local_write_index;

    assert(query_input == NULL);
    assert(query_output != NULL);

    LF_MISC_BARRIER_LOAD;

    local_read_index = qbsss->read_index;
    local_write_index = qbsss->write_index;

    *(uint64_t *)query_output = +(local_write_index - local_read_index);

    if (local_read_index > local_write_index)
      *(uint64_t *)query_output =
          qbsss->number_elements - *(uint64_t *)query_output;
  } break;

  case LF_QUEUE_BSS_QUERY_VALIDATE:
    // TRD : query_input can be NULL
    assert(query_output != NULL);

    lf_queue_bss_internal_validate(
        qbsss, (struct lf_misc_validation_info *)query_input,
        (enum lf_misc_validity *)query_output);
    break;
  }

  return;
}
