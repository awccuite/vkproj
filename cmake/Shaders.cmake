function(add_spirv_target)
    set(options OPTIMIZE)
    set(oneValueArgs TARGET OUTPUT_DIR)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(SP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT SP_TARGET)
        message(FATAL_ERROR "add_spirv_target: TARGET required")
    endif()
    if(NOT SP_SOURCES)
        message(FATAL_ERROR "add_spirv_target: SOURCES required")
    endif()
    if(NOT SP_OUTPUT_DIR)
        set(SP_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
    endif()

    find_program(GLSLC_EXECUTABLE glslc HINTS ENV VULKAN_SDK PATH_SUFFIXES Bin bin)
    if(NOT GLSLC_EXECUTABLE)
        message(FATAL_ERROR "glslc not found. Ensure Vulkan SDK or shaderc is installed and glslc is on PATH.")
    endif()

    set(outputs "")
    foreach(src ${SP_SOURCES})
        get_filename_component(name ${src} NAME)
        set(out "${SP_OUTPUT_DIR}/${name}.spv")
        add_custom_command(
            OUTPUT ${out}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${SP_OUTPUT_DIR}"
            COMMAND ${GLSLC_EXECUTABLE} $<$<BOOL:${SP_OPTIMIZE}>:-O> -o ${out} ${src}
            DEPENDS ${src}
            COMMENT "Compiling shader ${name}" VERBATIM)
        list(APPEND outputs ${out})
    endforeach()

    add_custom_target(${SP_TARGET} DEPENDS ${outputs})
    set_property(TARGET ${SP_TARGET} PROPERTY FOLDER "Shaders")
endfunction()