include_directories(
        ${GIO_INCLUDE_DIRS}
)

link_directories (${GIO_LIBRARIES})

add_executable(leds_tests
        TestMain.cpp
        TestLEDs.cpp
        FakeLog.cpp
        DBusBus.cpp
        DBusClient.cpp
        RunCommand.cpp
        RunDaemon.cpp
        TestBase.cpp TestBase.h)

target_link_libraries(leds_tests
        ${PROJECT_NAME_LOWER}-core
        ${GIO_LIBRARIES}
        )
