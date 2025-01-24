cmake_minimum_required(VERSION 3.16)
project(synthesizer_tryout)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/cmake")

include_directories(include)

find_package(raylib)

find_package(Jack REQUIRED)

set(OSCILLATOR_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/oscillator.c
  ${CMAKE_CURRENT_SOURCE_DIR}/include/oscillator.h
  )

add_library(oscillator)
target_sources(oscillator
 PRIVATE
  ${OSCILLATOR_SOURCES}
  )

target_include_directories(oscillator
 PRIVATE
  ${raylib_INCLUDE_DIRS}
  )

target_link_libraries(oscillator
  ${raylib_LIBRARIES}
  m
  )

set(RAY_OUT_BUFFER_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ray_out_buffer.c
  ${CMAKE_CURRENT_SOURCE_DIR}/include/ray_out_buffer.h
  )

add_library(ray_out_buffer)
target_sources(ray_out_buffer
 PRIVATE
  ${RAY_OUT_BUFFER_SOURCES}
  )

target_include_directories(ray_out_buffer
 PRIVATE
 ${INCLUDE}
 )

set(JACK_STUFF_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/jack_stuff.c
  ${CMAKE_CURRENT_SOURCE_DIR}/include/jack_stuff.h
  )

add_library(jack_stuff)
target_sources(jack_stuff
 PRIVATE
  ${JACK_STUFF_SOURCES}
  )

target_include_directories(jack_stuff
 PRIVATE
 ${INCLUDE}
 )

target_link_libraries(jack_stuff
  ${JACK_LIBRARIES}
  )

set(SINE_WAVE_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sine_wave.c
  )

add_executable(sine_wave)
target_sources(sine_wave
 PRIVATE
  ${SINE_WAVE_SRC}
  )

target_include_directories(sine_wave
  PRIVATE
  ${raylib_INCLUDE_DIRS}
  ${JACK_INCLUDE_DIRS}
  )

target_link_libraries(sine_wave
  ${raylib_LIBRARIES}
  ${JACK_LIBRARIES}
  m
  oscillator
  ray_out_buffer
  jack_stuff
  ui_stuff
  )

set(SINGLE_THREAD_EX_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/single_thread_example.c
  )

add_executable(single_thread_example)
target_sources(single_thread_example
 PRIVATE
  ${SINGLE_THREAD_EX_SRC}
  )

set(CONSUMER_PRODUCER1_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/consumer_producer1.c
  )

add_executable(consumer_producer1)
target_sources(consumer_producer1
 PRIVATE
  ${CONSUMER_PRODUCER1_SRC}
  )

set(CONSUMER_PRODUCER_WITH_BUF_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/consumer_producer_with_buf.c
  )

add_executable(consumer_producer_with_buf)
target_sources(consumer_producer_with_buf
 PRIVATE
  ${CONSUMER_PRODUCER_WITH_BUF_SRC}
  )

target_link_libraries(consumer_producer_with_buf
${JACK_LIBRARIES}
)

set(CONSUMER_PRODUCER_SINE_WAVE_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/consumer_producer_sine_wave.c
  )

add_executable(consumer_producer_sine_wave)
target_sources(consumer_producer_sine_wave
 PRIVATE
  ${CONSUMER_PRODUCER_SINE_WAVE_SRC}
  )

target_link_libraries(consumer_producer_sine_wave
  ${JACK_LIBRARIES}
  oscillator
  )

set(CONSUMER_PRODUCER_SINE_WAVE_JACK_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/consumer_producer_sine_wave_jack.c
  )

add_executable(consumer_producer_sine_wave_jack)
target_sources(consumer_producer_sine_wave_jack
 PRIVATE
  ${CONSUMER_PRODUCER_SINE_WAVE_JACK_SRC}
  )

target_link_libraries(consumer_producer_sine_wave_jack
  ${JACK_LIBRARIES}
  oscillator
  )

set(UI_STUFF_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ui_stuff.c
  ${CMAKE_CURRENT_SOURCE_DIR}/include/ui_stuff.h
  )

add_library(ui_stuff)
target_sources(ui_stuff
 PRIVATE
  ${UI_STUFF_SOURCES}
  )

target_include_directories(ui_stuff
 PRIVATE
 ${INCLUDE}
 )

target_link_libraries(ui_stuff
  ${RAYLIB}
  ray_out_buffer
  )

set(SINE_WAVE_UI_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sine_wave_ui.c
  )

add_executable(sine_wave_ui)
target_sources(sine_wave_ui
 PRIVATE
  ${SINE_WAVE_UI_SRC}
  )

target_link_libraries(sine_wave_ui
  ${RAYLIB}
  oscillator
  ui_stuff
  )
