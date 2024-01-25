#ifndef __OTA_PARSER
#define __OTA_PARSER

static const uint32_t UPDATE_CHUNK_SIZE = 2048;

char *ota_build_update_request_topic(int req_id, int req_chunk);
char *ota_build_update_request_url(char *payload);
char *ota_get_update_url(char *payload);

void ota_download_chunk(int chunk);
int ota_get_update_size(char *payload);
void ota_update_status(char *status);
#endif