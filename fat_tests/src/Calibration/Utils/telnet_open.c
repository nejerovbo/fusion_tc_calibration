/*
 telnet_open.c
 For opening a connection to a telnet server (as opposed to the more common "raw" socket)
*/

#include "indsock.h"
#include "telnet_open.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


void error_and_die(char *errmsg);


int opt_verbose = 0;
int gSock;

#define DDprintf(...)
//#define DDprintf printf

/*
   The following are the defined TELNET commands.  Note that these codes
   and code sequences have the indicated meaning only when immediately
   preceded by an IAC.

      NAME               CODE              MEANING

      SE                  240    End of subnegotiation parameters.
      NOP                 241    No operation.
      Data Mark           242    The data stream portion of a Synch.
                                 This should always be accompanied
                                 by a TCP Urgent notification.
      Break               243    NVT character BRK.
      Interrupt Process   244    The function IP.
      Abort output        245    The function AO.
      Are You There       246    The function AYT.
      Erase character     247    The function EC.
      Erase Line          248    The function EL.
      Go ahead            249    The GA signal.
      SB                  250    Indicates that what follows is
                                 subnegotiation of the indicated
                                 option.
      WILL (option code)  251    Indicates the desire to begin
                                 performing, or confirmation that
                                 you are now performing, the
                                 indicated option.
      WON'T (option code) 252    Indicates the refusal to perform,
                                 or continue performing, the
                                 indicated option.
      DO (option code)    253    Indicates the request that the
                                 other party perform, or
                                 confirmation that you are expecting
                                 the other party to perform, the
                                 indicated option.
      DON'T (option code) 254    Indicates the demand that the
                                 other party stop performing,
                                 or confirmation that you are no
                                 longer expecting the other party
                                 to perform, the indicated option.
      IAC                 255    Data Byte 255.

List of telnet  options can be found at;
http://www.thoughtproject.com/libraries/telnet/Reference/Options.htm

It seems that if I just answer "won't" to everything the server says,
it (eventually) gives me the "login: " (or whatever) prompt, 
but annoyingly, it doesn't send any sort of "I'm done" signal - 
I guess getting any non-command character is it.
*/

void telnet_send_str(const char *str)
{
    int len;
    int r;

    len = strlen(str);
    r = send(gSock, str, len, 0);
    if (r < 0) {
        error_and_die("");
    }
}

static unsigned char sock_buffer[2000];
static int sock_buffer_length = 0;
unsigned char *sock_buffer_p;

int tcp_getc()
{
    int rlen;

    while (sock_buffer_length <= 0)
    {
        rlen = recv(gSock, (char *) sock_buffer, sizeof(sock_buffer), 0);
        if (rlen < 0) {
            perror("recv");
            error_and_die("recv");
            return -1;
        }
        if (rlen == 0) {
            return EOF;
        }
        sock_buffer_p = sock_buffer;
        sock_buffer_length = rlen;
    }
    sock_buffer_length--;
    return *sock_buffer_p++;
}

/*
 see if bytes are available to read, etc.
 */
int tcp_status(int timeout_msec) {
    return socket_status(gSock, timeout_msec);
}

/* return >0 if bytes are available */
int tcp_bytes_available(int timeout_msec) {
    int st;

    if (sock_buffer_length > 0) {
        return sock_buffer_length;
    }
    st = tcp_status(timeout_msec);
    if (st < 0)
        DDprintf("Status %d\n", st);
    return st;
}

// Get a character from telnet, 
//  Respond to all requests with 252 (Won't)
//  Requests are 253 (Do) and 254 (Don't)
//  ignore all others.
//
int telnet_getc(void) {
    char wont[4];
    //char will[4];
    int ch;
    int command;
    int opt;

    wont[0] = 255;
    wont[1] = 252;
    wont[2] = 0;
    wont[3] = 0;

    for (;;) {
        ch = tcp_getc();
        if (ch == 255) {
            //command comming
            command = tcp_getc();
            opt = tcp_getc();
            DDprintf("> %d %d %d ", ch, command, opt);
            switch(command) {
                case 240:
                case 241:
                case 242:
                case 243:
                case 244:
                case 245:
                case 246:
                case 247:
                case 248:
                case 249:
                case 250:
                case 255:
                default:
                    DDprintf("(unhandled)\n");
                break;

                case 251: //Will
                case 252: //Won't
                    DDprintf("\n");
                break;

                case 253: //Do
                case 254: //Don't
                    // Respond with "Won't"
                    wont[2] = opt;
                    telnet_send_str(wont);
                    DDprintf("< %d %d %d\n", 255, 252, opt);
                break;
            }
        } else {
            return ch;
        }
    }
}

