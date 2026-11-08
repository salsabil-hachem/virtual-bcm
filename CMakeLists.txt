# Recette de compilation du projet
cmake_minimum_required(VERSION 3.16)
project(virtual_bcm C)

set(CMAKE_C_STANDARD 11)

# Le code du BCM (sans FreeRTOS ni CAN pour l'instant)
add_library(bcm_logic
    src/turn_signal.c
    src/door_lock.c
    src/auto_lights.c
)
target_include_directories(bcm_logic PUBLIC include)
target_compile_options(bcm_logic PRIVATE -Wall -Wextra -Werror)

# Unity (outil de test) est telecharge automatiquement
include(FetchContent)
FetchContent_Declare(unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.0
)
FetchContent_MakeAvailable(unity)

# Les tests : un programme de test par fonction du BCM
enable_testing()
foreach(test_name test_turn_signal test_door_lock test_auto_lights)
    add_executable(${test_name} tests/unit/${test_name}.c)
    target_link_libraries(${test_name} PRIVATE bcm_logic unity)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
