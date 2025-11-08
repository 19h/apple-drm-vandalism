cmake_minimum_required(VERSION 3.20)

project(AppleMusicWrapper LANGUAGES C CXX)

# --- Gengetopt Code Generation ---
# Find the gengetopt executable instead of the package.
# This is more robust as gengetopt often doesn't install CMake config files.
find_program(GENGETOPT_EXECUTABLE gengetopt REQUIRED)

set(GGO_INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/wrapper.ggo)
set(GGO_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/gengetopt)
set(GGO_OUTPUT_C_FILE ${GGO_OUTPUT_DIR}/cmdline.c)
set(GGO_OUTPUT_H_FILE ${GGO_OUTPUT_DIR}/cmdline.h)

# This command generates the C source and header files from the .ggo file.
add_custom_command(
    OUTPUT ${GGO_OUTPUT_C_FILE} ${GGO_OUTPUT_H_FILE}
    # Use the variable containing the path to the executable.
    COMMAND ${GENGETOPT_EXECUTABLE}
        --input=${GGO_INPUT_FILE}
        --output-dir=${GGO_OUTPUT_DIR}
        --c-extension=c
        --header-extension=h
    DEPENDS ${GGO_INPUT_FILE}
    COMMENT "Generating command line parser from ${GGO_INPUT_FILE}"
)

# This custom target drives the execution of the command above.
add_custom_target(
    GenerateCmdlineParser
    DEPENDS ${GGO_OUTPUT_C_FILE} ${GGO_OUTPUT_H_FILE}
)

# --- Wrapper Executable ---
# This target builds the simple chroot wrapper.
add_executable(wrapper src/wrapper/wrapper.c)

target_compile_options(wrapper PRIVATE -Wall -Wextra -Werror -O3)
# Set the output directory for the wrapper executable.
set_target_properties(wrapper PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)


# --- Main Application Executable ---
# This target builds the main application logic.
add_executable(main_app
    src/main_app/main.c
    src/main_app/config.c
    src/main_app/context.c
    src/main_app/server.c
    src/main_app/handlers.c
    src/main_app/auth.c
    src/main_app/drm.c
    src/main_app/bridge_wrapper.cpp
    ${GGO_OUTPUT_C_FILE}
    ${GGO_OUTPUT_H_FILE}
)

# The main_app target depends on the generated files.
add_dependencies(main_app GenerateCmdlineParser)

# Add include directories for the main application.
target_include_directories(main_app
    PUBLIC
        ${GGO_OUTPUT_DIR}
    PRIVATE
        src/main_app
)

# Set compiler standards and options for the main application.
set_target_properties(main_app PROPERTIES
    C_STANDARD 99
    CXX_STANDARD 11
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/rootfs/system/bin"
)

target_compile_definitions(main_app PRIVATE MyRelease)
target_compile_options(main_app PRIVATE -Wall -Wextra -Werror -O3)

# Allow undefined symbols from shared libraries
set_target_properties(main_app PROPERTIES LINK_FLAGS "-Wl,--allow-shlib-undefined")

# --- Library Linking ---
# Define the search path for the pre-built Android libraries.
set(ANDROID_LIB_PATH ${CMAKE_SOURCE_DIR}/rootfs/system/lib64)
link_directories(${ANDROID_LIB_PATH})

# Find the required shared libraries.
find_library(ANDROIDAPPMUSIC_LIB NAMES androidappmusic PATHS ${ANDROID_LIB_PATH} REQUIRED)
find_library(STORESERVICESCORE_LIB NAMES storeservicescore PATHS ${ANDROID_LIB_PATH} REQUIRED)
find_library(MEDIAPLATFORM_LIB NAMES mediaplatform PATHS ${ANDROID_LIB_PATH} REQUIRED)
find_library(CXX_SHARED_LIB NAMES c++_shared PATHS ${ANDROID_LIB_PATH} REQUIRED)

# Link the main application against the found libraries.
target_link_libraries(main_app
    ${CXX_SHARED_LIB}
    ${ANDROIDAPPMUSIC_LIB}
    ${STORESERVICESCORE_LIB}
    ${MEDIAPLATFORM_LIB}
)

# --- Installation (Optional) ---
# Defines where to install the final executables.
install(TARGETS wrapper
    RUNTIME DESTINATION bin
)
install(TARGETS main_app
    RUNTIME DESTINATION rootfs/system/bin
)
