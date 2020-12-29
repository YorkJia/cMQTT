//#include "dev_sign_api.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "cMQTT.h"
#include <pthread.h>



void HAL_Printf(const char *fmt, ...);


#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)


static void user_connect_success_callback(void)
{
    EXAMPLE_TRACE("DEBUG callback connect to broker success.\n");
}

static void user_disconnected_callback(void)
{
    EXAMPLE_TRACE("DEBUG callback disconnected.\n");
}



int example_publish(mqtt_client_t *pclient, int cnt)
{
    char payload[100];
    if(pclient == NULL){
        return FAIL_RETURN;
    }

    memset(payload, 0, sizeof(payload));
    sprintf(payload, "{\"count\": %d}", cnt);

    mqtt_publish_simple(pclient, 
                    "edge/change/B827EBFFFE9BAB1F/89860434031980047715", 
                    MQTT_QOS0, payload, strlen(payload));
}


void *mqtt_client_thread(void *arg)
{
    int *id = (int *)arg;
    char client_id[100];
    mqtt_client_t *pclient = NULL;
    int res, loop_cnt = 0;

    pthread_detach(pthread_self());

    memset(client_id, 0, sizeof(client_id));
    sprintf(client_id, "client/%d", *id);

    pclient = mqtt_client_new("127.0.0.1", 1883, NULL,
                            client_id,
                            "admin", 
                            "password");

    if(pclient == NULL){
        printf("mqtt client[%d] new fail.\n", *id);
        return ((void *)0);
    }

     do{
        HAL_Printf("try to connect broker...\n");
        res = mqtt_client_connect(pclient);
        sleep(1);
    }while(res != SUCCESS_RETURN);

    HAL_Printf("client[%d] connect broker success.\n", *id);

    while(1){
        if(loop_cnt++ > 600){
            loop_cnt = 0;
            example_publish(pclient, *id);
        }

        mqtt_client_yield(pclient, 100);
    }
}


static pthread_t tid[100];
static int thread_id[100];

int main(void)
{
    int i;
    for(i = 0; i < 100; i++){
        thread_id[i] = i + 1;

        pthread_create(&tid[i], NULL, mqtt_client_thread, (void *)&thread_id[i]);
        sleep(1);
    }

    while(1){

        sleep(10);
    }
}

