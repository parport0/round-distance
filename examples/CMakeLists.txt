aux_source_directory(./src DIR_examples_SRCS)

include_directories(../lib/Config)
include_directories(../lib/LCD)
include_directories(../lib/Touch)
include_directories(../lib/lvgl)
include_directories(./inc)

add_library(examples ${DIR_examples_SRCS})
target_link_libraries(examples PUBLIC Config LCD Touch LVGL hardware_clocks hardware_rtc hardware_adc hardware_dma hardware_spi pico_multicore)
