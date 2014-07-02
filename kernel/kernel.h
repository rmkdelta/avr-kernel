/******************************************************************************
  avr-kernel
  Copyright (C) 2014 Michael Crawford

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
******************************************************************************/

/** \file
 * \brief Documents the main interface of the kernel.
 * \see kernel_interface
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include "config.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * \defgroup kernel_interface Kernel Interface
 * \brief Contains the public interface of the kernel.
 * 
 * @{
 */

/** Defines an explicit type to use for thread id's. */
typedef uint8_t thread_id;

/** The id for the first thread. */
#define THREAD0 0
/** The id for the second thread. */
#define THREAD1 1
/** The id for the third thread. */
#define THREAD2 2
/** The id for the fourth thread. */
#define THREAD3 3
/** The id for the fifth thread. */
#define THREAD4 4
/** The id for the sixth thread. */
#define THREAD5 5
/** The id for the seventh thread. */
#define THREAD6 6
/** The id for the eighth thread. */
#define THREAD7 7

/**
 * The function type for threads used by the kernel.  To reduce code size and 
 * unnecessary stack usage, thread functions should be given the gcc attribute 
 * \c OS_task. While thread functions do not need to worry about saving or 
 * restoring any registers, it is not recommended to give them the \c naked 
 * attribute, because the compiler may generate code that assumes a prologue 
 * has set up the stack.
 * 
 * \param[in] my_id The thread id of this thread.
 * \param[in] arg A parameter to pass information to the thread.
 * 
 * \warning Do not allow any thread to return: doing so will break things...
 */
typedef void (*thread_ptr)(const thread_id my_id, void* arg);

/**
 * Creates a new thread of operation within the kernel.
 * 
 * \param[in] t_id The id of the new thread. If the thread id is an enabled 
 * thread, that thread will be replaced.
 * \param[in] entry_point The function that will be run as the new thread.
 * \param[in] suspended The initial state of the new thread. If true, the 
 * thread will not run until it is manually resumed.
 * \param[in] arg The parameter that will be passed to the function.
 * 
 * \warning If \c t_id is the currently active thread, this function does not 
 * return.
 */
extern void kn_create_thread(const thread_id t_id, thread_ptr entry_point, 
                             const bool suspended, void* arg);
/**
 * Replaces the calling thread with a new thread. Basically is just a wrapper 
 * for the \ref kn_create_thread that automatically supplies the id of the 
 * calling thread. See \ref kn_create_thread for behavior and parameter info.
 * 
 * \warning Does not return.
 * \see kn_create_thread
 */
static inline void kn_replace_self(thread_ptr entry_point, const bool suspended,
                                   void* arg);
  
/**
 * Allows a thread to yield execution to the scheduler.  Will return when the 
 * scheduler selects the calling thread for execution again.
 */
extern void kn_yield();

/**
 * Allows a thread to sleep for a certain amount of time. For sleep times 
 * longer than 65 seconds, use \ref kn_sleep_long.
 * 
 * \param[in] millis The amount of time in milliseconds to sleep.
 */
extern void kn_sleep(const uint16_t millis);

/**
 * Allows a thread to sleep for longer time periods than \ref kn_sleep. The 
 * total sleep time is <tt>secs*1000+millis</tt> milliseconds. For example, to 
 * sleep for 70.25 seconds, pass <tt>secs=70</tt> and <tt>millis=250</tt>. The 
 * maximum sleep time is approximately 136 years.
 * 
 * \param[in] secs The number of seconds to sleep.
 * \param[in] millis The number of milliseconds to sleep.
 */
extern void kn_sleep_long(uint32_t secs, uint16_t millis);

/**
 * Returns the system timer, in milliseconds. This value will overflow after 
 * 49 days.
 */
extern uint32_t kn_millis();

/**
 * Returns the id of the currently running thread.
 */
static inline thread_id kn_current_thread();

/**
 * Returns true if the specified thread is currently enabled. This does not 
 * necessarily mean that the thread is currently active, as it may be 
 * suspended or sleeping.
 */
extern bool kn_thread_enabled(const thread_id t_id);

/**
 * Returns true if the specified thread is enabled, but suspended.
 */
extern bool kn_thread_suspended(const thread_id t_id);

/**
 * Returns true if the specified thread is enabled, but currently sleeping.
 */
extern bool kn_thread_sleeping(const thread_id t_id);

/**
 * Disables the specified thread. If \c t_id is invalid, does nothing. After a 
 * thread has been disabled, you must call \ref kn_create_thread to restart it 
 * or replace it with a new thread. If you wish to "pause" a thread with the 
 * ability to resume it at a later time, use \ref kn_suspend.
 * 
 * \warning If kn_disable is the calling thread, this function does not return.
 */
extern void kn_disable(const thread_id t_id);

/**
 * Disables the calling thread.
 * 
 * \warning Does not return.
 * \see kn_disable
 */
static inline void kn_disable_self();

/**
 * Resumes the specified thread, so that the scheduler may select it for 
 * execution.
 */
extern void kn_resume(const thread_id t_id);

/**
 * Suspends the specified thread. If t_id is the calling thread, this function 
 * yields and will return after the thread has been resumed and selected for 
 * execution again.
 */
extern void kn_suspend(const thread_id t_id);

/**
 * Suspends the calling thread.
 * 
 * \see kn_suspend
 */
static inline void kn_suspend_self();

/**
 * Converts a bit number to a bit mask.  For example, bit 0 produces the mask 
 * 0x01.  Implemented using a lookup table for better performance than a loop 
 * with bitwise operations. Technically this function isn't part of the kernel, 
 * it's just made available since it provides a commonly used functionality.
 * 
 * \param[in] bit_num The bit number to be converted to a mask. Zero-indexed. 
 * Valid values are in the range [0,7].
 * 
 * \return The bit mask corresponding to \c bit_num.
 */
static inline uint8_t bit_to_mask(uint8_t bit_num) __attribute__((pure));

#ifdef KERNEL_USE_ASSERT
/**
 * A user supplied function that is called when an assertion fails. Used only 
 * if \ref KERNEL_USE_ASSERT is defined.
 * 
 * \param[in] expr The assertion expression that failed.
 * \param[in] file The name of the file where the assertion failed.
 * \param[in] base_file The file being compiled when the assertion failed.
 * \param[in] line The line number of \c file where the assertion failed.
 */
extern void kn_assertion_failure(const char* expr, const char* file,
                                 const char* base_file, const int line);
#endif

#ifdef KERNEL_USE_STACK_CANARY
/**
 * A user supplied function that is called when a stack overflow is detected. 
 * Used only if \ref KERNEL_USE_STACK_CANARY is defined.
 */
extern void kn_stack_overflow(const thread_id t_id);
#endif

/**
 * @}
 */

/** \cond */
#ifndef NULL  
  #define NULL ((void*)0)
#endif

#ifdef KERNEL_USE_ASSERT
  #define KERNEL_ASSERT(expr) \
    do { \
      if (!(expr)) \
        kn_assertion_failure(#expr, __FILE__, __BASE_FILE__, __LINE__); \
    } while (0)
#else
  #define KERNEL_ASSERT(expr) ((void)0)
#endif
/** \endcond */

// inline function definitions
#include "core/kernel-inl.h"

#endif