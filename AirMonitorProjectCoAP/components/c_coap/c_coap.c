 #include <stdio.h>
#include <string.h>
#include <netdb.h>
#include "esp_log.h"
#include "c_coap.h"

#include <coap3/coap.h>

static const char *TAG = "C_COAP";
ESP_EVENT_DEFINE_BASE(C_COAP_EVENT_BASE);

static coap_uri_t uri;
static coap_context_t *ctx = NULL;
static coap_session_t *session = NULL;

static char *DEVICE_TOKEN = NULL;

static unsigned char tokenProvision[8]  = {0};

//extern uint8_t ca_pem_start[] asm("_binary_coap_pem_start");
//extern uint8_t ca_pem_end[]   asm("_binary_coap_pem_end");

//static coap_session_t * coap_start_pki_session(coap_context_t *ctx, coap_address_t *dst_addr, coap_uri_t *uri);
static void coap_io_process_callback();
static coap_address_t *coap_get_address(coap_uri_t *uri);
static void coap_log_handler (coap_log_t level, const char *message);
static coap_response_t message_handler(coap_session_t *session, const coap_pdu_t *sent, const coap_pdu_t *received, const coap_mid_t mid);

void coap_start_client(){
    char       *server_uri = NULL;

    /* if (!coap_dtls_is_supported()) {
        ESP_LOGE(TAG, "Coap DTLS not supported");
    } */

    server_uri = malloc(256);
    sprintf(server_uri, "coap://%s", CONFIG_COAP_HOST_NAME);

    
    /* Set up the CoAP logging */
    coap_set_log_handler(coap_log_handler);
    coap_set_log_level(COAP_LOG_DEBUG);

    /* Set up the CoAP context */
    ctx = coap_new_context(NULL);
    if (!ctx) {
        ESP_LOGE(TAG, "coap_new_context() failed");
        //goto clean_up;
    }

    coap_register_response_handler(ctx, message_handler);
    xTaskCreate(coap_io_process_callback, "coap_io_process task", 6144, NULL, 6, NULL);

    if (coap_split_uri((const uint8_t *)server_uri, strlen(server_uri), &uri) == -1) {
        ESP_LOGE(TAG, "CoAP server uri error");
    }
    
    coap_address_t *dst_addr = coap_get_address(&uri);

    session = coap_new_client_session(ctx, NULL, dst_addr,
                                          uri.scheme == COAP_URI_SCHEME_COAP_TCP ? COAP_PROTO_TCP :
                                          COAP_PROTO_UDP);//coap_start_pki_session(ctx, dst_addr, &uri);

    if (!session) {
        ESP_LOGE(TAG, "coap_new_client_session() failed");
    }
}

static coap_optlist_t *generate_optlist(char *device_token) {
    coap_optlist_t *aux_optlist = NULL;

    u_char buf[4];
    coap_insert_optlist(&aux_optlist, coap_new_optlist(COAP_OPTION_CONTENT_FORMAT, coap_encode_var_safe(buf, sizeof (buf), COAP_MEDIATYPE_APPLICATION_JSON), buf));

    char uri[100];
    strcpy(uri, "api/v1/");
    strcat(uri, device_token);
    strcat(uri, "/telemetry");
    ESP_LOGI(TAG, "COAP telemetry URI: %s", uri);
    coap_insert_optlist(&aux_optlist, coap_new_optlist(COAP_OPTION_URI_PATH, strlen(uri), (const uint8_t *)uri));
    
    return aux_optlist;
}

void coap_stop_client(){
    if (session) {
        coap_session_release(session);
    }
    if (ctx) {
        coap_free_context(ctx);
    }
    coap_cleanup();
}

void save_device_token(char *device_token){
    DEVICE_TOKEN  = strdup(device_token);
}

