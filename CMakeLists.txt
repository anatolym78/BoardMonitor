cmake_minimum_required(VERSION 3.22)

project(BoardMonitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Boost settings
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_MULTITHREADED ON)
#add_definitions(-DBOOST_ALL_NO_LIB)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts Sql)
find_package(Boost 1.89.0 REQUIRED COMPONENTS filesystem json exception)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(
    APP_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    BoardStationApp.cpp
    BoardStationApp.h
    resources.qrc
    )

set(
    INTERFACE_SOURCES
    Interface/Session/SessionStackView.cpp
    Interface/Session/SessionStackView.h
    Interface/Session/SessionWorkspace.cpp
    Interface/Session/SessionWorkspace.h
    Interface/Session/SessionListView.cpp
    Interface/Session/SessionListView.h
    Interface/Session/SessionListDelegate.cpp
    Interface/Session/SessionListDelegate.h
    Interface/Telemetry/TelemetryDataView.cpp
    Interface/Telemetry/TelemetryDataView.h
    Interface/Charts/ChartsDashboardView.cpp
    Interface/Charts/ChartsDashboardView.h
    Interface/Charts/ParametersChartView.cpp
    Interface/Charts/ParametersChartView.h
    Interface/Player/PlaybackView.cpp
    Interface/Player/PlaybackView.h
    Interface/Uplink/UplinkEditorView.cpp
    Interface/Uplink/UplinkEditorView.h
    Interface/Tools/ConsoleView.cpp
    Interface/Tools/ConsoleView.h
    Interface/Tools/UplinkEditorDelegates.cpp
    Interface/Tools/UplinkEditorDelegates.h
    Interface/Tools/TelemetryDelegate.cpp
    Interface/Tools/TelemetryDelegate.h
    )

set(
    VIEWMODEL_SOURCES
    ViewModel/BoardParametersTreeModel.cpp
    ViewModel/BoardParametersTreeModel.h
    ViewModel/ChatViewGridModel.cpp
    ViewModel/ChatViewGridModel.h
    ViewModel/DataPlayer.cpp
    ViewModel/DataPlayer.h
    ViewModel/DriverDataPlayer.cpp
    ViewModel/DriverDataPlayer.h
    ViewModel/SessionPlayer.cpp
    ViewModel/SessionPlayer.h
    ViewModel/SessionsListModel.cpp
    ViewModel/SessionsListModel.h
    ViewModel/SessionSelectionModel.cpp
    ViewModel/SessionSelectionModel.h
    ViewModel/Session.cpp
    ViewModel/Session.h
    ViewModel/RecordedSession.cpp
    ViewModel/RecordedSession.h
    ViewModel/LiveSession.cpp
    ViewModel/LiveSession.h
    ViewModel/RecordedSessionsFactory.cpp
    ViewModel/RecordedSessionsFactory.h
    ViewModel/LiveSessionFactory.cpp
    ViewModel/LiveSessionFactory.h
    ViewModel/UplinkParametersTreeModel.cpp
    ViewModel/UplinkParametersTreeModel.h
    ViewModel/DebugViewModel.cpp
    ViewModel/DebugViewModel.h
    )

# set(
#     EMULATOR    _SOURCES
#     Model/Emulation/BoardDataEmulator.cpp
#     Model/Emulation/BoardDataEmulator.h
#     Model/Emulation/BoardDataGenerators/AccelerationGenerator.cpp
#     Model/Emulation/BoardDataGenerators/AccelerationGenerator.h
#     Model/Emulation/BoardDataGenerators/BatteryVoltageGenerator.cpp
#     Model/Emulation/BoardDataGenerators/BatteryVoltageGenerator.h
#     Model/Emulation/BoardDataGenerators/DesiredAngleRatesGenerator.cpp
#     Model/Emulation/BoardDataGenerators/DesiredAngleRatesGenerator.h
#     Model/Emulation/BoardDataGenerators/GpsLocationGenerator.cpp
#     Model/Emulation/BoardDataGenerators/GpsLocationGenerator.h
#     Model/Emulation/BoardDataGenerators/GroundSpeedGenerator.cpp
#     Model/Emulation/BoardDataGenerators/GroundSpeedGenerator.h
#     Model/Emulation/BoardDataGenerators/GyroAngleRatesGenerator.cpp
#     Model/Emulation/BoardDataGenerators/GyroAngleRatesGenerator.h
#     Model/Emulation/BoardDataGenerators/IsArmedGenerator.cpp
#     Model/Emulation/BoardDataGenerators/IsArmedGenerator.h
#     Model/Emulation/BoardDataGenerators/ParameterGenerator.cpp
#     Model/Emulation/BoardDataGenerators/ParameterGenerator.h
#     Model/Emulation/BoardDataGenerators/PositionAltitudeGenerator.cpp
#     Model/Emulation/BoardDataGenerators/PositionAltitudeGenerator.h
#     Model/Emulation/BoardDataGenerators/ThrottleAdjustmentValuesGenerator.cpp
#     Model/Emulation/BoardDataGenerators/ThrottleAdjustmentValuesGenerator.h
#     Model/Emulation/BoardDataGenerators/BoardDataJsonGenerator.cpp
#     Model/Emulation/BoardDataGenerators/BoardDataJsonGenerator.h    
#     )

