cmake_minimum_required(VERSION 3.15)

if (NOT DEFINED CMAKE_INSTALL_PREFIX)
    set (CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Install to /usr/local" FORCE)
endif()

configure_file(install/si.nullob.pass-secrets.service.in si.nullob.pass-secrets.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/si.nullob.pass-secrets.service DESTINATION share/dbus-1/services)

configure_file(install/pass-secrets.service.in pass-secrets.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pass-secrets.service DESTINATION lib/systemd/user)

project(pass-secrets)

set(CMAKE_CXX_STANDARD 17)

include_directories("include")
add_subdirectory("nanoid_cpp")

find_package(sdbus-c++ REQUIRED)

add_executable(pass-secrets main.cpp impl/SecretService.cpp impl/SecretService.h impl/Session.cpp impl/Session.h interop/PassStore.cpp interop/PassStore.h interop/PassCollection.cpp interop/PassCollection.h interop/PassItem.cpp interop/PassItem.h interop/DocumentHelper.h interop/DocumentHelper.cpp impl/Collection.cpp impl/Collection.h impl/Item.cpp impl/Item.h impl/CollectionProxy.cpp impl/CollectionProxy.h impl/ItemProxy.cpp impl/ItemProxy.h impl/sessions/Plain.cpp impl/sessions/Plain.h impl/sessions/sessions.h)
target_link_libraries(pass-secrets PRIVATE SDBusCpp::sdbus-c++ nanoid)
target_compile_definitions(pass-secrets PRIVATE RAPIDJSON_HAS_STDSTRING)
install(TARGETS pass-secrets RUNTIME)