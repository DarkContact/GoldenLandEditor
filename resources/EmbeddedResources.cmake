function(add_embedded_resources LIBRARY_NAME RESOURCE_DIR HEADER_NAME)
    # 1. Собираем утилиту, если она еще не создана
    if(NOT TARGET bin2c_tool)
        add_executable(bin2c_tool "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/bin2c.cpp")
    endif()

    # 2. Ищем файлы
    file(GLOB RES_FILES CONFIGURE_DEPENDS "${RESOURCE_DIR}/*")
    
    set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen_${LIBRARY_NAME}")
    file(MAKE_DIRECTORY "${GEN_DIR}")

    set(ALL_GEN_FILES "")
    set(HEADER_CONTENT "#pragma once\n#include <stdint.h>\n\n")

    foreach(RES_PATH ${RES_FILES})
        get_filename_component(RES_NAME "${RES_PATH}" NAME)
        # Очистка имени для C++ переменной
        string(REGEX REPLACE "[^a-zA-Z0-9]" "_" VAR_NAME "${RES_NAME}")
        string(TOLOWER "${VAR_NAME}" VAR_NAME)

        set(OUT_CPP "${GEN_DIR}/${VAR_NAME}.cpp")

        add_custom_command(
            OUTPUT "${OUT_CPP}"
            COMMAND bin2c_tool "${RES_PATH}" "${OUT_CPP}" "${VAR_NAME}"
            DEPENDS bin2c_tool "${RES_PATH}"
            COMMENT "Encoding ${RES_NAME}"
            VERBATIM
        )

        list(APPEND ALL_GEN_FILES "${OUT_CPP}")
        string(APPEND HEADER_CONTENT "extern const uint8_t ${VAR_NAME}[];\n")
        string(APPEND HEADER_CONTENT "extern const uint32_t ${VAR_NAME}_size;\n\n")
    endforeach()

    # 3. Генерируем общий заголовочный файл
    set(GEN_HEADER "${GEN_DIR}/${HEADER_NAME}")
    file(GENERATE OUTPUT "${GEN_HEADER}" CONTENT "${HEADER_CONTENT}")

    # 4. Создаем библиотеку
    add_library(${LIBRARY_NAME} STATIC ${ALL_GEN_FILES})
    target_include_directories(${LIBRARY_NAME} PUBLIC "${GEN_DIR}")
    
    # Чтобы заголовочный файл был виден как зависимость
    set_target_properties(${LIBRARY_NAME} PROPERTIES PUBLIC_HEADER "${GEN_HEADER}")
endfunction()
