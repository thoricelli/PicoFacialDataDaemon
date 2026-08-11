cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 17)

project(picofacialdatadaemon CXX)

set(SOURCES
    src/main.cpp
    src/pvr/pxr-eye-tracking-service.cpp
    src/facial-tracking-socket.cpp
    src/facial-tracking.cpp
    src/data-buffer.cpp
)

#PICO Android OS libs
add_library(binder SHARED IMPORTED)
set_target_properties(binder PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/${ANDROID_ABI}/libbinder.so
)

add_library(utils SHARED IMPORTED)
set_target_properties(utils PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/${ANDROID_ABI}/libutils.so
)

add_library(cutils SHARED IMPORTED)
set_target_properties(cutils PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/${ANDROID_ABI}/libcutils.so
)

add_library(base SHARED IMPORTED)
set_target_properties(base PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/${ANDROID_ABI}/libbase.so
)

add_library(binder_ndk_real SHARED IMPORTED)
set_target_properties(binder_ndk_real PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/${ANDROID_ABI}/libbinder_ndk.so
)

add_executable(picofacialdatadaemon ${SOURCES})

target_compile_options(picofacialdatadaemon PRIVATE 
    -fno-rtti
    -fno-exceptions

    # Debug Configuration (No optimization, maximum debuggability)
    $<$<CONFIG:Debug>:
        -O0
        -g3
        -fno-omit-frame-pointer
    >

    # Release Configuration (Maximum optimization)
    $<$<CONFIG:Release>:
        -O3
        -flto
        -ffunction-sections
        -fdata-sections
        -fvisibility=hidden
    >
)

target_compile_definitions(picofacialdatadaemon PRIVATE
    $<$<CONFIG:Release>:NDEBUG>
)

target_link_options(picofacialdatadaemon PRIVATE
    -Wl,--allow-shlib-undefined

    $<$<CONFIG:Release>:
        -flto
        -Wl,--gc-sections
        -Wl,-s
    >
)

target_include_directories(picofacialdatadaemon PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(picofacialdatadaemon PRIVATE
    third_party/native/libs/binder/include
    third_party/native/libs/binder/ndk/include_apex
    third_party/core/libutils/include
    third_party/core/liblog/include
    third_party/core/base/include
    third_party/core/libcutils/include
    third_party/core/libsystem/include
)
target_link_libraries(picofacialdatadaemon 
    binder
    utils
    cutils
    base
    binder_ndk_real
    log
)