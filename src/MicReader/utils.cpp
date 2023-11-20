#define _UTILS_CPP_

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "def.h"
#define NORM_24BIT(src, H_X, M_X, L_X) { \
int32_t data = ((((int32_t)(src[L_X])) & 0xff) | ((((int32_t)(src[M_X])) << 8) & 0xff00)  | ((((int32_t)(src[H_X])) << 16) & 0xff0000)) << 8; \
return ((data < 0) ? -((float)(-data)) : ((float)data)) / ((float)0x7fffff00); }

float norm_24bit_msb(unsigned char* src) {
    NORM_24BIT(src, 0, 1, 2);
}

float norm_24bit_lsb(unsigned char* src) {
    NORM_24BIT(src, 2, 1, 0);
}

// Print data samples
void mon(void* buf, size_t bytes, size_t byte_width) {
    if (!buf || !bytes || !byte_width)
        return;

    uint8_t* ss = reinterpret_cast<uint8_t*>(buf);
    size_t cnt = bytes / byte_width;
    size_t i = 0;
    
    DEBUG_PRINTF("[%d]:\n", cnt);
    while(cnt) {
        for (size_t k = 0; (k < A_MON_WORDS_PER_LINE) && cnt; k++) {
            switch(byte_width) {
            case 1:
                DEBUG_PRINTF("%02x ", ss[i++]);
                break;
            case 2:
                DEBUG_PRINTF("%02x %02x  ", ss[i], ss[i + 1]);
                i += 2;
                break;
            case 3:
                DEBUG_PRINTF("%02x %02x %02x  ", ss[i], ss[i + 1], ss[i + 2]);
                i += 3;
                break;
            case 4:
                DEBUG_PRINTF("%02x %02x %02x %02x  ", ss[i], ss[i + 1], ss[i + 2], ss[i + 3]);
                i += 4;
                break;
            }
            if (!((cnt--) & 0x7ff))
                vTaskDelay(1);
        }
        DEBUG_PRINTF("\n");
    }
}

// Print mels 
void mon_mfcc(float* buf, size_t len) {
    if (!buf || !len)
        return;

    size_t n = 0;
    DEBUG_PRINTF("[%d]:\n", len);
    for (size_t k = 0; k < len;) {
        DEBUG_PRINTF("%f \t", buf[k]);
        if ((++n) >= A_MON_MFCC_WORDS_PER_LINE) {
            DEBUG_PRINTF("\n");
            n = 0;           
        }
        if (!((++k) & 0x7ff))
            vTaskDelay(1);
    }
    if (n)
        DEBUG_PRINTF("\n");      
}