set(
    MODEL_SOURCES
    Model/DriverAdapter.cpp
    Model/DriverAdapter.h
    Model/Parameters/Tree/ParameterTreeGroupItem.cpp
    Model/Parameters/Tree/ParameterTreeGroupItem.h
    Model/Parameters/Tree/ParameterTreeHistoryItem.cpp
    Model/Parameters/Tree/ParameterTreeHistoryItem.h
    Model/Parameters/Tree/ParameterTreeArrayItem.cpp
    Model/Parameters/Tree/ParameterTreeArrayItem.h
    Model/Parameters/Tree/ParameterTreeItem.cpp
    Model/Parameters/Tree/ParameterTreeItem.h
    Model/Parameters/Tree/ParameterTreeStorage.cpp
    Model/Parameters/Tree/ParameterTreeStorage.h
    Model/Parameters/Tree/ParameterTreeJsonParser.cpp
    Model/Parameters/Tree/ParameterTreeJsonParser.h
    Model/Parameters/AppConfigurationReader.cpp
    Model/Parameters/AppConfigurationReader.h
    Model/Parameters/BoardParameterSingle.h
    Model/Parameters/BoardParameterSingle.cpp
    Model/Parameters/BoardParameterValue.h
    Model/Parameters/BoardParameterValue.cpp
    Model/Parameters/BoardMessagesSqliteWriter.cpp
    Model/Parameters/BoardMessagesSqliteWriter.h
    Model/Parameters/BoardMessagesSqliteReader.cpp
    Model/Parameters/BoardMessagesSqliteReader.h
    Model/Parameters/WriteTreeWorker.cpp
    Model/Parameters/WriteTreeWorker.h
    Model/Parameters/ReadTreeWorker.cpp
    Model/Parameters/ReadTreeWorker.h
    Model/Parameters/Range.h
    Model/Parameters/Step.h
    )

set(
    SERVICE_SOURCES  
    Services/RCImitator/src/driver.hh
    Services/RCImitator/src/builder.hh
    )

set(
    PROJECT_SOURCES
    ${APP_SOURCES}
    ${INTERFACE_SOURCES}
    ${VIEWMODEL_SOURCES}
    ${MODEL_SOURCES}
    ${SERVICE_SOURCES}
    )

add_library(${PROJECT_NAME}_plugin SHARED
    Services/RCImitator/src/data/data.hh
    Services/RCImitator/src/data/generator.cc
    Services/RCImitator/src/data/generator.hh
    Services/RCImitator/src/data/series_generator.cc
    Services/RCImitator/src/data/series_generator.hh
    Services/RCImitator/src/data/parameter.cc
    Services/RCImitator/src/data/parameter.hh
    Services/RCImitator/src/data/parameters.hh
    Services/RCImitator/src/data/randomizer.hh
    Services/RCImitator/src/data/serializer.cc
    Services/RCImitator/src/data/serializer.hh
    Services/RCImitator/src/data/series.hh
    Services/RCImitator/src/data/series.cc

    Services/RCImitator/src/driver/builder.cc
    Services/RCImitator/src/driver/builder.hh
    Services/RCImitator/src/driver/imitator.cc
    Services/RCImitator/src/driver/imitator.hh

    Services/RCImitator/src/builder_wrapper.cc
    Services/RCImitator/src/builder_wrapper.hh
    Services/RCImitator/src/builder.hh
    Services/RCImitator/src/driver.hh
    )
# Include directories to the business logic lib
target_include_directories(${PROJECT_NAME}_plugin PRIVATE
    ${Boost_INCLUDE_DIRS}
)
# Link target libraries to the business logic lib
target_link_libraries(${PROJECT_NAME}_plugin PRIVATE
    Boost::filesystem
    Boost::json
    Boost::exception
    Qt5::Core
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PROJECT_SOURCES})

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_sources(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCES}
    )

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Boost::filesystem
    Boost::json
    Boost::exception
    Qt${QT_VERSION_MAJOR}::Widgets 
    Qt${QT_VERSION_MAJOR}::Charts 
    Qt${QT_VERSION_MAJOR}::Sql
)

set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(BoardMonitor)
endif()
