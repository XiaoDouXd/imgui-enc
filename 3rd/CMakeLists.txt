cmake_minimum_required(VERSION 3.25)
project(3rd)

set(SDL_SHARED false)
set(SDL_STATIC true)
set(SDL_TEST false)
set(SDL_TEST_ENABLED_BY_DEFAULT OFF)

add_subdirectory(imgui)
add_subdirectory(SDL3)
add_subdirectory(glm)