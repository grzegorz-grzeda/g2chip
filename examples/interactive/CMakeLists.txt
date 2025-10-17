# SPDX-License-Identifier: MIT
#
project(g2chip-interactive)

add_executable(${PROJECT_NAME} 
    main.c
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE g2chip
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE SDL2
)