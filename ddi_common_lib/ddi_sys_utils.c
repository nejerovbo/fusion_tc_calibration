/*****************************************************************************
 * (c) Copyright 2019-2020 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_sys_utils.c
 *  Created by Johana Lehrer on 2019-07-10
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <sys/mman.h>
#include "ddi_sys_utils.h"
#include "ddi_defines.h"
#include "ddi_debug.h"

typedef struct
{
  int done;
  int ret;
  uint8_t *data;
  size_t len;
} cmdstat_t;

static int system_with_output(const char *cmd, uint8_t *output_data, size_t *p_len)
{
  int ret;
  FILE *fp;
  uint8_t *p;
  size_t len, maxlen;

  maxlen = *p_len;
  *p_len = 0;
  p = output_data;
  fp = popen(cmd, "r");
  if (!fp)
  {
    return -1;
  }

  while (maxlen > 0)
  {
    len = fread(p, 1, maxlen, fp);
    if (len == 0)
      break;
    maxlen -= len;
    p += len;
  }

  ret = pclose(fp);
  *p_len = (size_t)(p - output_data);

  return ret;
}


void system_command_kill(pid_t pid)
{
  if (pid)
  {
    kill(-pid, SIGKILL);
  }
}

/*
timeout, pid, output
1)    0,   0,   0   system                  : blocks calling thread
2)    0,   0,   1   system_with_output      : blocks calling thread : output returned
3)    0,   1,   0   fork system             : blocks calling thread : but another thread may kill child process
4)    0,   1,   1   fork system_with_output : blocks calling thread : output returned
5)    1,   0,   0   fork system             : blocks with timeout
6)    1,   0,   1   fork system_with_output : blocks with timeout   : output returned
7)    1,   1,   0   fork system             : blocks with timeout   : but another thread may kill child process
8)    1,   1,   1   fork system_with_output : blocks with timeout   : but another thread may kill child process : output returned
*/

int system_command(const char *cmd, uint32_t timeout_usec, pid_t *p_pid, uint8_t *output_data, size_t *p_len)
{
  static cmdstat_t *status;
  pid_t pid;
  int ret = 2;

  // options: 1 or 2
  if ((timeout_usec == 0) && (p_pid == 0))
  {
    if (output_data && p_len) {
      return system_with_output(cmd, output_data, p_len);
    } else {
      return system(cmd);
    }
  }

  status = (cmdstat_t *)mmap(NULL, sizeof(cmdstat_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (output_data && p_len)
  {
    status->len = *p_len;
    status->data = (uint8_t *)mmap(NULL, *p_len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  }
  else
  {
    status->data = 0;
    status->len = 0;
  }

  status->done = 0; 
  status->ret = 0; 

  pid = fork();
  if (pid == 0) // child process
  {
    setpgid(getpid(), getpid());
    if (status->data && status->len) {
      status->ret = system_with_output(cmd, status->data, &status->len);
    } else {
      status->ret = system(cmd);
    }
    status->done = 1;
    exit(0);
  }
  else // main process
  {
    if (p_pid) {
      *p_pid = pid; // options: 3, 4, 7 or 8 (if they want to kill a hung process)
    }

    if (timeout_usec == 0) // options: 3 or 4 (block the main process indefinitely)
    {
      while (!status->done)
      {
        usleep(100000);
      }
    }
    else // options: 5, 6, 7 or 8
    {
      while (!status->done && timeout_usec)
      {
        int t = MIN(100000, timeout_usec);
        timeout_usec -= t;
        usleep(t);
      }

      if (!status->done && timeout_usec == 0)
      {
        ELOG("command timedout\n");
        if (p_pid) {
          *p_pid = 0;
        }

        kill(-pid, SIGKILL);
        ret = 2;
      }
      else
      {
        ret = status->ret; // copy the child process exit code
      }
    }

    // options: 4, 6 or 8 (copy the child's mmap stdout data back to the caller's output data param)
    if (output_data)
    {
      if (status->len)
      {
        memcpy(output_data, status->data, status->len);
      }

      munmap(status->data, *p_len); // unmap the shared data buffer
      *p_len = status->len;         // then update the return output length param
    }

    ret = status->ret;
    munmap(status, sizeof(cmdstat_t));
  }

  return ret;
}

static sig_handler_t *sig_handler = 0;
static uint32_t sig_mask = 0;

static void handle_signal_action(int sig_number, siginfo_t *info, void *ctx)
{
//  ucontext_t *context = (ucontext_t *)ctx;

  VLOG("sig_number: %d fd: %d\n", sig_number, info ? info->si_fd : -1);

  if (sig_handler)
  {
    if (sig_mask & (1 << (sig_number -1)))
    {
      sig_handler(sig_number);
    }
  }
}

int system_signal_handler(sig_handler_t *handler, uint32_t sigmask)
{
  int sig;
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));

  sa.sa_sigaction = handle_signal_action;
  sa.sa_flags = SA_RESTART | SA_SIGINFO;
  sigemptyset(&sa.sa_mask);

  for (sig = 1; sig <= 32; sig++)
  {
    if (sigmask & (1 << (sig - 1)))
    {
      //printf("sig: 0x%02x\n", sig);

      if (sigaction(sig, &sa, 0) != 0) {
        perror("sigaction()");
        return -1;
      }
    }
  }

  sig_mask = sigmask;
  sig_handler = handler;

  return 0;
}

