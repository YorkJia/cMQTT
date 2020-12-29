#ifndef __C_MQTT_H
#define __C_MQTT_H

#include "iotx_mqtt_client.h"
#include "mqtt_api.h"

/* Maximum send buffer size */
#define CONFIG_MQTT_BUFFER_SEND_MAX      (100*1024)
/* Maximum read buffer size */
#define CONFIG_MQTT_BUFFER_READ_MAX      (1*1024)

typedef iotx_mc_client_t mqtt_client_t;

typedef enum {
    MQTT_QOS0 = 0,
    MQTT_QOS1,
    MQTT_QOS2,
    MQTT_QOS3_SUB_LOCAL
} mqtt_qos_t;


typedef iotx_mqtt_event_msg_t   mqtt_event_msg_t;

typedef void (*mqtt_event_callback_func_fpt)(void *pcontext, void *pclient, mqtt_event_msg_t *msg);

typedef iotx_mqtt_topic_info_t  mqtt_topic_info_t;

void mqtt_set_connect_success_callback(mqtt_client_t *pclient, connect_success_callback_fpt callback_func);

void mqtt_set_disconnected_callback(mqtt_client_t *pclient, disconnected_callback_fpt callback_func);


mqtt_client_t *mqtt_client_new(const char *host, 
                            int port,
                            const char *ca_crt,
                            const char *client_id,
                            const char *username,
                            const char *password);

int mqtt_set_clean_session(mqtt_client_t *pclient, int clean_session);

int mqtt_set_keepalive_interval_second(mqtt_client_t *pclient, int keep_alive_second);

int mqtt_set_will_opt(mqtt_client_t *pclient, 
                    mqtt_qos_t qos, int retain,
                    const char *topic_name,
                    const char *will_message);

int mqtt_client_connect(mqtt_client_t *pclient);

int mqtt_client_yield(mqtt_client_t *pclient, int timeout_ms);

int mqtt_client_close(mqtt_client_t *pclient);

int mqtt_subscribe(mqtt_client_t *pclient,
                const char *topic_name,
                mqtt_qos_t qos,
                mqtt_event_callback_func_fpt topic_callback,
                void *pcontext);

int mqtt_unsubscribe(mqtt_client_t *pclient, const char *topic_name);

int mqtt_publish(mqtt_client_t *pclient, 
            const char *topic_name, 
            mqtt_topic_info_t *msg);

int mqtt_publish_simple(mqtt_client_t *pclient, 
                    const char *topic_name, 
                    mqtt_qos_t qos, 
                    void *data, int len);







#endif