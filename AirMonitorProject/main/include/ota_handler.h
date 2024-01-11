#ifndef __OTA_HANDLER
#define __OTA_HANDLER

void ota_check();
void ota_incoming_update_handler(char * payload);


void ota_update_chunk_received(char *data, int data_len);
void ota_send_status_update(char * payload);

#endif