set(3RDPARTY_DIR "${PROJECT_DIR}/3rdparty")

# Set model type
set(MODEL_IS_QUANT true)
if(MODEL_IS_QUANT)
    set(MODEL_DIR "model_quant")
    set(MODEL_INPUT_BYTES 1)
    set(MODEL_OUTPUT_BYTES 1)
else()
    set(MODEL_DIR "model")
    set(MODEL_INPUT_BYTES 4)
    set(MODEL_OUTPUT_BYTES 4)
endif()


# Set kws srcs and incs
set(KWS_DIR "./KWS/")
set(KWS_SRC
    "${KWS_DIR}/kws.cpp"
    "${KWS_DIR}/${MODEL_DIR}/codegen/host/src/kws_lib0.c"
    "${KWS_DIR}/${MODEL_DIR}/codegen/host/src/kws_lib1.c"
    "${KWS_DIR}/${MODEL_DIR}/runtime/src/runtime/crt/memory/stack_allocator.c"
    "${KWS_DIR}/${MODEL_DIR}/runtime/src/runtime/crt/common/crt_backend_api.c"
)
set(KWS_INC
    "${KWS_DIR}/"
    "${KWS_DIR}/${MODEL_DIR}/runtime/include"
    "${KWS_DIR}/${MODEL_DIR}/codegen/host/include"
)


set(NMSIS_DIR "${3RDPARTY_DIR}/NMSIS/NMSIS/")
set(RISCV_MATH_SRC 
    "${NMSIS_DIR}/DSP/Source/FastMathFunctions/riscv_cos_f32.c"
    "${NMSIS_DIR}/DSP/Source/CommonTables/riscv_common_tables.c"
    "${NMSIS_DIR}/DSP/Source/CommonTables/riscv_const_structs.c"
    "${NMSIS_DIR}/DSP/Source/TransformFunctions/riscv_rfft_fast_f32.c"
    "${NMSIS_DIR}/DSP/Source/TransformFunctions/riscv_cfft_f32.c"
    "${NMSIS_DIR}/DSP/Source/TransformFunctions/riscv_cfft_radix8_f32.c"
    "${NMSIS_DIR}/DSP/Source/TransformFunctions/riscv_bitreversal2.c"
    "${NMSIS_DIR}/DSP/Source/TransformFunctions/riscv_rfft_fast_init_f32.c"
    "${NMSIS_DIR}/DSP/Source/TransformFunctions/riscv_cfft_init_f32.c")
set(RISCV_MATH_INC
    "${NMSIS_DIR}/Core/Include/"
    "${NMSIS_DIR}/DSP/Include/"
)


# Set micro srcs and incs
set(MFCC_SRC "${KWS_DIR}/mfcc_preprocessor/")
set(MIC_SRC
    "./MicReader/audio_rx_slot.cpp"
    "./MicReader/mic_reader.cpp"
    "./MicReader/utils.cpp"
    "./MicReader/fragment_detector.cpp"
    "./MicReader/mic_fragment_reader.cpp"
    "${MFCC_SRC}/mfcc_preprocessor.cpp"
    "${MFCC_SRC}/mfcc/mfcc.cpp"
    "${RISCV_MATH_SRC}"
)
set(MIC_INC
    "./MicReader/"
    "${MFCC_SRC}/"
    "${MFCC_SRC}/mfcc"
    "${RISCV_MATH_INC}"
)

set(GRC_SDK_DIR "${3RDPARTY_DIR}/grc_sdk")
set(GRC_SDK_SRC
    "${GRC_SDK_DIR}/grc/Grc.cpp"
    "${GRC_SDK_DIR}/grc/i2c/grc_i2c.c"
    "${GRC_SDK_DIR}/grc/i2c/crc_calculation.c"
    "${GRC_SDK_DIR}/grc/i2c/grc_ll_api.c"
    "${GRC_SDK_DIR}/grc/i2c/grc_ll_protocol_commands.c"
)
set(GRC_SDK_INC
    "${GRC_SDK_DIR}/"
)