char error_buff[800];
int error_buff_index = 0;

void error_and_die(char *errmsg) {
    fprintf(stderr, "%s\n", error_buff);
    fprintf(stderr, "%s\n", errmsg);
    exit(-1);
}

void show(int ch, int display_it) {
    if (ch == EOF) {
        error_and_die("Unexpected error, aborting");
    }
    if (ch == 0) {
        return;
    }
    if (display_it) {
        printf("%c", ch);
        fflush(stdout);
    }
    error_buff[error_buff_index] = ch;
    error_buff[error_buff_index+1] = 0;
    error_buff_index++;
    if (error_buff_index >= (sizeof(error_buff)-1) || (ch == '\n')) {
        error_buff_index=0;
    }
}

void get_input(int display_it) {
    int ch;

    //wait for some bytes to be available.
    while(tcp_bytes_available(100) == 0) {
        ;
    }
    // read everything that is available
    while(tcp_bytes_available(1) > 0) {
        ch = telnet_getc();
        show(ch, display_it);
    }
}

//
// wait for the server to echo back what we've typed. 
//

// this has a subtle bug if the string being waited for
// has repeated sequences.
//  should really buffer a line, and check against that.
//
void wait_for_echo(const char *sent) {
    int j;
    int ch;

    j = 0;
    for(;;) {
        ch = telnet_getc();
        if (ch == 0) {
            j = 0;
            continue;
        }
        show(ch, opt_verbose);
        if (ch != sent[j]) {
            j = 0;
            continue;
        }
        j++;
        if (sent[j] == 0) 
            break;
    }
}

void wait_for_echo_cr(char *sent) {
    int ch;

    for(;;) {
        wait_for_echo(sent);
        ch = telnet_getc();
        show(ch, opt_verbose);
        if (ch == '\n') 
            break;
    }
}

void wait_for_cr(void) {
    int ch;

    for(;;) {
        ch = telnet_getc();
        show(ch, opt_verbose);
        if (ch == '\n') 
            return;
    }
}

void wait_for_close(int display_it) {
    int ch;

    for(;;) {
        ch = telnet_getc();
        if (ch == EOF)
            break;
        show(ch, display_it);
    }
}

// echo off/on
// attempt to disable/enable echo (full duplex)
//

void telnet_echo_off(void) {
    telnet_send_str("\377\376\001");  //IAC, Don't, Echo
}

void telnet_echo_on(void) {
    telnet_send_str("\377\375\001");  //IAC, Do, Echo
}


#define BBSIZE 32000
char big_buff[BBSIZE];
int bblen = 0;

void flush_buffer(void) {
    int r;

    r = send(gSock, big_buff, bblen, 0);
    if (r < 0) {
        error_and_die("");
    }
    bblen = 0;
}

void buffered_telnet_send_str(char *str) {
    int len;

    len = strlen(str);
    if (len + bblen > sizeof(big_buff)) {
        flush_buffer();
    }

    strcpy(&big_buff[bblen], str);
    bblen += len;
}

int telnet_gets(char *buffer, int buffer_size) {
    int j;
    int ch;

    for(j=0; j < (buffer_size-1); ) {
        ch = telnet_getc();
	if (ch == '\r') {
	    continue;
	}
	if (ch == '\n') {
	    buffer[j] = 0;
	    return j;
	}
	buffer[j] = ch;
	j++;
    }
    buffer[j] = 0;
    return j;
}

int telnet_open(const char *IP, int port) {
    sock_buffer_length = 0;
    gSock = connect_to_host(IP, port);
    //printf("Debug gSock = %d\n", gSock);
    return gSock;
}

void telnet_verbose(int verbose) {
    opt_verbose = verbose;
}

int telnet_close(void) {
    close_connection(gSock);
    return 0;
}
