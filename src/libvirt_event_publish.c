#include <stdio.h>
#include <stdlib.h>
#include <libvirt/libvirt.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "mosquitto.h"

#define KEEP_ALIVE 60
#define MSG_MAX_SIZE  512

char buff[MSG_MAX_SIZE];

static int  myEventCallback(virConnectPtr conn,
		      virDomainPtr dom,
		      int event,
		      int detail,
		      void *opaque) 
{
    struct mosquitto *mosq = (struct mosquitto *)opaque;

    const char *name = virDomainGetName(dom);

    struct tm *currTime;
    time_t now;
    time(&now);
    currTime = localtime(&now);

    snprintf(buff, MSG_MAX_SIZE, "{time: %d/%d/%d %d:%d:%d, domain: %s, event: %d}\n", currTime->tm_year + 1900, currTime->tm_mon + 1, currTime->tm_mday, currTime->tm_hour, currTime->tm_min, currTime->tm_sec, name, event);
   
    mosquitto_publish(mosq, NULL, "topic1", strlen(buff) + 1, buff, 0, 0);

    return 0;
}

static void* eventThreadLoop() {
    while(1) {
        if(virEventRunDefaultImpl() < 0) {
            printf("Run errer.\n");
        }
    }
    abort();
}

int main(int argc, char *argv[]) {

    if(argc != 3) {
        fprintf(stderr, "Please input:%s <hostname> <port>.\n", argc);
        return -1;
    }

    const char *host = argv[1];
    int port = atoi(argv[2]);

    virConnectPtr conn = NULL;

    int eventid = VIR_DOMAIN_EVENT_ID_LIFECYCLE;

    virEventRegisterDefaultImpl();

    conn = virConnectOpen(NULL); 
    if(conn == NULL) {
        fprintf(stderr, "connect hypervisor is failed.\n");
        return -1;
    }

    pthread_t eventThread;

    pthread_create(&eventThread, NULL, eventThreadLoop, NULL);

    int ret;
    struct mosquitto *mosq;

    ret = mosquitto_lib_init();
    if(ret) {
        fprintf(stderr, "mosquitto lib init is failed.\n");
        virConnectClose(conn);
        return -1;
    }

    mosq = mosquitto_new("Pub", true, NULL);
    if(mosq == NULL) {
        fprintf(stderr, "Create mosquitto instance is failed.\n");
        mosquitto_lib_cleanup();
        virConnectClose(conn);
        return -1;
    }

    const char *cafile = "/root/cLan/certs/ca/ca.crt";
    const char *capath = "/root/cLan/certs/ca";
    const char *certfile = "/root/cLan/certs/client/client.crt";
    const char *keyfile = "/root/cLan/certs/client/client.key";
    ret = mosquitto_tls_set(mosq, cafile, capath, certfile, keyfile, NULL);
    if(ret) {
        fprintf(stderr, "TLS certification  is failed.\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        virConnectClose(conn);
        return -1;
    }

    ret = mosquitto_connect(mosq, host, port, KEEP_ALIVE);
    if(ret) {
        fprintf(stderr, "Connect broker is failed.\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        virConnectClose(conn);
        return -1;
    }

    ret = mosquitto_loop_start(mosq);
    if(ret) {
        fprintf(stderr, "mosquitto loop is failed.\n");
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        virConnectClose(conn);
        return -1;
    }

    int id = virConnectDomainEventRegisterAny(conn, 
    				 	      NULL,
					      eventid,
					      VIR_DOMAIN_EVENT_CALLBACK(myEventCallback),
					      mosq,
					      NULL);

    while(1) pause();

    mosquitto_disconnect(mosq);

    mosquitto_destroy(mosq);

    mosquitto_lib_cleanup();

    virConnectDomainEventDeregisterAny(conn, id);

    virConnectClose(conn);

    return 0;
}

