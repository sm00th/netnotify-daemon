#include "config.h"
#include "netnotify-daemon.h"
#include "netnotify-daemon-glue.h"

static const char HELLO[] = "OK NETNOTIFY "VERSION"\n";

G_DEFINE_TYPE( NetnotifyDaemon, netnotify_daemon, G_TYPE_OBJECT );

static GHashTable *clients;

static void client_destroy( gpointer data )
{
    struct client *client = (struct client *)data;
    g_object_unref( client->conn );
    g_free( client );
}

static gboolean client_send( struct client *client, const gchar *msg, gsize msgsize )
{
    GOutputStream *out;
    GError *error;

    out = g_io_stream_get_output_stream( G_IO_STREAM( client->conn ) );
    error = NULL;
    if( g_output_stream_write( out, msg, msgsize, NULL, &error ) < 1 ) {
        if( error != NULL ) {
            g_printerr( "Failed to send data: %s\n", error->message );
            g_error_free( error );
        }
        return TRUE;
    }
    return FALSE;
}

static gboolean event_incoming( GSocketService *srv, GSocketConnection *conn, GObject *source, gpointer userdata )
{
    struct client *client;
    gint new_id = 0;

    g_object_ref( conn );
    client       = g_new(struct client, 1);
    while( g_hash_table_lookup( clients, GINT_TO_POINTER( new_id ) ) != NULL ) {
        new_id++;
    }
    client->id   = new_id;
    client->conn = conn;

    client_send( client, (gchar*)HELLO, sizeof( HELLO ) - 1 );
    g_hash_table_insert(clients, GINT_TO_POINTER(client->id), client);

    return FALSE;
}

static void netnotify_daemon_class_init( NetnotifyDaemonClass *daemon_class )
{
}

static void netnotify_daemon_init( NetnotifyDaemon *daemon )
{
}

static gboolean notify_client( gpointer key, gpointer value, gpointer user_data )
{
    struct client *client;
    GString *message;

    client = (struct client*) value;
    message = (GString*)user_data;

    return client_send( client, message->str, message->len );
}

gboolean netnotify_daemon_notify_handler( NetnotifyDaemon *daemon,
        const gchar *app_name,
        guint id,
        const gchar *icon,
        const gchar *summary,
        const gchar *body,
        gchar **actions,
        GHashTable *hints,
        int timeout, DBusGMethodInvocation *context )
{
    GString *message = g_string_new( "" );

    g_string_printf( message, "%s\n%d\nEND%s\n", TIMEOUT_STR, timeout, TIMEOUT_STR );
    if( icon != NULL && g_strcmp0( icon, "" ) ) {
        g_string_append( message, g_strconcat( IMG_STR, "\n", icon, "\nEND", IMG_STR, "\n", NULL ) );
    }
    if( summary != NULL && g_strcmp0( summary, "" ) ) {
        g_string_append( message, g_strconcat( SUMMARY_STR, "\n", summary, "\nEND", SUMMARY_STR, "\n", NULL ) );
    }
    if( body != NULL && g_strcmp0( body, "" ) ) {
        g_string_append( message, g_strconcat( BODY_STR, "\n", body, "\nEND", BODY_STR, "\n", NULL ) );
    }
    g_hash_table_foreach_remove( clients, notify_client, message );
    g_string_free( message, TRUE );

    dbus_g_method_return( context, id );
    return TRUE;
}

gboolean netnotify_daemon_close_notification_handler()
{
    return TRUE;
}

gboolean netnotify_daemon_get_capabilities( NetnotifyDaemon *daemon,
        char ***caps )
{
    return TRUE;
}

gboolean netnotify_daemon_get_server_information( NetnotifyDaemon *daemon,
        char **out_name,
        char **out_vendor,
        char **out_version,
        char **out_spec_ver )
{
    *out_name = g_strdup( "Netnotification Daemon" );
    *out_version = g_strdup( PACKAGE_VERSION );
    return TRUE;
}

static void request_dbus_name( DBusGConnection *connection )
{
    DBusGProxy *bus_proxy;
    GError *error = NULL;

    bus_proxy = dbus_g_proxy_new_for_name ( connection,
            DBUS_SERVICE_DBUS,
            DBUS_PATH_DBUS,
            DBUS_INTERFACE_DBUS );

    error = NULL;
    if( !dbus_g_proxy_call( bus_proxy, "RequestName", &error,
                G_TYPE_STRING, "org.freedesktop.Notifications",
                G_TYPE_UINT, 0,
                G_TYPE_INVALID,
                G_TYPE_UINT, NULL,
                G_TYPE_INVALID ) )
    {
        g_printerr( "Could not acquire name: %s\n", error->message );
        g_error_free( error );
        exit( 1 );
    }
    g_object_unref( bus_proxy );
}

static void server_bind( GSocketService *server )
{
    struct sockaddr_in saddr;
    GSocketAddress *gsaddr;
    GError *error = NULL;

    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_family      = AF_INET;
    saddr.sin_port        = htons( DAEMON_PORT );
    gsaddr = g_socket_address_new_from_native( &saddr, sizeof( saddr ) );

    if( !g_socket_listener_add_address( G_SOCKET_LISTENER( server ), gsaddr, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, NULL, NULL, &error ) ) {
        g_printerr( "Failed to add listener: %s\n", error->message );
        g_error_free( error );
        g_object_unref( gsaddr );
        exit( 1 );
    }

    g_object_unref( gsaddr );
}

int main( int argc, char **argv )
{
    NetnotifyDaemon *daemon;
    DBusGConnection *connection;
    GError *error;
    GMainLoop *loop;
    GSocketService *server;

    g_type_init();

    error = NULL;
    connection = dbus_g_bus_get( DBUS_BUS_SESSION, &error );
    if ( connection == NULL ) {
        g_printerr ( "Failed to open connection to bus: %s\n",
            error->message );
        g_error_free (error);
        exit( 1 );
    }

    dbus_g_object_type_install_info( NETNOTIFY_TYPE_DAEMON, &dbus_glib_netnotify_daemon_object_info );
    request_dbus_name( connection );

    daemon = g_object_new( NETNOTIFY_TYPE_DAEMON, NULL );
    dbus_g_connection_register_g_object( connection,
        "/org/freedesktop/Notifications",
        G_OBJECT( daemon ) );

    clients = g_hash_table_new_full( g_direct_hash, g_direct_equal, NULL, client_destroy );

    server = g_socket_service_new();
    server_bind( server );

    g_signal_connect( server, "incoming", G_CALLBACK( event_incoming ), NULL );
    g_socket_service_start( server );

    loop = g_main_loop_new( NULL, FALSE );
    g_main_loop_run( loop );

    g_object_unref( daemon );
    dbus_g_connection_unref( connection );
    g_object_unref( server );

    return 0;
}
