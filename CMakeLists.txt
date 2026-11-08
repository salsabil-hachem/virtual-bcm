# Recette de compilation du projet
cmake_minimum_required(VERSION 3.16)
project(virtual_bcm C)

set(CMAKE_C_STANDARD 11)

# Le code du BCM (sans FreeRTOS ni CAN pour l'instant)
add_library(bcm_logic src/turn_signal.c)
target_include_directories(bcm_logic PUBLIC include)
target_compile_options(bcm_logic PRIVATE -Wall -Wextra -Werror)

# Unity (outil de test) est telecharge automatiquement
include(FetchContent)
FetchContent_Declare(unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.0
)
FetchContent_MakeAvailable(unity)

# Les tests
enable_testing()
add_executable(test_turn_signal tests/unit/test_turn_signal.c)
target_link_libraries(test_turn_signal PRIVATE bcm_logic unity)
add_test(NAME test_turn_signal COMMAND test_turn_signal)