//Publish some data
int coap_send_data(char *data) {
    ESP_LOGI(TAG, "Voy a mandar %s", data);
    if(DEVICE_TOKEN == NULL){
        ESP_LOGE(TAG, "El token todavia es null");
        return 1;
    }

    size_t tokenlength;
    unsigned char token[8];
    coap_pdu_t *request = NULL;

    request = coap_new_pdu(COAP_MESSAGE_NON, COAP_REQUEST_CODE_POST, session);
    if (!request) {
        ESP_LOGE(TAG, "coap_new_pdu() failed");
        //goto clean_up;
    }
    /* Add in an unique token */
    coap_session_new_token(session, &tokenlength, token);
    coap_add_token(request, tokenlength, token);

    coap_optlist_t* optlistsend = generate_optlist(DEVICE_TOKEN);
    coap_add_optlist_pdu(request, &optlistsend);

    coap_add_data(request, strlen(data), (unsigned char*) data);

    coap_send(session, request);

    coap_delete_optlist(optlistsend);
    optlistsend = NULL;
    return 0;
    //return ESP_OK;
}

static coap_response_t message_handler(coap_session_t *session, const coap_pdu_t *sent, const coap_pdu_t *received, const coap_mid_t mid) {
    ESP_LOGI(TAG, "Me ha llegado algo");
    const unsigned char *data = NULL;
    size_t data_len;
    size_t offset;
    size_t total;
    coap_pdu_code_t rcvd_code = coap_pdu_get_code(received);

    if (COAP_RESPONSE_CLASS(rcvd_code) == 2) {
        if (coap_get_data_large(received, &data_len, &data, &offset, &total)) {
            if (data_len != total) {
                printf("---Unexpected partial data received offset %u, length %u\n", offset, data_len);
            }
            coap_bin_const_t token = coap_pdu_get_token(received);
            if (memcmp(token.s, tokenProvision, token.length) == 0) {
                ESP_LOGI(TAG, "El token era igual (%d bytes):\n%.*s\n", data_len, (int)data_len, data);

                // Post the event
                esp_event_post(C_COAP_EVENT_BASE, C_COAP_EVENT_RECEIVED_DATA, data, (int)data_len, 0);
            }
            else {
                ESP_LOGI(TAG, "He recibido algo que no es el token (%d bytes):\n%.*s\n", data_len, (int)data_len, data);
            }
        }
        return COAP_RESPONSE_OK;
    }
    else {
        ESP_LOGI(TAG, "Received COAP message desde else (%d bytes):\n%.*s\n", data_len, (int)data_len, data);

    }
    return COAP_RESPONSE_OK;
}

esp_err_t coap_client_provision_send(char * data) {
    size_t tokenlength;
    coap_pdu_t *request = NULL;

    request = coap_new_pdu(COAP_MESSAGE_CON, COAP_REQUEST_CODE_POST, session);
    if (!request) {
        ESP_LOGE(TAG, "Error en coap_new_pdu()");
        return ESP_FAIL;
    }
    
    /* Add in an unique token */
    coap_session_new_token(session, &tokenlength, tokenProvision);
    if (coap_add_token(request, tokenlength, tokenProvision) == 0) {
        ESP_LOGE(TAG, "Error en coap_add_token()");
        return ESP_FAIL;
    }

    char *provision_path = "api/v1/provision";
    if (coap_add_option(request, COAP_OPTION_URI_PATH, strlen(provision_path), (u_char*) provision_path) == 0) {
        ESP_LOGE(TAG, "Error en coap_add_option()");
        return ESP_FAIL;
    }

    if (coap_add_data(request, strlen(data), (unsigned char*) data) == 0) {
        ESP_LOGE(TAG, "Error en coap_add_data_large_request()");
        return ESP_FAIL;
    }

    if (coap_send(session, request) == COAP_INVALID_MID) {
        ESP_LOGE(TAG, "Error en coap_send()");
        return ESP_FAIL;
    }
    return ESP_OK;
}

static void coap_io_process_callback() {
    while (true) {
        coap_io_process(ctx, 2000);
    }
}

static void coap_log_handler (coap_log_t level, const char *message) {
    uint32_t esp_level = ESP_LOG_INFO;
    char *cp = strchr(message, '\n');

    if (cp)
        ESP_LOG_LEVEL(esp_level, TAG, "%.*s", (int)(cp-message), message);
    else
        ESP_LOG_LEVEL(esp_level, TAG, "%s", message);
}

