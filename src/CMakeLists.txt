project("src")

# set(SOURCES_EX_10 "exercices/ex10/ex10.cpp" "camera.h" "shader.h" "exercices/object.h")

#These commands are there to specify the path to the folder containing the object and textures files as macro
#With these you can just use PATH_TO_OBJECTS and PATH_TO_TEXTURE in your c++ code and the compiler will replace it by the correct expression
add_compile_definitions(PATH_TO_OBJECTS="${CMAKE_CURRENT_SOURCE_DIR}/objects")
add_compile_definitions(PATH_TO_TEXTURE="${CMAKE_CURRENT_SOURCE_DIR}/textures")


# add_executable(${PROJECT_NAME}_ex10 ${SOURCES_EX_10})
# target_link_libraries(${PROJECT_NAME}_ex10 PUBLIC OpenGL::GL glfw glad)
