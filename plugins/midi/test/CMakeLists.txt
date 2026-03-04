cmake_minimum_required(VERSION 3.16)
project(midi_test LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)

add_executable(midi_test WIN32 MACOSX_BUNDLE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    ../src/common/midiprotocol.cpp ../src/common/midiprotocol.h
    ../src/common/mackiecontrolprotocol.cpp ../src/common/mackiecontrolprotocol.h
    ../src/common/mackiecontrolhandler.cpp ../src/common/mackiecontrolhandler.h
    ../src/common/mididevice.cpp ../src/common/mididevice.h
    ../src/common/midioutputdevice.cpp ../src/common/midioutputdevice.h
    ../src/common/miditemplate.h
    miditemplate_stub.cpp
    midi_test.cpp midi_test.h
)
target_include_directories(midi_test PRIVATE
    ../../interfaces
    ../src/common
)

target_link_libraries(midi_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
