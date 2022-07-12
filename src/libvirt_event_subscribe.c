#include <stdio.h>
#include <stdlib.h>
#include "mosquitto.h"

#define KEEPALIVE 60

int running = 1;

void my_connect_callback(struct mosquitto *mosq, void *obj, int rec) {
    printf("Call the function: my_connect_callback.\n");
    if(rec){
        printf("On_connect error.\n");
        exit(1);
    } else {
	if(mosquitto_subscribe(mosq, NULL, "topic1", 0)) {
            printf("Set topic error.\n");
	    exit(1);
        }
    }
}

void my_disconnect_callback(struct mosquitto *mosq, void *obj, int rec) {
    printf("Call the function: my_disconnect_callback.\n");
    running = 0;
}

void my_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int*granted_qos) {
    printf("Call the function: my_subscribe_callback.\n");
}

void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    printf("Call the function: my_message_callback.\n");
    
    printf("Receive a message %s: %s", (char*)msg->topic, (char*)msg->payload);

    if(0 == strcmp((char*)msg->payload, "quit")) {
        mosquitto_disconnect(mosq);
    }
}

int main(int argc, char *argv[]) {

    if(argc != 3) {
        fprintf(stderr, "Please input:%s <hostname> <port>.\n", argv[0]);
        return -1;
    }

    const char *host = argv[1];
    int port = atoi(argv[2]);

    int ret = 0;
    struct mosquitto *mosq;

    ret = mosquitto_lib_init();
    if(ret) {
        printf("init lib is failed.\n");
        return -1;
    }

    mosq = mosquitto_new("sub", true, NULL);
    if(mosq == NULL) {
        printf("create new mosquitto instance failed.\n");
        mosquitto_lib_cleanup();
        return -1;
    }

    mosquitto_connect_callback_set(mosq, my_connect_callback);
    mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);
    mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
    mosquitto_message_callback_set(mosq, my_message_callback);

    const char *cafile = "/root/cLan/mosquitto/certs/ca/ca.crt";
    const char *capath = "/root/cLan/mosquitto/certs/ca";
    const char *certfile = "/root/cLan/mosquitto/certs/client/client.crt";
    const char *keyfile = "/root/cLan/mosquitto/certs/client/client.key";
    ret = mosquitto_tls_set(mosq, cafile, capath, certfile, keyfile, NULL);
    if(ret) {
        printf("TLS certification is failed.\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return -1;       
    }

    ret = mosquitto_connect(mosq, host, port, KEEPALIVE);
    if(ret) {
        printf("connect to broker is failed.\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return -1;       
    }

    printf("Subscribe begin.\n");
    int loop = mosquitto_loop_start(mosq);
    if(loop) {
        printf("mosquitto loop error.\n");
        goto cleanup;
        return -1;
    }

    const char cmd[10];
    while(running) {
        scanf("%s", cmd);
        if(0 == strcmp(cmd, "quit")) {
            running = 0;
        }
    }

cleanup:
    mosquitto_disconnect(mosq);    
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    printf("Subscribe end.\n");
    return 0;   
}
