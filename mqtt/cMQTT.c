#include "cMQTT.h"
#include "infra_log.h"
#include "infra_list.h"
#include "infra_defs.h"
#include <string.h>
#include "mqtt_api.h"
#include "hal.h"
#include "mqtt_wrapper.h"


void iotx_mc_pub_wait_list_init(iotx_mc_client_t *pClient);
void iotx_mc_set_client_state(iotx_mc_client_t *pClient, iotx_mc_state_t newState);
void iotx_mc_release(iotx_mc_client_t *pclient);


static void mqtt_connect_data_common_init(MQTTPacket_connectData *connect_data,
                                        int MQTTVersion)
{
    /** The eyecatcher for this structure.  must be MQTC. */
    connect_data->struct_id[0] = 'M';
    connect_data->struct_id[1] = 'Q';
    connect_data->struct_id[2] = 'T';
    connect_data->struct_id[0] = 'C';
    /** The version number of this structure.  Must be 0 */
    connect_data->struct_version = 0;
    /** Version of MQTT to be used.  3 = 3.1 4 = 3.1.1
      */
    connect_data->MQTTVersion = MQTTVersion;
    /** The eyecatcher for this structure.  must be MQTW. */
    connect_data->will.struct_id[0] = 'M';
    connect_data->will.struct_id[1] = 'Q';
    connect_data->will.struct_id[2] = 'T';
    connect_data->will.struct_id[3] = 'W';
    /** The version number of this structure.  Must be 0 */
    connect_data->will.struct_version = 0;
    
    connect_data->cleansession = 0;

    connect_data->willFlag = 0;
}


