/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#include "infra_config.h"

#ifdef INFRA_REPORT

#include <stdio.h>
#include <string.h>
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_string.h"
#include "infra_report.h"

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
void HAL_Printf(const char *fmt, ...);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);
int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN]);
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN]);
uint64_t HAL_UptimeMs(void);
int HAL_GetFirmwareVersion(char *version);

#ifdef INFRA_MEM_STATS
    #include "infra_mem_stats.h"
    #define SYS_REPORT_MALLOC(size) LITE_malloc(size, MEM_MAGIC, "sys.report")
    #define SYS_REPORT_FREE(ptr)    LITE_free(ptr)
#else
    #define SYS_REPORT_MALLOC(size) HAL_Malloc(size)
    #define SYS_REPORT_FREE(ptr)    HAL_Free(ptr)
#endif

#ifdef INFRA_LOG
    #include "infra_log.h"
    #define VERSION_DEBUG(...)  log_debug("version", __VA_ARGS__)
    #define VERSION_ERR(...)    log_err("version", __VA_ARGS__)
#else
    #define VERSION_DEBUG(...)  do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
    #define VERSION_ERR(...)    do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#endif

static int g_report_id = 0;

int iotx_report_id(void)
{
    if (++g_report_id < 0) {
        g_report_id = 0;
    }
    return g_report_id;
}

static info_report_func_pt info_report_func = NULL;

void iotx_set_report_func(info_report_func_pt func)
{
    info_report_func = func;
}
/*  aos will implement this function */
#if defined(BUILD_AOS)
extern void aos_get_version_hex(unsigned char version[VERSION_NUM_SIZE]);
#else
void aos_get_version_hex(unsigned char version[VERSION_NUM_SIZE])
{
    const char *p_version = IOTX_SDK_VERSION;
    int i = 0, j = 0;
    unsigned char res = 0;

    for (j = 0; j < 3; j++) {
        for (res = 0; p_version[i] <= '9' && p_version[i] >= '0'; i++) {
            res = res * 10 + p_version[i] - '0';
        }
        version[j] = res;
        i++;
    }
    version[3] = 0x00;
}
#endif



/*  aos will implement this function */
#if defined(BUILD_AOS)
extern void aos_get_mac_hex(unsigned char mac[MAC_ADDRESS_SIZE]);
#else
void aos_get_mac_hex(unsigned char mac[MAC_ADDRESS_SIZE])
{
    memcpy(mac, "\x01\x02\x03\x04\x05\x06\x07\x08", MAC_ADDRESS_SIZE);
}
#endif

/*  aos will implement this function */
#if defined(BUILD_AOS)
extern void aos_get_chip_code(unsigned char chip_code[CHIP_CODE_SIZE]);
#else
void aos_get_chip_code(unsigned char chip_code[CHIP_CODE_SIZE])
{
    memcpy(chip_code, "\x01\x02\x03\x04", CHIP_CODE_SIZE);
}
#endif

const char *DEVICE_INFO_UPDATE_FMT = "{\"id\":\"%d\",\"version\":\"1.0\",\"params\":["
                                     "{\"attrKey\":\"SYS_LP_SDK_VERSION\",\"attrValue\":\"%s\",\"domain\":\"SYSTEM\"},"
                                     "{\"attrKey\":\"SYS_SDK_LANGUAGE\",\"attrValue\":\"C\",\"domain\":\"SYSTEM\"}"
                                     "],\"method\":\"thing.deviceinfo.update\"}";




/* report ModuleID */
int iotx_report_mid(void *pclient)
{
    return SUCCESS_RETURN;
}

#ifndef BUILD_AOS
unsigned int aos_get_version_info(unsigned char version_num[VERSION_NUM_SIZE],
                                  unsigned char random_num[RANDOM_NUM_SIZE], unsigned char mac_address[MAC_ADDRESS_SIZE],
                                  unsigned char chip_code[CHIP_CODE_SIZE], unsigned char *output_buffer, unsigned int output_buffer_size)
{
    char *p = (char *)output_buffer;

    if (output_buffer_size < AOS_ACTIVE_INFO_LEN) {
        return 1;
    }

    memset(p, 0, output_buffer_size);

    infra_hex2str(version_num, VERSION_NUM_SIZE, p);
    p += VERSION_NUM_SIZE * 2;
    infra_hex2str(random_num, RANDOM_NUM_SIZE, p);
    p += RANDOM_NUM_SIZE * 2;
    infra_hex2str(mac_address, MAC_ADDRESS_SIZE, p);
    p += MAC_ADDRESS_SIZE * 2;
    infra_hex2str(chip_code, CHIP_CODE_SIZE, p);
    p += CHIP_CODE_SIZE * 2;
    strcat(p, "1111111111222222222233333333334444444444");

    return 0;
}
#endif
#endif

