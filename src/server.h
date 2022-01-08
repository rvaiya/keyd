#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

#define MSG_PING 1
#define MSG_MAPPING 2
#define MSG_RELOAD 3
#define MSG_RESET 4

#define MAX_RESPONSE_SIZE 4096
#define MAX_REQUEST_SIZE 1024

int create_server_socket();
void server_process_connections(int sd);

int client_send_message(uint8_t type, const char *payload);
#endif
