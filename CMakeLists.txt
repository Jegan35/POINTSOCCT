cmake_minimum_required(VERSION 3.16)
project(OCCT_Industrial VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic Meta-Object Compiler (MOC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ✅ 1. ADDED Qml and Quick3D to find_package
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL WebSockets Qml Quick3D)
find_package(OpenCASCADE REQUIRED)

# Tell the compiler where to find the Eigen3 math headers for KDL
include_directories(/usr/include/eigen3)

qt_add_executable(appOCCT
    main.cpp
    OcctWidget.h OcctWidget.cpp
    MainWindow.h MainWindow.cpp
    LeftPanel.h LeftPanel.cpp
    RightPanel.h RightPanel.cpp
    RightHeader.h RightHeader.cpp
    MenuTray.h MenuTray.cpp
    ClientBackend.h ClientBackend.cpp
    LineGeometry.h LineGeometry.cpp
    LoginDialog.h LoginDialog.cpp
)
# ✅ 2. ADDED Qt6::Qml and Qt6::Quick3D to target_link_libraries
target_link_libraries(appOCCT
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::WebSockets
    Qt6::Qml
    Qt6::Quick3D
    TKernel TKMath TKG2d TKG3d TKGeomBase TKGeomAlgo TKBRep TKTopAlgo
    TKMesh TKService TKOpenGl TKV3d TKXSBase TKSTEP TKSTEPBase TKSTEPAttr
    TKIGES TKShHealing TKSTL  TKPrim orocos-kdl
)

target_include_directories(appOCCT PRIVATE ${OpenCASCADE_INCLUDE_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/step" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")