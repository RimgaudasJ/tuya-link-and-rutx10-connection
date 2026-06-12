#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>
#include <argp.h>
#include <signal.h>
#include <libubus.h>
#include <libubox/blobmsg_json.h>


#include "tuya.h"
#include "option_parse.h"
#include "ubus_handler.h"

#define REPORT_INTERVAL_SEC 5

static char doc[] = "Periodic system metrics reporter to Tuya IoT cloud.";
static char args_doc[] = "";
static struct argp argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };

static void handle_signal(int sig);
volatile sig_atomic_t stop = 0;

static struct uloop_timeout mqtt_timer;
static tuya_mqtt_context_t client;


static void mqtt_loop_cb(struct uloop_timeout *t);

int main(int argc, char **argv)
{
    openlog("tuye_daemon", LOG_PID | LOG_CONS, LOG_DAEMON);
    int rc = EXIT_SUCCESS;

     // signal handling
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if(sigaction(SIGTERM, &sa, NULL) == -1){
        syslog(LOG_ERR, "SIGTERM");
        goto cleanup_log;
    }
    if(sigaction(SIGINT, &sa, NULL) == -1){
        syslog(LOG_ERR, "SIGINT");
        goto cleanup_log;
    }
    if(sigaction(SIGTSTP, &sa, NULL) == -1){
        syslog(LOG_ERR, "SIGTSTP");
        goto cleanup_log;
    }

    // Avoid unexpected process termination when a socket write hits a closed peer.
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        syslog(LOG_ERR, "Failed to ignore SIGPIPE");
        goto cleanup_log;
    }

    // Connect to ubus
    struct ubus_context *ctx;
    if (ubus_init_connection(&ctx) != 0) {
        syslog(LOG_ERR, "Failed to initialize ubus");
        rc = EXIT_FAILURE;
        goto cleanup_log;
    }

    // Argument parsing and initialization
    struct arguments args;


    memset(&args, 0, sizeof(args));
    argp_parse(&argp, argc, argv, 0, NULL, &args);

    
    syslog(LOG_INFO, "Starting with device_id=%s product_id=%s",
           args.device_id, args.product_id);

    if (tuya_init(&client, args.device_id, args.device_secret) != 0) {
        syslog(LOG_ERR, "Failed to initialize Tuya MQTT"); 
        rc = EXIT_FAILURE;
        goto cleanup_ubus;
    }

    mqtt_timer.cb = mqtt_loop_cb;
    uloop_timeout_set(&mqtt_timer, 100);
    uloop_init();
    ubus_add_uloop(ctx);
    uloop_run();
    tuya_mqtt_loop(&client);
    uloop_done();
    //cleanups
    tuya_mqtt_disconnect(&client);
    tuya_mqtt_deinit(&client);
    
cleanup_ubus:
    ubus_free(ctx);
cleanup_log:
    closelog();
    return rc;
}

static void handle_signal(int sig) {
    syslog(LOG_INFO, "Received signal %d, exiting", sig);
    uloop_end();
}

static void mqtt_loop_cb(struct uloop_timeout *t)
{
    tuya_mqtt_loop(&client);

    // reschedule every 100 ms
    uloop_timeout_set(t, 100);
}
