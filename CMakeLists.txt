cmake_minimum_required(VERSION 3.22)

project(
  GroupChatServer
  VERSION 1.0
  DESCRIPTION "A11"
  LANGUAGES C)
#add_compile_options(-pthread -fsanitize=thread)
#add_link_options(-pthread -fsanitize=thread)
#add_compile_options(-pthread -g -fsanitize=address)
#add_link_options(-pthread -g -fsanitize=address)
include_directories(include)
add_executable(server src/server.c)
add_executable(client src/client.c)

