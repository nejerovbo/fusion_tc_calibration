#ifndef DDI_ATOMIC_H
#define DDI_ATOMIC_H

#ifdef _WIN32
#define _WINSOCKAPI_
#include <Windows.h>

static inline int _ddi_atomic_compare_exchange(volatile LONG *ptarget, volatile LONG *pexpected, LONG desired)
{
  LONG tmp = InterlockedCompareExchange(ptarget, desired, *pexpected);
  if (tmp == *pexpected)
    return 1;
  *pexpected = tmp;
  return 0;
}

#define ddi_atomic_compare_exchange(ptarget, pexpected, desired) _ddi_atomic_compare_exchange((volatile LONG *)(ptarget), (volatile LONG *)(pexpected), desired)
#define ddi_atomic_and(ptarget,mask) (InterlockedAnd((volatile LONG*)(ptarget),(mask)) & (mask))
#define ddi_atomic_or(ptarget,mask) (InterlockedAnd((volatile LONG*)(ptarget),(mask)) | (mask))

#else // GCC: https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html

/** ddi_atomic_compare_exchange
  performs an atomic compare and exchange operation:

  if (*target == *pexpected) {
    *target = desired;
    return true;
  } else {
    *pexpected = *target;
    return false;
  }
 @returns true if the operation succeeded; false otherwise.
 */
#define ddi_atomic_compare_exchange(ptarget,pexpected,desired) (__atomic_compare_exchange_n((volatile uint32_t*)(ptarget), (pexpected), (desired), 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))

/** ddi_atomic_and
  performs the atomic boolean operation:

  *target &= mask;

  @returns the previous value of *target.
*/
#define ddi_atomic_and(ptarget,mask) (__atomic_fetch_and((volatile uint32_t*)(ptarget),(mask), __ATOMIC_SEQ_CST))

/** ddi_atomic_or
 performs the boolean operation:

 *target |= mask;

 @returns the previous value of *target.
 */
#define ddi_atomic_or(ptarget,mask) (__atomic_fetch_or((volatile uint32_t*)(ptarget),(mask), __ATOMIC_SEQ_CST))
#endif


/** ddi_atomic_compare_swap
 Convenience function to perform an atomic compare and swap operation with 'expected' as a param instead of a pointer:

  if (*target == *pexpected) {
    *target = desired;
    return true;
  } else {
    return false;
  }
 @returns true if the operation succeeded; false otherwise.
 */
static inline int ddi_atomic_compare_swap(volatile uint32_t *ptarget, uint32_t expected, uint32_t desired)
{
  return ddi_atomic_compare_exchange(ptarget, &expected, desired);
}

#endif // DDI_ATOMIC_H
