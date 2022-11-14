/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

int telnet_open(const char *IP, int port);
int telnet_close(void);
void telnet_verbose(int verbose);
void telnet_send_str(const char *str);
int telnet_gets(char *buffer, int buffer_size);


void get_input(int display_it);
void wait_for_echo(const char *sent);
void wait_for_cr(void);
void wait_for_close(int display_it);
void telnet_echo_off(void);
void telnet_echo_on(void);

#ifdef __cplusplus
}
#endif
