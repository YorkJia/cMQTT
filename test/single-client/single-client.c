//#include "dev_sign_api.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "cMQTT.h"
#include <pthread.h>



void HAL_Printf(const char *fmt, ...);


#define SN          "B827EBFFFE9BAB1F"
#define ICCID       "89860434031980047715"

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)



void example_message_arrive(void *pcontext, void *pclient, mqtt_event_msg_t *msg)
{
    mqtt_topic_info_t     *topic_info = (mqtt_topic_info_t *)msg->msg;
    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            /* print topic name and topic message */
            EXAMPLE_TRACE("Message Arrived:");
            EXAMPLE_TRACE("Topic  : %.*s", topic_info->topic_len, topic_info->ptopic);
            EXAMPLE_TRACE("Payload: %.*s\n", topic_info->payload_len, topic_info->payload);
            break;
        default:
            break;
    }
}



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



int main(int argc, char *argv[])
{
    int res, loop_cnt = 0, cnt = 0;
    mqtt_client_t *pclient = NULL;
    pclient = mqtt_client_new("127.0.0.1", 1883, NULL,
                            "client/01",
                            "admin", 
                            "password");
  
    mqtt_set_connect_success_callback(pclient, user_connect_success_callback);
    mqtt_set_disconnected_callback(pclient, user_disconnected_callback);

    
    mqtt_set_will_opt(pclient, MQTT_QOS1, 0,
                    "edge/offline/B827EBFFFE9BAB1F/89860434031980047715",
                    "device offline");


    do{
        HAL_Printf("try to connect broker...\n");
        res = mqtt_client_connect(pclient);
        sleep(1);
    }while(res != SUCCESS_RETURN);
    

    res = mqtt_subscribe(pclient, "iotlink/server/ntp", MQTT_QOS1, example_message_arrive, NULL);
    if(res < 0){
        HAL_Printf("subscribe[%s] fail.\n", "iotlink/server/ntp");
    }

    res = mqtt_subscribe(pclient, "iotlink/server/ntp1", MQTT_QOS1, example_message_arrive, NULL);
    if(res < 0){
        HAL_Printf("subscribe[%s] fail.\n", "iotlink/server/ntp");
    }

    res = mqtt_subscribe(pclient, "iotlink/server/ntp2", MQTT_QOS1, example_message_arrive, NULL);
    if(res < 0){
        HAL_Printf("subscribe[%s] fail.\n", "iotlink/server/ntp");
    }
    
    example_publish(pclient, cnt++);

    iotx_mc_topic_handle_t *node = NULL;
    while (1) {
        if(loop_cnt++ > 100){
            loop_cnt = 0;
            example_publish(pclient, cnt++);

            /*
            list_for_each_entry(node, &pclient->list_sub_handle, linked_list, iotx_mc_topic_handle_t){
                EXAMPLE_TRACE("sub topic type[%d] qos:[%d] name : %s\n",node->topic_type, node->qos, node->topic_filter);
            }
            */
        }

        mqtt_client_yield(pclient, 100);
    }

    mqtt_client_close(pclient);

    return 0;
}

