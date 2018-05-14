configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${GIO_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME_LOWER}
        main.cpp
        )

target_link_libraries(${PROJECT_NAME_LOWER}
        ${PROJECT_NAME_LOWER}-core
        ${GIO_LIBRARIES}
        )

install(TARGETS ${PROJECT_NAME_LOWER} RUNTIME DESTINATION sbin)

install(FILES
        org.thinkglobally.GemianLEDs.conf
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/dbus-1/system.d
        )

add_subdirectory(core)
