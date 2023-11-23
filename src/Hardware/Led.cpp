#include "Led.hpp"
#include "Common.hpp"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "led_strip.h"

struct RGB {
  uint32_t r, g, b;
  RGB operator>>(uint32_t val) const { return {r >> val, g >> val, b >> val}; }
} static const colour_table[] = {
    /* Red */ {100, 0, 0},
    /* Green */ {0, 100, 0},
    /* Blue */ {0, 0, 100},
    /* Yellow */ {70, 70, 0},
    /* White */ {70, 70, 70},
    /* Cyan */ {0, 100, 100},
    /* Black */ {0, 0, 0}};

#define LED_GPIO_PIN GPIO_NUM_46
#define LED_STRIP_LEN 3
#define LED_MODEL LED_MODEL_WS2812

static led_strip_handle_t s_led_strip;

static void led_open() {
  // LED strip general initialization, according to your led board design
  led_strip_config_t strip_config = {
      .strip_gpio_num =
          LED_GPIO_PIN, // The GPIO that connected to the LED strip's data line
      .max_leds = LED_STRIP_LEN, // The number of LEDs in the strip,
      .led_pixel_format =
          LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
      .led_model = LED_MODEL,   // LED strip model
      .flags = false,           // whether to invert the output signal
  };

  // LED strip backend configuration: RMT
  led_strip_rmt_config_t rmt_config = {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    .rmt_channel = 0,
#else
    .clk_src = RMT_CLK_SRC_DEFAULT, // different clock source can lead to
                                    // different power consumption
    .resolution_hz = 0,             // RMT counter clock frequency
    .mem_block_symbols =
        0,          // How many RMT symbols can one RMT channel hold at one time
    .flags = false, // DMA feature is available on ESP target like ESP32-S3
#endif
  };

  ESP_ERROR_CHECK(
      led_strip_new_rmt_device(&strip_config, &rmt_config, &s_led_strip));
}

static void colorWipe(RGB c, uint32_t wait) {
  for (int i = 0; i < LED_STRIP_LEN; i++) {
    ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, i, c.r, c.g, c.b));
    /* Refresh the strip to send data */
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
    delayMS(wait);
  }
}

static void colorSetPixel(RGB c, uint32_t pixel) {
  ESP_ERROR_CHECK(
      led_strip_set_pixel(s_led_strip, pixel % LED_STRIP_LEN, c.r, c.g, c.b));
  /* Refresh the strip to send data */
  ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
}

Led::Led() : ILed(LED_GPIO_PIN) {
  led_open();
  set(false);
}

void Led::set(bool state) { set(state ? White : Black); }

void Led::set(Colour colour, int led_num, Brightness b) {
  if (led_num < 0) {
    colorWipe(colour_table[colour] >> b, 0);
  } else {
    colorSetPixel(colour_table[colour] >> b, led_num);
  }
}

void Led::flash(ILed::Colour colour, unsigned nums) {
  const unsigned dur = 100;
  set(false);
  while (nums--) {
    set(colour);
    delayMS(dur / 2);
    set(false);
    if (nums)
      delayMS(dur);
  }
}
