# Recette de compilation du projet
cmake_minimum_required(VERSION 3.16)
project(virtual_bcm C)

set(CMAKE_C_STANDARD 11)
include(FetchContent)

# ---------------------------------------------------------------
# 1. Le code du BCM : les regles + la traduction CAN (sans FreeRTOS)
# ---------------------------------------------------------------
add_library(bcm_logic
    src/turn_signal.c
    src/door_lock.c
    src/auto_lights.c
    src/alarm.c
    src/bcm.c
    src/bcm_comm.c
)
target_include_directories(bcm_logic PUBLIC include)
target_compile_options(bcm_logic PRIVATE -Wall -Wextra -Werror)

# Petit programme de test du branchement CAN
add_executable(can_test app/can_test.c app/can_socket.c)
target_include_directories(can_test PRIVATE app)
target_link_libraries(can_test PRIVATE bcm_logic)

# ---------------------------------------------------------------
# 2. FreeRTOS, version POSIX (tourne sur Linux), telecharge automatiquement
# ---------------------------------------------------------------
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE app)
set(FREERTOS_PORT GCC_POSIX CACHE STRING "" FORCE)
set(FREERTOS_HEAP 3 CACHE STRING "" FORCE)
FetchContent_Declare(freertos_kernel
    GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
    GIT_TAG        V11.1.0
)
FetchContent_MakeAvailable(freertos_kernel)

# ---------------------------------------------------------------
# 3. Le programme BCM complet (FreeRTOS + CAN)
# ---------------------------------------------------------------
add_executable(bcm app/main.c app/can_socket.c)
target_include_directories(bcm PRIVATE app)
target_link_libraries(bcm PRIVATE bcm_logic freertos_kernel freertos_config)

# ---------------------------------------------------------------
# 4. Les tests (Unity)
# ---------------------------------------------------------------
FetchContent_Declare(unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.0
)
FetchContent_MakeAvailable(unity)

enable_testing()
foreach(test_name test_turn_signal test_door_lock test_auto_lights test_alarm test_bcm test_bcm_comm)
    add_executable(${test_name} tests/unit/${test_name}.c)
    target_link_libraries(${test_name} PRIVATE bcm_logic unity)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
