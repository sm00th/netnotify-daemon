#ifndef __NETNOTIFY_DAEMON_H__

#define __NETNOTIFY_DAEMON_H__

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <dbus/dbus-glib.h>

#define DAEMON_PORT 3000

#define SUMMARY_STR "SUM"
#define BODY_STR "BODY"
#define IMG_STR "IMG"
#define TIMEOUT_STR "TOUT"

#define NETNOTIFY_TYPE_DAEMON                  (netnotify_daemon_get_type ())
#define NETNOTIFY_DAEMON(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NETNOTIFY_TYPE_DAEMON, NetnotifyDaemon))
#define NETNOTIFY_IS_DAEMON(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NETNOTIFY_TYPE_DAEMON))
#define NETNOTIFY_DAEMON_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), NETNOTIFY_TYPE_DAEMON, NetnotifyDaemonClass))
#define NETNOTIFY_IS_DAEMON_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), NETNOTIFY_TYPE_DAEMON))
#define NETNOTIFY_DAEMON_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), NETNOTIFY_TYPE_DAEMON, NetnotifyDaemonClass))

typedef struct _NetnotifyDaemon        NetnotifyDaemon;
typedef struct _NetnotifyDaemonClass   NetnotifyDaemonClass;

struct _NetnotifyDaemon {
	GObject parent;
};

struct _NetnotifyDaemonClass {
	GObjectClass parent;
};

struct client {
	int id;
	GSocketConnection *conn;
};

GType netnotify_daemon_get_type( void );

gboolean netnotify_daemon_notify_handler( NetnotifyDaemon *daemon,
		const gchar *app_name,
		guint id,
		const gchar *icon,
		const gchar *summary,
		const gchar *body,
		gchar **actions,
		GHashTable *hints,
		int timeout, DBusGMethodInvocation *context );
gboolean netnotify_daemon_close_notification_handler();
gboolean netnotify_daemon_get_capabilities( NetnotifyDaemon *daemon,
        char ***caps );
gboolean netnotify_daemon_get_server_information( NetnotifyDaemon *daemon,
        char **out_name,
        char **out_vendor,
        char **out_version,
        char **out_spec_ver );

#endif
