cmake_minimum_required(VERSION 3.15)

if (NOT DEFINED CMAKE_INSTALL_PREFIX)
    set (CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install to /usr" FORCE)
endif()

option(INSTALL_DBUS_SERVICE "Overwrite the gnome-keyring org.freedesktop.secrets.service file." ON)

if(INSTALL_DBUS_SERVICE)
    configure_file(install/org.freedesktop.secrets.service.in org.freedesktop.secrets.service)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.secrets.service DESTINATION share/dbus-1/services)
endif()

configure_file(install/pass-secrets.service.in pass-secrets.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pass-secrets.service DESTINATION lib/systemd/user)

project(pass-secrets)

set(CMAKE_CXX_STANDARD 17)

include_directories("include")
add_subdirectory("nanoid_cpp")

find_package(sdbus-c++ REQUIRED)

add_executable(pass-secrets main.cpp impl/SecretService.cpp impl/SecretService.h impl/Session.cpp impl/Session.h interop/PassStore.cpp interop/PassStore.h interop/PassCollection.cpp interop/PassCollection.h interop/PassItem.cpp interop/PassItem.h interop/DocumentHelper.h interop/DocumentHelper.cpp impl/Collection.cpp impl/Collection.h impl/Item.cpp impl/Item.h impl/CollectionProxy.cpp impl/CollectionProxy.h impl/ItemProxy.cpp impl/ItemProxy.h)
target_link_libraries(pass-secrets PRIVATE SDBusCpp::sdbus-c++ nanoid)
target_compile_definitions(pass-secrets PRIVATE RAPIDJSON_HAS_STDSTRING)
install(TARGETS pass-secrets RUNTIME)