static coap_address_t *coap_get_address(coap_uri_t *uri) {
  static coap_address_t dst_addr;
    char *phostname = NULL;
    struct addrinfo hints;
    struct addrinfo *addrres;
    int error;
    char tmpbuf[INET6_ADDRSTRLEN];

    phostname = (char *)calloc(1, uri->host.length + 1);
    if (phostname == NULL) {
        ESP_LOGE(TAG, "calloc failed");
        return NULL;
    }
    memcpy(phostname, uri->host.s, uri->host.length);

    memset ((char *)&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;

    error = getaddrinfo(phostname, NULL, &hints, &addrres);
    if (error != 0) {
        ESP_LOGE(TAG, "DNS lookup failed for destination address %s. error: %d", phostname, error);
        free(phostname);
        return NULL;
    }
    if (addrres == NULL) {
        ESP_LOGE(TAG, "DNS lookup %s did not return any addresses", phostname);
        free(phostname);
        return NULL;
    }
    free(phostname);
    coap_address_init(&dst_addr);
    switch (addrres->ai_family) {
    case AF_INET:
        memcpy(&dst_addr.addr.sin, addrres->ai_addr, sizeof(dst_addr.addr.sin));
        dst_addr.addr.sin.sin_port        = htons(uri->port);
        inet_ntop(AF_INET, &dst_addr.addr.sin.sin_addr, tmpbuf, sizeof(tmpbuf));
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", tmpbuf);
        break;
    case AF_INET6:
        memcpy(&dst_addr.addr.sin6, addrres->ai_addr, sizeof(dst_addr.addr.sin6));
        dst_addr.addr.sin6.sin6_port        = htons(uri->port);
        inet_ntop(AF_INET6, &dst_addr.addr.sin6.sin6_addr, tmpbuf, sizeof(tmpbuf));
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", tmpbuf);
        break;
    default:
        ESP_LOGE(TAG, "DNS lookup response failed");
        return NULL;
    }
    freeaddrinfo(addrres);

    return &dst_addr;
}

/*
static coap_session_t * coap_start_pki_session(coap_context_t *ctx, coap_address_t *dst_addr, coap_uri_t *uri)
{
    unsigned int ca_pem_bytes = ca_pem_end - ca_pem_start;

    //unsigned int client_crt_bytes = client_crt_end - client_crt_start;
    //unsigned int client_key_bytes = client_key_end - client_key_start;
    static coap_dtls_pki_t dtls_pki;

    memset (&dtls_pki, 0, sizeof(dtls_pki));
    dtls_pki.version = COAP_DTLS_PKI_SETUP_VERSION;

    dtls_pki.verify_peer_cert        = 1;
    //dtls_pki.check_common_ca         = 1;
    dtls_pki.allow_self_signed       = 1;
    dtls_pki.allow_expired_certs     = 1;
    dtls_pki.cert_chain_validation   = 1;
    dtls_pki.cert_chain_verify_depth = 3;
    //dtls_pki.check_cert_revocation   = 1;
    //dtls_pki.allow_no_crl            = 1;
    //dtls_pki.allow_expired_crl       = 1;
    //dtls_pki.allow_bad_md_hash       = 1;
    //dtls_pki.allow_short_rsa_length  = 1;
    //dtls_pki.validate_cn_call_back   = verify_cn_callback;
    //dtls_pki.cn_call_back_arg        = NULL;
    //dtls_pki.validate_sni_call_back  = NULL;
    //dtls_pki.sni_call_back_arg       = NULL;
    //memset(client_sni, 0, sizeof(client_sni));
    //memcpy(client_sni, uri->host.s, MIN(uri->host.length, sizeof(client_sni)));
    
    //dtls_pki.client_sni = client_sni;
    
    dtls_pki.pki_key.key_type = COAP_PKI_KEY_PEM_BUF;
    dtls_pki.pki_key.key.pem_buf.ca_cert = ca_pem_start;
    dtls_pki.pki_key.key.pem_buf.ca_cert_len = ca_pem_bytes;

    return coap_new_client_session_pki(ctx, NULL, dst_addr, COAP_PROTO_DTLS, &dtls_pki);
}*/