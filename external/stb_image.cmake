set(STB_IMAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/stb_image)

add_library(stb_image STATIC)

target_sources(stb_image
  PRIVATE
    ${STB_IMAGE_DIR}/stb_image.h
    ${STB_IMAGE_DIR}/stb_image_ext.h

    ${STB_IMAGE_DIR}/stb_image_ext.cpp
)

target_include_directories(stb_image PUBLIC ${STB_IMAGE_DIR})
