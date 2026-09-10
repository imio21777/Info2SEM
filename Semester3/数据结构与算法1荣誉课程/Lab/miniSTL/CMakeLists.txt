cmake_minimum_required(VERSION 3.12)

if(NOT DEFINED PROJECT_NAME)
    set(SUBPROJECT OFF)

    project(miniSTL 
        VERSION 1.0.1
        DESCRIPTION "A header-only mini DS library"
    )
else()
    set(SUBPROJECT ON)
endif()

set(miniSTL_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCES_DIR ${miniSTL_DIR}/include/miniSTL)
set(CMAKE_INSTALL_PREFIX ${miniSTL_DIR}/lib)

add_library(miniSTL INTERFACE)

target_compile_features(miniSTL INTERFACE cxx_std_17)

target_include_directories(
    miniSTL
    INTERFACE 
    $<BUILD_INTERFACE:${SOURCES_DIR}/..>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


if (SUBPROJECT)

    add_library(miniSTL::miniSTL ALIAS miniSTL)

else()
    include(GNUInstallDirs)

    install(TARGETS miniSTL
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file("miniSTLConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
    )

    install(EXPORT ${PROJECT_NAME}_Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
    )

    install(FILES 
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
    )

    install(DIRECTORY ${SOURCES_DIR} DESTINATION include)

endif()
