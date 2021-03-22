cmake_minimum_required(VERSION 3.19)
project(pass-secrets)

set(CMAKE_CXX_STANDARD 17)

include_directories("include")
add_subdirectory("nanoid_cpp")

find_package(sdbus-c++ REQUIRED)

add_executable(pass-secrets main.cpp impl/SecretService.cpp impl/SecretService.h impl/Session.cpp impl/Session.h interop/PassStore.cpp interop/PassStore.h interop/PassCollection.cpp interop/PassCollection.h interop/PassItem.cpp interop/PassItem.h interop/DocumentHelper.h interop/DocumentHelper.cpp impl/Collection.cpp impl/Collection.h impl/Item.cpp impl/Item.h)
target_link_libraries(pass-secrets PRIVATE SDBusCpp::sdbus-c++ nanoid)
target_compile_definitions(pass-secrets PRIVATE RAPIDJSON_HAS_STDSTRING)