static int mqtt_client_common_init(mqtt_client_t *pClient,
                            const char *host,
                            int         port,
                            const char *ca_crt,
                            const char *client_id,
                            const char *username,
                            const char *password)
{
    int rc = FAIL_RETURN;
    int len_tmp;
    iotx_mc_state_t mc_state = IOTX_MC_STATE_INVALID;
    MQTTPacket_connectData *connectdata = NULL;;

    if (pClient == NULL || host == NULL || client_id == NULL || 
        username == NULL || password == NULL) {
        return NULL_VALUE_ERROR;
    }
    // init MQTT conectdata
    connectdata = &(pClient->connect_data);

    pClient->lock_generic = HAL_MutexCreate();
    if (!pClient->lock_generic) {
        return FAIL_RETURN;
    }

    pClient->lock_list_pub = HAL_MutexCreate();
    if (!pClient->lock_list_pub) {
        goto RETURN;
    }

    pClient->lock_yield = HAL_MutexCreate();
    if (!pClient->lock_yield) {
        goto RETURN;
    }

    pClient->lock_write_buf = HAL_MutexCreate();
    if (!pClient->lock_write_buf) {
        goto RETURN;
    }

    pClient->lock_read_buf = HAL_MutexCreate();
    if (!pClient->lock_read_buf) {
        goto RETURN;
    }
    // host
    len_tmp = strlen(host);
    pClient->buf_host_name = (char *)HAL_Malloc(len_tmp + 1);
    if(pClient->buf_host_name == NULL){
        HAL_Printf("malloc host name buffer fail.\n");
        goto RETURN;
    }
    memset(pClient->buf_host_name, 0, len_tmp + 1);
    strncpy(pClient->buf_host_name, host, len_tmp);
    pClient->ipstack.pHostAddress = pClient->buf_host_name;
    // port
    pClient->ipstack.port = port;
    // request timeout 
    pClient->request_timeout_ms = CONFIG_MQTT_REQUEST_TIMEOUT;

#ifdef PLATFORM_HAS_DYNMEM
#if !( WITH_MQTT_DYN_BUF)
    pClient->buf_send = mqtt_malloc(pInitParams->write_buf_size);
    if (pClient->buf_send == NULL) {
        goto RETURN;
    }
    pClient->buf_size_send = pInitParams->write_buf_size;

    pClient->buf_read = mqtt_malloc(pInitParams->read_buf_size);
    if (pClient->buf_read == NULL) {
        goto RETURN;
    }
    pClient->buf_size_read = pInitParams->read_buf_size;
#else
    pClient->buf_size_send_max = CONFIG_MQTT_BUFFER_SEND_MAX;   
    pClient->buf_size_read_max = CONFIG_MQTT_BUFFER_READ_MAX;
#endif
#else
    pClient->buf_size_send = IOTX_MC_TX_MAX_LEN;
    pClient->buf_size_read = IOTX_MC_RX_MAX_LEN;
#endif

    pClient->keepalive_probes = 0;
    pClient->handle_event.h_fp = NULL;
    pClient->handle_event.pcontext = NULL;
    /* Initialize reconnect parameter */
    pClient->reconnect_param.reconnect_time_interval_ms = IOTX_MC_RECONNECT_INTERVAL_MIN_MS;
    
#if !WITH_MQTT_ONLY_QOS0
    iotx_mc_pub_wait_list_init(pClient);
#endif

#ifdef PLATFORM_HAS_DYNMEM
    INIT_LIST_HEAD(&pClient->list_sub_handle);
    INIT_LIST_HEAD(&pClient->list_sub_sync_ack);
#endif
    /* Initialize MQTT connect parameter */
    // connect data init
    mqtt_connect_data_common_init(connectdata, IOTX_MC_MQTT_VERSION); // import
    // client id
    len_tmp = strlen(client_id);
    pClient->buf_client_id = (char *)HAL_Malloc(len_tmp + 1);
    if(pClient->buf_client_id == NULL){
        HAL_Printf("malloc client id buffer fail.\n");
        goto RETURN;
    }
    memset(pClient->buf_client_id, 0, len_tmp + 1);
    strncpy(pClient->buf_client_id, client_id, len_tmp);
    connectdata->clientID.cstring = pClient->buf_client_id;
    // username
    len_tmp = strlen(username);
    pClient->buf_username = (char *)HAL_Malloc(len_tmp + 1);
    if(pClient->buf_username == NULL){
        HAL_Printf("malloc username buffer fail.\n");
        goto RETURN;
    }
    memset(pClient->buf_username, 0, len_tmp + 1);
    strncpy(pClient->buf_username, username, len_tmp);
    connectdata->username.cstring = pClient->buf_username;
    // password
    len_tmp = strlen(password);
    pClient->buf_password = (char *)HAL_Malloc(len_tmp + 1);
    if(pClient->buf_password == NULL){
        HAL_Printf("malloc password buffer fail.\n");
        goto RETURN;
    }
    memset(pClient->buf_password, 0, len_tmp + 1);
    strncpy(pClient->buf_password, password, len_tmp);
    connectdata->password.cstring = pClient->buf_password;

    connectdata->keepAliveInterval = CONFIG_MQTT_KEEPALIVE_INTERVAL;

    iotx_time_init(&pClient->next_ping_time);
    iotx_time_init(&pClient->reconnect_param.reconnect_next_time);

    // network init
    memset(&pClient->ipstack, 0, sizeof(utils_network_t));
    rc = iotx_net_init(&pClient->ipstack, host, port, ca_crt);
    if (SUCCESS_RETURN != rc) {
        mc_state = IOTX_MC_STATE_INVALID;
        goto RETURN;
    }
    mc_state = IOTX_MC_STATE_INITIALIZED;
    rc = SUCCESS_RETURN;
    HAL_Printf("MQTT init success!");

RETURN :
    iotx_mc_set_client_state(pClient, mc_state);
    if (rc != SUCCESS_RETURN) {
#ifdef PLATFORM_HAS_DYNMEM
        if (pClient->buf_send != NULL) {
            HAL_Free(pClient->buf_send);
            pClient->buf_send = NULL;
        }
        if (pClient->buf_read != NULL) {
            HAL_Free(pClient->buf_read);
            pClient->buf_read = NULL;
        }
#endif
        if (pClient->lock_list_pub) {
            HAL_MutexDestroy(pClient->lock_list_pub);
            pClient->lock_list_pub = NULL;
        }
        if (pClient->lock_write_buf) {
            HAL_MutexDestroy(pClient->lock_write_buf);
            pClient->lock_write_buf = NULL;
        }
        if (pClient->lock_read_buf) {
            HAL_MutexDestroy(pClient->lock_read_buf);
            pClient->lock_read_buf = NULL;
        }
        if (pClient->lock_yield) {
            HAL_MutexDestroy(pClient->lock_yield);
            pClient->lock_yield = NULL;
        }


        if(pClient->buf_host_name != NULL){
            HAL_Free(pClient->buf_host_name);
            pClient->buf_host_name = NULL;
        }
        if(pClient->buf_client_id != NULL){
            HAL_Free(pClient->buf_client_id);
            pClient->buf_client_id = NULL;
        }
        if(pClient->buf_username != NULL){
            HAL_Free(pClient->buf_username);
            pClient->buf_username = NULL;
        }
        if(pClient->buf_password != NULL){
            HAL_Free(pClient->buf_password);
            pClient->buf_password = NULL;
        }
    }
    return rc;
}


/*----------------- public API interface -----------------------------*/
void mqtt_set_connect_success_callback(mqtt_client_t *pclient, connect_success_callback_fpt callback_func)
{
    pclient->connect_success_callback = callback_func;
}

void mqtt_set_disconnected_callback(mqtt_client_t *pclient, disconnected_callback_fpt callback_func)
{
    pclient->disconnected_callback = callback_func;
}



mqtt_client_t *mqtt_client_new(const char *host, 
                            int port,
                            const char *ca_crt,
                            const char *client_id,
                            const char *username,
                            const char *password)
{
    int err;
    mqtt_client_t *pclient = NULL;

    pclient = (mqtt_client_t *)HAL_Malloc(sizeof(mqtt_client_t));
    if(pclient == NULL){
        HAL_Printf("not enough memory.\n");
        return NULL;
    }
    memset(pclient, 0, sizeof(mqtt_client_t));

    // initialize MQTT client
    err = mqtt_client_common_init(pclient, host, port, ca_crt, client_id, username, password);
    if(err != SUCCESS_RETURN){
        HAL_Printf("mqtt client common init fail.\n");
        iotx_mc_release(pclient);
        return NULL;
    }
    
    return pclient;
}

