cmake_minimum_required(VERSION 3.17)

set(PROJECT "ENVILoader")

PROJECT(${PROJECT})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /DWIN32 /EHsc /MP")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# Normalize the incoming install path
# Get the env path and allow the user to change it
set(INSTALL_DIR_PATH $ENV{HDPS_INSTALL_DIR} CACHE PATH "The HDPS Install Directoy")
# Normalize the incoming install path
file(TO_CMAKE_PATH ${INSTALL_DIR_PATH} INSTALL_DIR)

find_package(Qt5 COMPONENTS Widgets WebEngineWidgets REQUIRED)

set(PLUGIN_SOURCES
    src/ENVILoaderPlugin.h
    src/ENVILoaderPlugin.cpp
    src/ENVILoader.h
    src/ENVILoader.cpp
    src/ENVILoaderModel.h
    src/ENVILoaderModel.cpp
)

set(WIDGET_SOURCES
    src/SubsampleSettingsWidget.h
	src/SubsampleSettingsWidget.cpp
)

set(AUX src/ENVILoaderPlugin.json)

file(GLOB UIS src/*.ui)

set(SOURCES 
	${PLUGIN_SOURCES}
	${WIDGET_SOURCES}
)

source_group( Plugin FILES ${PLUGIN_SOURCES} ${AUX})
source_group(Widgets FILES ${WIDGET_SOURCES})
source_group(UI FILES ${UIS})
source_group(Aux FILES ${AUX})

include_directories("${INSTALL_DIR}/$<CONFIGURATION>/include/")
#include_directories("external/mio/single_include")

QT5_WRAP_UI(UI_HEADERS ${UIS})

add_library(${PROJECT} SHARED ${SOURCES} ${UIS} ${AUX})

set_property(TARGET ${PROJECT} PROPERTY CXX_STANDARD 17)

add_subdirectory(external/mio)
target_link_libraries(${PROJECT} mio::mio)

target_link_libraries(${PROJECT} Qt5::Widgets)
target_link_libraries(${PROJECT} Qt5::WebEngineWidgets)

if(MSVC)
    set(LIB_SUFFIX "${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
    set(LIB_SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif()

target_link_libraries(${PROJECT} "${INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}HDPS_Public${LIB_SUFFIX}")
target_link_libraries(${PROJECT} "${INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${LIB_SUFFIX}")
target_link_libraries(${PROJECT} "${INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}ImageData${LIB_SUFFIX}")

install(TARGETS ${PROJECT}
   RUNTIME DESTINATION Plugins COMPONENT SHAREDLIB
)

add_custom_command(TARGET ${PROJECT} POST_BUILD
	COMMAND "${CMAKE_COMMAND}"
	--install ${CMAKE_BINARY_DIR}
	--config $<CONFIGURATION>
	--prefix ${INSTALL_DIR}/$<CONFIGURATION>
)

# Automatically set the debug environment (command + working directory) for MSVC in debug mode
set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY  $<$<CONFIG:DEBUG>:${INSTALL_DIR}/debug>)
set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_COMMAND $<$<CONFIG:DEBUG>:${INSTALL_DIR}/debug/HDPS.exe>)

# Automatically set the debug environment (command + working directory) for MSVC in release mode
set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY  $<$<CONFIG:RELEASE>:${INSTALL_DIR}/release>)
set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_COMMAND $<$<CONFIG:RELEASE>:${INSTALL_DIR}/release/HDPS.exe>)