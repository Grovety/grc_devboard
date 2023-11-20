#ifndef _AUDIO_RX_SLOT_H_
#define _AUDIO_RX_SLOT_H_

#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <driver/i2s_types.h>
#include <driver/i2s_std.h>
#include <driver/i2s_pdm.h>
#include <driver/gpio.h>

#include "def.h"

#define TST_ESP_OK(f)  {errors += ((f == ESP_OK) ? 0 : 1);}

typedef struct IOPlan {
    uint32_t dma_frame_num;
    uint32_t dma_desc_num;
    uint32_t polling_buffer_size;
    uint32_t read_fragment_size;
} IOPlan;

class AudioRxSlot {
public:

    AudioRxSlot(int bck_pin, int ws_pin, int data_pin, int channel_pin = -1);
    ~AudioRxSlot();

    int begin(uint32_t sample_rate, IOPlan& io_plan, int port_number = 0, eBitWidth bit_width = bwBitWidth32, 
        eSlotType slot_type = stStdRight, eSlotMode slot_mode = smMono, eBitShift bit_shift = bsEnable);
    void end();
    static void plan(uint32_t sample_rate, uint32_t data_bit_width, uint32_t slot_num,
        uint32_t polling_time_msec, uint32_t fragment_time_msec, IOPlan& io_plan);
    size_t get_fragment_size();
    size_t get_buffer_size();
    size_t read(void* fragment);
    size_t read(void* fragment, size_t bytes);
  
private:

    gpio_num_t mclk_pin_;
    gpio_num_t bck_pin_;
    gpio_num_t ws_pin_;
    gpio_num_t din_pin_;
    gpio_num_t dout_pin_;
    gpio_num_t channel_pin_;
    size_t bytes_width_;
    i2s_chan_handle_t rx_handle_;
    size_t fragment_size_;
    size_t buffer_size_;
};


#endif // _AUDIO_RX_SLOT_H_