set(U8G2_DIR "${3RDPARTY_DIR}/u8g2")
set(U8G2_SRC
    "${U8G2_DIR}/csrc/u8x8_setup.c"
    "${U8G2_DIR}/csrc/u8x8_8x8.c"
    "${U8G2_DIR}/csrc/u8x8_display.c"
    "${U8G2_DIR}/csrc/u8x8_cad.c"
    "${U8G2_DIR}/csrc/u8x8_byte.c"
    "${U8G2_DIR}/csrc/u8x8_gpio.c"
    "${U8G2_DIR}/csrc/u8x8_d_ssd1306_128x64_noname.c"
    "${U8G2_DIR}/csrc/u8g2_setup.c"
    "${U8G2_DIR}/csrc/u8g2_cleardisplay.c"
    "${U8G2_DIR}/csrc/u8g2_buffer.c"
    "${U8G2_DIR}/csrc/u8g2_fonts.c"
    "${U8G2_DIR}/csrc/u8g2_font.c"
    "${U8G2_DIR}/csrc/u8g2_hvline.c"
    "${U8G2_DIR}/csrc/u8g2_ll_hvline.c"
    "${U8G2_DIR}/csrc/u8g2_intersection.c"
    "${U8G2_DIR}/csrc/u8g2_d_setup.c"
    "${U8G2_DIR}/csrc/u8g2_d_memory.c"
    "${U8G2_DIR}/cppsrc/U8x8lib.cpp"
)
set(U8G2_INC
    "${U8G2_DIR}/cppsrc"
    "${U8G2_DIR}/csrc"
)

idf_component_register(
    SRCS "main.cpp"
    "./Grc/BaseGrc.cpp"
    "./Grc/VibroGrc.cpp"
    "./Grc/GestureGrc.cpp"
    "./Grc/RhythmGrc.cpp"

    "./App/GestureScenario.cpp"
    "./App/RhythmScenario.cpp"
    "./App/VibroScenario.cpp"
    "./App/App.cpp"
    "./App/KWS.cpp"
    "./App/Event.cpp"
    "./App/Listener.cpp"
    "./App/GrcController.cpp"
    "./App/Status.cpp"

    "./Common/Common.cpp"
    "./Common/Statistics.cpp"
    "./Common/Log.cpp"

    "Hardware/Accelerometer.cpp"
    "Hardware/Led.cpp"
    "Hardware/Lcd.cpp"                            
    "Hardware/Button.cpp"
    "Hardware/I2C.cpp"
    "Hardware/NVStorage.cpp"

    ${KWS_SRC} ${MIC_SRC}
    ${U8G2_SRC}
    ${GRC_SDK_SRC}

    INCLUDE_DIRS
    "./Common/include"
    "./Grc/include"
    "./App/include"
    "./Hardware"

    "${3RDPARTY_DIR}/NumCpp/include"
    ${KWS_INC} ${MIC_INC}
    ${U8G2_INC}
    ${GRC_SDK_INC}

    REQUIRES
    "led_strip"
    "nvs_flash"
    "esp_timer"
    "driver"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=unused-const-variable -Wno-error=delete-non-virtual-dtor -Wno-error=implicit-function-declaration -fpermissive)
target_compile_options(${COMPONENT_LIB} PUBLIC "-DLOG_LOCAL_LEVEL=ESP_LOG_INFO")
add_compile_definitions(USE_LOG=1)
add_compile_definitions(LOG_LEVEL=LOG_LEVEL_INFO)
# add_compile_definitions(NO_LCD)

add_compile_definitions(NUMCPP_NO_USE_BOOST)
add_compile_definitions(U8X8_USE_PINS)

# Set defines
add_compile_definitions(A_SAMPLE_RATE=16000)
add_compile_definitions(A_BITS_PER_SAMPLE_DATA_16BIT)
add_compile_definitions(A_HW_BITS_PER_SAMPLE_DATA_16BIT)
add_compile_definitions(A_VOLUME_X32)
add_compile_definitions(USE_AVERAGE_SIG)
add_compile_definitions(A_MFCC_ENVIRONMENT_PREPARED_TEMPLATE)
add_compile_definitions(GET_MFCC=1)
add_compile_definitions(KWS_NUM_MFCC=10)
add_compile_definitions(KWS_WIN_MS=40)
add_compile_definitions(KWS_STRIDE_MS=20)

if(MODEL_IS_QUANT)
    add_compile_definitions(KWS_INPUT_SCALE=1.0835615396499634)
    add_compile_definitions(KWS_INPUT_ZERO=100)
    add_compile_definitions(KWS_OUTPUT_SCALE=0.00390625)
    add_compile_definitions(KWS_OUTPUT_ZERO=-128)
endif()
add_compile_definitions(MODEL_IN_BYTES=${MODEL_INPUT_BYTES})
add_compile_definitions(MODEL_OUT_BYTES=${MODEL_OUTPUT_BYTES})
