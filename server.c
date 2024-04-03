#include <thrift/c_glib/thrift.h>
#include <thrift/c_glib/protocol/thrift_binary_protocol_factory.h>
#include <thrift/c_glib/protocol/thrift_protocol_factory.h>
#include <thrift/c_glib/server/thrift_server.h>
#include <thrift/c_glib/server/thrift_simple_server.h>
#include <thrift/c_glib/transport/thrift_buffered_transport_factory.h>
#include <thrift/c_glib/transport/thrift_server_socket.h>
#include <thrift/c_glib/transport/thrift_server_transport.h>

#include <stdio.h>
#include <stdbool.h>
#include <glib.h>
#include <glib-object.h>

#include "gen-c_glib/send_statement_service.h"
#include "lab1/database.h"
#include "lab1/table.h"
#include "request_processor.h"


// ================ START OF DECLARATIONS ================

G_BEGIN_DECLS

#define TYPE_SEND_STATEMENT_HANDLER \
  (send_statement_handler_get_type ())

#define SEND_STATEMENT_HANDLER(obj)                                \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                   \
                               TYPE_SEND_STATEMENT_HANDLER,        \
                               send_statement_handler_impl))

#define SEND_STATEMENT_HANDLER_CLASS(c)                    \
  (G_TYPE_CHECK_CLASS_CAST ((c),                                \
                            TYPE_SEND_STATEMENT_HANDLER,   \
                            send_statement_handler_implClass))

#define IS_SEND_STATEMENT_HANDLER(obj)                             \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                   \
                               TYPE_SEND_STATEMENT_HANDLER))

#define IS_SEND_STATEMENT_HANDLER_CLASS(c)                 \
  (G_TYPE_CHECK_CLASS_TYPE ((c),                                \
                            TYPE_SEND_STATEMENT_HANDLER))

#define SEND_STATEMENT_HANDLER_GET_CLASS(obj)              \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                            \
                              TYPE_SEND_STATEMENT_HANDLER, \
                              send_statement_handler_implClass))

struct _send_statement_handler_impl {
    SendStatement_serviceHandler parent_instance;
};
typedef struct _send_statement_handler_impl send_statement_handler_impl;

struct _send_statement_handler_implClass {
    SendStatement_serviceHandlerClass parent_class;
};

typedef struct _send_statement_handler_implClass send_statement_handler_implClass;

GType send_statement_handler_get_type(void);

G_END_DECLS


// ================ END OF DECLARATIONS ================


// ================ START OF IMPLEMENTATION ================


G_DEFINE_TYPE(send_statement_handler_impl,
              send_statement_handler,
              TYPE_SEND_STATEMENT_SERVICE_HANDLER
)

static database* db;

static gboolean send_statement_handler_execute (SendStatement_serviceIf *iface, ServerResponse **_return, const Statement_ * stmt, GError **error) {
    THRIFT_UNUSED_VAR(iface);
    THRIFT_UNUSED_VAR(error);

    ServerResponse* response = process_client_request(stmt, db);

    g_object_set(*_return, "status", response->status,"info", strdup(response->info), NULL);
    if (response->__isset_rows) {
        g_object_set(*_return,"rows", response->rows,NULL);
    }
    g_object_unref(response);
    return true;
}

static void
send_statement_handler_finalize (GObject *object)
{
    send_statement_handler_impl *self =
    SEND_STATEMENT_HANDLER (object);


    G_OBJECT_CLASS (send_statement_handler_parent_class)->finalize (object);
}

static void
send_statement_handler_init (send_statement_handler_impl *self)
{
    // no fields -> empty initializer
}

static void
send_statement_handler_class_init (send_statement_handler_implClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    SendStatement_serviceHandlerClass *dataExchangeServiceHandlerClass =
    SEND_STATEMENT_SERVICE_HANDLER_CLASS(klass);

    gobject_class->finalize = send_statement_handler_finalize;

    dataExchangeServiceHandlerClass->execute_stmt =
            send_statement_handler_execute;

}

// ================ END OF IMPLEMENTATION ================



int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Required argument: <port> <db_filename>");
        return -1;
    }
    errno = 0;
    int port = strtol(argv[1], NULL, 10);

    if (errno != 0) {
        printf("Incorrect port.\n");
        return -1;
    }
    db = createDatabase(argv[2], DEFAULT_PAGE_SIZE);
    send_statement_handler_impl *handler;
    SendStatement_serviceProcessor *processor;

    ThriftServerTransport *server_transport;
    ThriftTransportFactory *transport_factory;
    ThriftProtocolFactory *protocol_factory;
    ThriftServer *server;

    GError *error = NULL;

#if (!GLIB_CHECK_VERSION(2, 36, 0))
    g_type_init ();
#endif
    handler = g_object_new(TYPE_SEND_STATEMENT_HANDLER,
                           NULL);
    processor = g_object_new(TYPE_SEND_STATEMENT_SERVICE_PROCESSOR,
                             "handler", handler,
                             NULL);

    server_transport = g_object_new(THRIFT_TYPE_SERVER_SOCKET,
                                    "port", port,
                                    NULL);
    transport_factory = g_object_new(THRIFT_TYPE_BUFFERED_TRANSPORT_FACTORY,
                                     NULL);
    protocol_factory = g_object_new(THRIFT_TYPE_BINARY_PROTOCOL_FACTORY,
                                    NULL);

    server = g_object_new(THRIFT_TYPE_SIMPLE_SERVER,
                          "processor", processor,
                          "server_transport", server_transport,
                          "input_transport_factory", transport_factory,
                          "output_transport_factory", transport_factory,
                          "input_protocol_factory", protocol_factory,
                          "output_protocol_factory", protocol_factory,
                          NULL);

    printf("Listening on port: %d\n", port);
    thrift_server_serve(server, &error);
    if (error != NULL) {
        fprintf(stderr, "Error: %s\n", error->message);
        g_clear_error(&error);
        return -1;
    }

    g_object_unref(server);
    g_object_unref(protocol_factory);
    g_object_unref(transport_factory);
    g_object_unref(server_transport);

    g_object_unref(processor);
    g_object_unref(handler);
    closeDatabase(db);
    return 0;
}