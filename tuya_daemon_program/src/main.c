#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>
#include <argp.h>
#include <signal.h>


#include "daemon.h"
#include "hardware.h"
#include "network.h"
#include "cpu.h"
#include "tuya.h"
#include "json_builder.h"
#include "option_parse.h"

#define REPORT_INTERVAL_SEC 5

static char doc[] = "Periodic system metrics reporter to Tuya IoT cloud.";
static char args_doc[] = "";
static struct argp argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };

static void handle_signal(int sig);
volatile sig_atomic_t stop = 0;

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

    // Argument parsing and initialization
    struct arguments args;
    tuya_mqtt_context_t client;
    char json_buf[JSON_BUF];

    memset(&args, 0, sizeof(args));
    argp_parse(&argp, argc, argv, 0, NULL, &args);

    
    syslog(LOG_INFO, "Starting with device_id=%s product_id=%s",
           args.device_id, args.product_id);

    if (args.daemon_flag) {
        if (create_daemon() != 0) {
            syslog(LOG_ERR, "Failed to create daemon process");
            rc = EXIT_FAILURE;
            goto cleanup_log;
        }
        syslog(LOG_INFO, "Running as daemon");
    }

    if (tuya_init(&client, args.device_id, args.device_secret) != 0) {
        syslog(LOG_ERR, "Failed to initialize Tuya MQTT");
        rc = EXIT_FAILURE;
        goto cleanup_tuya;
    }

    syslog(LOG_INFO, "Tuya MQTT initialized, entering report loop");

    for (; stop!=1;) {
        int elapsed;
        /* Drive MQTT for REPORT_INTERVAL_SEC seconds */
        for (elapsed = 0; elapsed < REPORT_INTERVAL_SEC; elapsed++) {
            tuya_mqtt_loop(&client);
            sleep(1);
        }

        if (build_json(json_buf, sizeof(json_buf)) == 0) {
            syslog(LOG_INFO, "Reporting metrics payload");
            int report_rc = tuya_report(&client, json_buf);
            if (report_rc != 0)
                syslog(LOG_ERR, "tuya_report returned %d (may still succeed)", report_rc);
            else
                syslog(LOG_INFO, "Metrics reported successfully");
        } else {
            syslog(LOG_ERR, "Failed to build metrics JSON");
        }
    }

    //cleanups
cleanup_tuya:
    tuya_mqtt_disconnect(&client);
    tuya_mqtt_deinit(&client);
cleanup_log:
    closelog();
    return rc;
}

static void handle_signal(int sig) {
    syslog(LOG_INFO, "Received signal %d, exiting", sig);
    stop = 1;
}