int mqtt_set_clean_session(mqtt_client_t *pclient, int clean_session)
{

}

int mqtt_set_keepalive_interval_second(mqtt_client_t *pclient, int keep_alive_second)
{
    pclient->connect_data.keepAliveInterval = keep_alive_second;
}

int mqtt_set_will_opt(mqtt_client_t *pclient, 
                    mqtt_qos_t qos, int retain,
                    const char *topic_name,
                    const char *will_message)
{
    int len;
    if(pclient == NULL || topic_name == NULL || will_message == NULL){
        return FAIL_RETURN;
    }

    len = strlen(topic_name);
    pclient->buf_will_topic = (char *)HAL_Malloc(len + 1);
    if(pclient->buf_will_topic == NULL){
        HAL_Printf("malloc will topic bufferr fail.\n");
        return FAIL_RETURN;
    }
    memset(pclient->buf_will_topic, 0, len + 1);
    strncpy(pclient->buf_will_topic, topic_name, len);

    len = strlen(will_message);
    pclient->buf_will_message = (char *)HAL_Malloc(len + 1);
    if(pclient->buf_will_message == NULL){
        HAL_Printf("malloc will message bufferr fail.\n");
        return FAIL_RETURN;
    }
    memset(pclient->buf_will_message, 0, len + 1);
    strncpy(pclient->buf_will_message, will_message, len);

    
    pclient->connect_data.willFlag = 1;
    pclient->connect_data.will.qos = qos;
    pclient->connect_data.will.retained = retain;
    // if will flag = 1, if topicName is NULL, MQTT client can not connect broker success
    pclient->connect_data.will.topicName.cstring = pclient->buf_will_topic; 
    pclient->connect_data.will.message.cstring = pclient->buf_will_message;

    return SUCCESS_RETURN;
}

int mqtt_client_connect(mqtt_client_t *pclient)
{
    return wrapper_mqtt_connect(pclient);
}

int mqtt_client_yield(mqtt_client_t *pclient, int timeout_ms)
{
    return wrapper_mqtt_yield(pclient, timeout_ms);
}

int mqtt_client_close(mqtt_client_t *pclient)
{
    void **client;
    if(pclient == NULL){
        HAL_Printf("pclient is NULL.\n");
        return NULL_VALUE_ERROR;
    }

    wrapper_mqtt_release((void *)&pclient);
    return SUCCESS_RETURN;
}

int mqtt_subscribe(mqtt_client_t *pclient,
                const char *topic_name,
                mqtt_qos_t qos,
                mqtt_event_callback_func_fpt topic_callback,
                void *pcontext)
{
    return IOT_MQTT_Subscribe(pclient, topic_name, qos, topic_callback, pcontext);
}


int mqtt_subscribe_sync(mqtt_client_t *pclient,
                        const char *topic_name,
                        mqtt_qos_t qos,
                        mqtt_event_callback_func_fpt topic_callback,
                        void *pcontext,
                        int timeout_ms)
{
    return IOT_MQTT_Subscribe_Sync(pclient, topic_name, qos, topic_callback, pcontext, timeout_ms);
}

int mqtt_unsubscribe(mqtt_client_t *pclient, const char *topic_name)
{
    if(pclient == NULL || topic_name == NULL || strlen(topic_name) == 0){
        HAL_Printf("unsubscribe params err.\n");
        return NULL_VALUE_ERROR;
    }

    return wrapper_mqtt_unsubscribe(pclient, topic_name);
}

int mqtt_publish(mqtt_client_t *pclient, const char *topic_name, mqtt_topic_info_t *msg)
{   
    int rv = -1;
    if(pclient == NULL || topic_name == NULL || strlen(topic_name) == 0){
        HAL_Printf("publish params err.\n");
        return NULL_VALUE_ERROR;
    }

    return wrapper_mqtt_publish(pclient, topic_name, msg);
}

int mqtt_publish_simple(mqtt_client_t *pclient, const char *topic_name, mqtt_qos_t qos, void *data, int len)
{
    mqtt_topic_info_t mqtt_msg;
    int rc = -1;

    if (pclient == NULL || topic_name == NULL || strlen(topic_name) == 0) {
        HAL_Printf("simple publish params err.\n");
        return NULL_VALUE_ERROR;
    }

    memset(&mqtt_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));

    mqtt_msg.qos         = qos;
    mqtt_msg.retain      = 0;
    mqtt_msg.dup         = 0;
    mqtt_msg.payload     = (void *)data;
    mqtt_msg.payload_len = len;

    rc = wrapper_mqtt_publish(pclient, topic_name, &mqtt_msg);

    if (rc < 0) {
        HAL_Printf("mqtt_publish_simple.\n");
        return -1;
    }
    return rc;
}


