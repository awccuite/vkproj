# Shader compilation CMakeLists.txt
# --------------------------------
# This CMake script defines a **compile_shaders** target which converts all
# GLSL/HLSL files in this directory to SPIR-V bytecode using *glslangValidator*.
# The resulting .spv files are written next to their source files and are **not**
# rebuilt by default – you must explicitly build the target (for example via
# `cmake --build . --target compile_shaders` or through the `build.sh -s` helper
# script).
#
#  * No renderer‐library dependency – recompiling shaders does not trigger a
#    full C++ rebuild.
#  * Handles MSYS2 / Windows path conversions automatically.
#  * Fails early with a helpful error message if *glslangValidator* cannot be
#    located.

cmake_minimum_required(VERSION 3.20)

# -------- Locate glslangValidator -------------------------------------------
# Name differs on Windows, so search for both.
if(WIN32)
    set(_GLSLANG_NAMES glslangValidator.exe glslangValidator)
else()
    set(_GLSLANG_NAMES glslangValidator)
endif()

find_program(GLSLANG_VALIDATOR NAMES ${_GLSLANG_NAMES}
    HINTS
        $ENV{VULKAN_SDK}/Bin $ENV{VULKAN_SDK}/bin
        $ENV{VK_SDK_PATH}/Bin $ENV{VK_SDK_PATH}/bin
        /usr/bin /usr/local/bin
        /c/VulkanSDK/*/Bin
        /c/msys64/mingw64/bin           # MSYS2 mingw packages
)

# Try PATH as the last resort (find_program might fail on some MSYS paths).
if(NOT GLSLANG_VALIDATOR)
    set(GLSLANG_VALIDATOR glslangValidator)
endif()

# Convert possible Windows style "C:/" path to MSYS style "/c/" for the shell
if(IS_ABSOLUTE "${GLSLANG_VALIDATOR}" AND WIN32 AND CMAKE_HOST_SYSTEM_NAME STREQUAL "MSYS")
    execute_process(COMMAND cygpath -u "${GLSLANG_VALIDATOR}" OUTPUT_VARIABLE GLSLANG_VALIDATOR_UNIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(GLSLANG_VALIDATOR_UNIX)
        set(GLSLANG_VALIDATOR "${GLSLANG_VALIDATOR_UNIX}")
    endif()
endif()

# Diagnostic output so users know which compiler is used
message(STATUS "glslangValidator executable: ${GLSLANG_VALIDATOR}")

# Abort if still not found (will also trigger if cygpath failed)
execute_process(COMMAND ${GLSLANG_VALIDATOR} --version RESULT_VARIABLE _GLSLANG_OK OUTPUT_QUIET ERROR_QUIET)
if(NOT _GLSLANG_OK EQUAL 0)
    message(FATAL_ERROR "glslangValidator not found or not executable – please install the Vulkan SDK and ensure glslangValidator is on PATH.")
endif()

# -------- Enumerate shaders --------------------------------------------------
file(GLOB SHADER_SOURCES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "*.vert" "*.frag" "*.geom" "*.comp" "*.tesc" "*.tese"
)

if(NOT SHADER_SOURCES)
    message(WARNING "No shader source files found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

add_custom_target(compile_shaders COMMENT "Compile all GLSL shaders to SPIR-V")

foreach(SHADER_FILE ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER_FILE} NAME)
    set(SPIRV_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME}.spv")

    add_custom_command(
        OUTPUT  ${SPIRV_FILE}
        COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE} -o ${SPIRV_FILE}
        DEPENDS ${SHADER_FILE}
        COMMENT "Compiling shader: ${SHADER_FILE} -> ${FILE_NAME}.spv"
        VERBATIM
    )

    add_custom_target(compile_${FILE_NAME} DEPENDS ${SPIRV_FILE})
    add_dependencies(compile_shaders compile_${FILE_NAME})
endforeach()

# End of shader CMakeLists.txt 