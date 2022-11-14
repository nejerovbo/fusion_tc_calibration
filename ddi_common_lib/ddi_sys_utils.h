/*****************************************************************************
 * (c) Copyright 2019 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_sys_utils.h
 *  Created by Johana Lehrer on 2019-07-10
 */

#ifndef _DDI_SYS_UTILS_H
#define _DDI_SYS_UTILS_H

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/** system_command
 @brief Executes a system call @see SYSTEM(3) specified by the 'cmd' string
 with the following, optional, convenience functions:

 1) When 'timeout_usec' is specified as (non-zero):
 The system call has not completed by the specified timeout it is terminated.

 2) When 'ppid' is specified:
 The system call is executed in a child process which can be terminated
 by passing the returned pid_t to the system_command_kill function.
 NOTE: if the calling process has registered its own system_signal_handler
 then it is the responsibility of the calling process to call the
 system_command_kill function with the returned pid_t from its
 exit handler to terminate the child process.

 3) When 'output_data' and 'p_len' are specified:
 The stdout of the system call child process is returned in the buffer
 pointed to by the 'output_data' parameter. The output data will not exceed
 the value which 'p_len' points to upon entry to the system_call function.
 On exit the value which 'p_len' points to is set to the actual number of
 bytes read from the stdout.

 Combinations of 1, 2, 3 are permissable. Behavior is as follows:
 timeout, pid, output
 1)    0,   0,   0   system                  : blocks calling thread
 2)    0,   0,   1   system_with_output      : blocks calling thread : output returned
 3)    0,   1,   0   fork system             : blocks calling thread : but another thread may kill child process
 4)    0,   1,   1   fork system_with_output : blocks calling thread : output returned
 5)    1,   0,   0   fork system             : blocks with timeout
 6)    1,   0,   1   fork system_with_output : blocks with timeout   : output returned
 7)    1,   1,   0   fork system             : blocks with timeout   : but another thread may kill child process
 8)    1,   1,   1   fork system_with_output : blocks with timeout   : but another thread may kill child process : output returned

 @param cmd The command string to be executed
 @param timeout_usec Specifies a timeout after which if the child process has not completed it is terminated.
 @param output_data Pointer to a memory buffer which receives the child process stdout output
 @param p_len Upon entry specifies the size of the 'output_data' buffer. Upon exit receives the size of the stdout data from the child process
 @return The return code from the system call.
 */
int system_command(const char *cmd, uint32_t timeout_usec, pid_t *ppid, uint8_t *output_data, size_t *p_len);

/** system_command_kill
 @brief Kills a child process spawned from a call to system_command with the 'ppid' param specified.
 */
void system_command_kill(pid_t pid);

/** sig_handler_t
 Prototype of the signal handler callback function registered with the system_signal_handler.
 */
typedef void (sig_handler_t)(int sig_number);

/** system_signal_handler
 Registers a signal callback handler function which is called when any signals in 'sigmask' occur.
 NOTE: only one callback handler exists for a given process.
 @param handler User callback function for the specified signal
 @param sigmask A bitmask of the signals to register for: e.g. ((1 << (SIGINT-1)) || (1 << (SIGPIPE-1)))
 @return 0 on success; -1 otherwise
 */
int system_signal_handler(sig_handler_t *handler, uint32_t sigmask);

#ifdef __cplusplus
}
#endif

#endif // _DDI_SYS_UTILS_H

