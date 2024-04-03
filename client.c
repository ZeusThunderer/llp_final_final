#include <thrift/c_glib/transport/thrift_transport.h>
#include <thrift/c_glib/transport/thrift_socket.h>
#include <thrift/c_glib/transport/thrift_buffered_transport.h>
#include <thrift/c_glib/protocol/thrift_binary_protocol.h>

#include <glib-object.h>
#include <glib.h>
#include <stdio.h>
#include <stdbool.h>

#include "lab2/linq.h"
#include "lab2/data.h"
#include "serializer.h"
#include "gen-c_glib/nt_structs_types.h"
#include "gen-c_glib/send_statement_service.h"

#define BUF_SIZE 1024

void print_items(GPtrArray *rows);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Required args: <address> <port>\n");
        return -1;
    }

    char* hostname = argv[1];

    errno = 0;
    int port = strtol(argv[2], NULL, 10);

    if (errno != 0) {
        printf("Incorrect port.\n");
        return -1;
    }


    ThriftSocket *socket;
    ThriftTransport *transport;
    ThriftProtocol *protocol;

    SendStatement_serviceIf *service;

    gboolean success;
    gchar *info = NULL;
    ServerResponse *result = NULL;
    GError *error = NULL;

#if (!GLIB_CHECK_VERSION(2, 36, 0))
    g_type_init ();
#endif

    socket = g_object_new(THRIFT_TYPE_SOCKET,
                          "hostname", hostname,
                          "port", port,
                          NULL);

    transport = g_object_new(THRIFT_TYPE_BUFFERED_TRANSPORT,
                             "transport", socket,
                             NULL);
    protocol = g_object_new(THRIFT_TYPE_BINARY_PROTOCOL,
                            "transport", transport,
                            NULL);

    thrift_transport_open(transport, &error);

    if (error != NULL) {
        fprintf(stderr, "Error: %s\n", error->message);
        g_clear_error(&error);
        if (transport) g_object_unref(transport);
        if (socket) g_object_unref(socket);
        return -1;
    }
    service = g_object_new(TYPE_SEND_STATEMENT_SERVICE_CLIENT,
                          "input_protocol", protocol,
                          "output_protocol", protocol,
                          NULL);

    char buffer[BUF_SIZE];
    buffer[0] = '\0';

    while (true) {
        printf("stmt > ");
        fgets(buffer, BUF_SIZE, stdin);
        buffer[BUF_SIZE - 1] = '\0';
        if (strcmp(buffer, "quit\n") == 0)
            break;
        Statement* s = parse_statement(buffer);
        if (!s) continue;
        Statement_ *st = serialize(s);
        if (!st) continue;

        result = g_object_new(TYPE_SERVER_RESPONSE, NULL);
        success = send_statement_service_if_execute_stmt(service, &result, st, &error);
        if (success) {
            g_object_get(result, "info", &info, NULL);
            fprintf(stdout, "%s\n", info);
            if (result->__isset_rows) {
                print_items(result->rows);
            }
        } else {
            fprintf(stderr, "Error: %s\n", error->message);
            g_clear_error(&error);
        }
    }

    thrift_transport_close(transport, &error);
    success = success && (error == NULL);
    if (result)
        g_object_unref(result);
    g_clear_error(&error);
    if (service)
        g_object_unref(service);
    g_object_unref(protocol);
    g_object_unref(transport);
    g_object_unref(socket);

    return (success ? 0 : 1);

}

void printVal(Val_ * val) {

}
void print_items(GPtrArray *rows) {

}
