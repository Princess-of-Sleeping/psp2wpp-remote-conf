
#include <stdio.h>
#include <libusb.h>
#include "psp2wpp.h"


#define ACM_CTRL_DTR 0x1
#define ACM_CTRL_RTS 0x2

libusb_device_handle *psp2wpp_open_core(uint16_t vid, uint16_t pid){

	int res;
	libusb_device_handle *hDev = NULL;

	hDev = libusb_open_device_with_vid_pid(NULL, vid, pid);
	if(hDev == NULL){
		return NULL;
	}

	do {
		res = libusb_claim_interface(hDev, 0);
		if(res < 0){
			fprintf(stderr, "Error claiming interface: %s\n", libusb_error_name(res));
			break;
		}

		do {
			res = libusb_claim_interface(hDev, 1);
			if(res < 0){
				fprintf(stderr, "Error claiming interface: %s\n", libusb_error_name(res));
				break;
			}

			do {
				res = libusb_control_transfer(hDev, 0x21, 0x22, ACM_CTRL_DTR | ACM_CTRL_RTS, 0, NULL, 0, 0);
				if(res < 0){
					fprintf(stderr, "Error during control transfer: %s\n", libusb_error_name(res));
					break;
				}

				return hDev;
			} while(0);

			libusb_release_interface(hDev, 1);
		} while(0);

		libusb_release_interface(hDev, 0);
	} while(0);

	libusb_close(hDev);

	return NULL;
}

libusb_device_handle *psp2wpp_open(uint16_t vid, uint16_t pid){

	libusb_device_handle *hDev = NULL;

	int res;
	int retry = 5;

	while(1){
		do {
			hDev = libusb_open_device_with_vid_pid(NULL, vid, pid);
			sleep(1);
			retry--;
		} while(hDev == NULL && retry != 0);

		if(hDev == NULL){
			// sleep(1);
			// continue;
			break;
		}

		do {
			for(int if_num=0;if_num<2;if_num++){
				// detach kernel driver, if attached
				if(libusb_kernel_driver_active(hDev, if_num)){
					libusb_detach_kernel_driver(hDev, if_num);
				}

				res = libusb_claim_interface(hDev, if_num);
				if(res < 0){
					fprintf(stderr, "Error claiming interface: %s\n", libusb_error_name(res));
					break;
				}
			}

			do {
				res = libusb_control_transfer(hDev, 0x21, 0x22, ACM_CTRL_DTR | ACM_CTRL_RTS, 0, NULL, 0, 0);
				if(res < 0){
					fprintf(stderr, "Error during control transfer: %s\n", libusb_error_name(res));
					break;
				}

/*
				uint8_t rbuf[0xC];
				int data_len = 0;

				memset(rbuf, 0, sizeof(rbuf));

				res = libusb_bulk_transfer(hDev, LIBUSB_ENDPOINT_IN | 1, rbuf, sizeof(rbuf), &data_len, 5000);
				if(res < 0){
					printf("Failed SceUsbSerial handshake.\n");
					break;
				}

				if(data_len != 10 || memcmp(rbuf, (const uint8_t[]){0xA1, 0x20, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00}, 10) != 0){
					printf("Failed SceUsbSerial handshake.\n");
					printf("Invalid start magic.\n");
					break;
				}
*/

				printf("Connected device.\n");

				return hDev;
			} while(0);

			libusb_release_interface(hDev, 1);
			libusb_release_interface(hDev, 0);
		} while(0);

		libusb_close(hDev);
	}

	return NULL;
}

int psp2wpp_usb_send(libusb_device_handle *hDev, const void *data, size_t length, int timeout){

	int res, send_len;

	send_len = 0;

	res = libusb_bulk_transfer(hDev, LIBUSB_ENDPOINT_OUT | 2, (unsigned char *)data, length, &send_len, 5000);
	if(res < 0){
		return res;
	}

	return send_len;
}

int psp2wpp_usb_recv(libusb_device_handle *hDev, void *data, size_t length, int timeout){

	int res, recv_len;

	recv_len = 0;

	res = libusb_bulk_transfer(hDev, LIBUSB_ENDPOINT_IN | 1 | 0x83, data, length, &recv_len, 5000);
	if(res < 0){
		return res;
	}

	return recv_len;
}


static libusb_device_handle *g_hDevice = NULL;
static psp2wpp_comm_packet g_comm_packet;

int psp2wpp_is_connected(void){
	return g_hDevice != NULL;
}

int psp2wpp_open_ex(void){

	int res;

	if(g_hDevice != NULL){
		return 0;
	}

	g_hDevice = psp2wpp_open(0x54C, 0x69B);
	if(g_hDevice == NULL){
		return -1;
	}

	res = psp2wpp_usb_recv(g_hDevice, &g_comm_packet, sizeof(g_comm_packet), 500);
	if(res == LIBUSB_ERROR_TIMEOUT){
		g_comm_packet.sequence = 0; // If get success here, unlucky.
		psp2wpp_usb_send(g_hDevice, &g_comm_packet, sizeof(g_comm_packet), 5000);
		psp2wpp_usb_recv(g_hDevice, &g_comm_packet, sizeof(g_comm_packet), 5000);
	}

	if(res < 0){
		printf("psp2wpp_usb_recv 0x%X\n", res);
		return res;
	}

	printf("sequence 0x%X\n", g_comm_packet.sequence);
	printf("cmd      0x%X\n", g_comm_packet.cmd);
	printf("result   0x%X\n", g_comm_packet.result);
	printf("size     0x%X\n", g_comm_packet.size);

	return 0;
}

int psp2wpp_open_ex_2(void){

	int res;

	if(g_hDevice != NULL){
		return 0;
	}

	g_hDevice = psp2wpp_open_core(0x54C, 0x69B);
	if(g_hDevice == NULL){
		return -1;
	}

	return 0;
}

int psp2wpp_close_ex(void){

	if(g_hDevice != NULL){
		libusb_release_interface(g_hDevice, 1);
		libusb_release_interface(g_hDevice, 0);
		libusb_close(g_hDevice);
		g_hDevice = NULL;
	}

	return 0;
}

int psp2wpp_usb_send_ex(const void *data, size_t length, int timeout){

	int res;

	res = psp2wpp_usb_send(g_hDevice, data, length, timeout);
	if(res == -1){
		psp2wpp_close_ex();
	}

	return res;
}

int psp2wpp_usb_recv_ex(void *data, size_t length, int timeout){

	int res;

	res = psp2wpp_usb_recv(g_hDevice, data, length, timeout);
	if(res == -1){
		psp2wpp_close_ex();
	}

	return res;
}

int psp2wpp_send_cmd(uint32_t cmd){

	int res;

	do {
		g_comm_packet.cmd = cmd;

		res = psp2wpp_usb_send_ex(&g_comm_packet, sizeof(g_comm_packet), 5000);
		if(res < 0){
			printf("psp2wpp_usb_send_ex 0x%X\n", res);
			return res;
		}

		res = psp2wpp_usb_recv_ex(&g_comm_packet, sizeof(g_comm_packet), 5000);
		if(res < 0){
			printf("psp2wpp_usb_recv_ex 0x%X\n", res);
			return res;
		}

		// printf("sequence 0x%X\n", g_comm_packet.sequence);
		// printf("cmd      0x%X\n", g_comm_packet.cmd);
		// printf("result   0x%X\n", g_comm_packet.result);
		// printf("size     0x%X\n", g_comm_packet.size);
	} while(g_comm_packet.result == 1);

	return 0;
}
