add_library(
        ${PROJECT_NAME_LOWER}-core STATIC
        ConsoleLog.cpp
        Daemon.cpp
        DaemonConfig.cpp
        DBusConnectionHandle.cpp
        DBusEventLoop.cpp
        DBusMessageHandle.cpp
        EventLoop.cpp
        HandlerRegistration.cpp
        LEDs.cpp
        NullLog.cpp
        SyslogLog.cpp
        LightState.cpp
)