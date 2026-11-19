set(STB_IMAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/stb_image)

add_library(stb_image STATIC)

target_sources(stb_image
  PRIVATE
    ${STB_IMAGE_DIR}/stb_image.h
    ${STB_IMAGE_DIR}/stb_image_ext.h

    ${STB_IMAGE_DIR}/stb_image_ext.cpp
)

target_include_directories(stb_image PUBLIC ${STB_IMAGE_DIR})

# Определяем версию stb_image
set(STB_IMAGE_HEADER "${STB_IMAGE_DIR}/stb_image.h")
set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "${STB_IMAGE_HEADER}")

file(STRINGS "${STB_IMAGE_HEADER}" stb_first_line LIMIT_COUNT 1)
if(stb_first_line MATCHES "v([0-9]+\\.[0-9]+)")
    set(STB_IMAGE_VERSION "${CMAKE_MATCH_1}")
endif()
