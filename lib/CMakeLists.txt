file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/*.c)

add_library(LVGL ${SOURCES})
target_compile_definitions(LVGL PRIVATE -DLV_CONF_PATH=${CMAKE_CURRENT_LIST_DIR}/../examples/inc/lv_conf.h)

include_directories(${CMAKE_SOURCE_DIR}/lvgl/examples/inc)
include_directories(${CMAKE_CURRENT_LIST_DIR}/lvgl)
