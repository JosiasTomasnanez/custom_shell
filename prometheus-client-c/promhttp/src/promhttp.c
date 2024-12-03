#include <string.h>

#include <microhttpd.h>
#include "prom.h"

prom_collector_registry_t *PROM_ACTIVE_REGISTRY;

void promhttp_set_active_collector_registry(prom_collector_registry_t *active_registry) {
  if (!active_registry) {
    PROM_ACTIVE_REGISTRY = PROM_COLLECTOR_REGISTRY_DEFAULT;
  } else {
    PROM_ACTIVE_REGISTRY = active_registry;
  }
}

enum MHD_Result promhttp_handler(void *cls, struct MHD_Connection *connection, const char *url, const char *method,
                     const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) { (void) cls; (void) version; (void) upload_data; (void) upload_data_size; (void) con_cls;
  if (strcmp(method, "GET") != 0) {
    char *buf = "Invalid HTTP Method\n";
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(buf), (void *)buf, MHD_RESPMEM_PERSISTENT);
    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
    MHD_destroy_response(response);
    return ret;
  }
  if (strcmp(url, "/") == 0) {
    char *buf = "OK\n";
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(buf), (void *)buf, MHD_RESPMEM_PERSISTENT);
    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
  }
  if (strcmp(url, "/metrics") == 0) {
    const char *buf = prom_collector_registry_bridge(PROM_ACTIVE_REGISTRY);
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(buf), (void *)buf, MHD_RESPMEM_MUST_FREE);
    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
  }
  char *buf = "Bad Request\n";
  struct MHD_Response *response = MHD_create_response_from_buffer(strlen(buf), (void *)buf, MHD_RESPMEM_PERSISTENT);
  enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
  MHD_destroy_response(response);
  return ret;
}

struct MHD_Daemon *promhttp_start_daemon(unsigned int flags, unsigned short port, MHD_AcceptPolicyCallback apc,
                                         void *apc_cls) {
  return MHD_start_daemon(flags, port, apc, apc_cls, &promhttp_handler, NULL, MHD_OPTION_END);
}

