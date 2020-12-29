#include "infra_config.h"

#ifdef INFRA_COMPAT
#include <string.h>
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"

sdk_impl_ctx_t g_sdk_impl_ctx = {0};

#if !defined(INFRA_LOG)
void IOT_SetLogLevel(IOT_LogLevel level) {}
#endif

#ifdef MQTT_COMM_ENABLED
#include "mqtt_api.h"

#ifdef INFRA_LOG
    #include "infra_log.h"
    #define sdk_err(...)       log_err("infra_compat", __VA_ARGS__)
    #define sdk_info(...)      log_info("infra_compat", __VA_ARGS__)
#else
    #define sdk_err(...)
    #define sdk_info(...)
#endif

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);

#endif /* #ifdef MQTT_COMM_ENABLED */




#endif
