
#ifndef _PSP2WPP_H_
#define _PSP2WPP_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _psp2wpp_comm_packet {
	uint32_t sequence;
	uint32_t cmd;
	int result;
	int size;
} psp2wpp_comm_packet;


int psp2wpp_is_connected(void);

int psp2wpp_open_ex(void);
int psp2wpp_open_ex_2(void);
int psp2wpp_close_ex(void);

int psp2wpp_usb_send_ex(const void *data, size_t length, int timeout);
int psp2wpp_usb_recv_ex(void *data, size_t length, int timeout);

int psp2wpp_send_cmd(uint32_t cmd);

#ifdef __cplusplus
}
#endif

#endif /* _PSP2WPP_H_ */
