cmake_minimum_required(VERSION 3.17)

set(ENVIPROJECT "ENVILoader")

PROJECT(${ENVIPROJECT})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /DWIN32 /EHsc /MP /permissive- /Zc:__cplusplus")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# Check if the directory to the ManiVault installation has been provided
if(NOT DEFINED MV_INSTALL_DIR)
    set(MV_INSTALL_DIR "" CACHE PATH "Directory where ManiVault is installed")
    message(FATAL_ERROR "Please set MV_INSTALL_DIR to the directory where ManiVault is installed")
endif()
file(TO_CMAKE_PATH ${MV_INSTALL_DIR} MV_INSTALL_DIR)

find_package(Qt6 COMPONENTS Widgets WebEngineWidgets REQUIRED)

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

set(ACTIONS_SOURCES
    src/SubsamplingRatioAction.h
	src/SubsamplingRatioAction.cpp
)

set(AUX src/ENVILoaderPlugin.json)

set(SOURCES 
	${PLUGIN_SOURCES}
	${WIDGET_SOURCES}
	${ACTIONS_SOURCES}
)

source_group(Plugin FILES ${PLUGIN_SOURCES} ${AUX})
source_group(Widgets FILES ${WIDGET_SOURCES})
source_group(Actions FILES ${ACTIONS_SOURCES})
source_group(Aux FILES ${AUX})

add_library(${ENVIPROJECT} SHARED ${SOURCES} ${AUX})

target_include_directories(${ENVIPROJECT} PRIVATE "${MV_INSTALL_DIR}/$<CONFIGURATION>/include/")

target_compile_features(${ENVIPROJECT} PRIVATE cxx_std_17)

set(INSTALL_SUBPROJECTS OFF)
add_subdirectory(external/mio)
target_link_libraries(${ENVIPROJECT} PRIVATE mio::mio)

target_link_libraries(${ENVIPROJECT} PRIVATE Qt6::Widgets)
target_link_libraries(${ENVIPROJECT} PRIVATE Qt6::WebEngineWidgets)

set(HDPS_LINK_PATH "${MV_INSTALL_DIR}/$<CONFIGURATION>/lib")
set(PLUGIN_LINK_PATH "${MV_INSTALL_DIR}/$<CONFIGURATION>/$<IF:$<CXX_COMPILER_ID:MSVC>,lib,Plugins>")
set(HDPS_LINK_SUFFIX $<IF:$<CXX_COMPILER_ID:MSVC>,${CMAKE_LINK_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>)

set(HDPS_LINK_LIBRARY "${HDPS_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}HDPS_Public${HDPS_LINK_SUFFIX}")
set(POINTDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${HDPS_LINK_SUFFIX}") 
set(IMAGEDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}ImageData${HDPS_LINK_SUFFIX}") 

target_link_libraries(${ENVIPROJECT} PRIVATE "${HDPS_LINK_LIBRARY}")
target_link_libraries(${ENVIPROJECT} PRIVATE "${POINTDATA_LINK_LIBRARY}")
target_link_libraries(${ENVIPROJECT} PRIVATE "${IMAGEDATA_LINK_LIBRARY}")

install(TARGETS ${ENVIPROJECT}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${ENVIPROJECT} POST_BUILD
	COMMAND "${CMAKE_COMMAND}"
	--install ${CMAKE_CURRENT_BINARY_DIR}
	--config $<CONFIGURATION>
	--prefix ${MV_INSTALL_DIR}/$<CONFIGURATION>
)

if(MSVC)
	# Automatically set the debug environment (command + working directory) for MSVC in debug mode
	set_property(TARGET ${ENVIPROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY  $<$<CONFIG:DEBUG>:${MV_INSTALL_DIR}/debug>)
	set_property(TARGET ${ENVIPROJECT} PROPERTY VS_DEBUGGER_COMMAND $<$<CONFIG:DEBUG>:${MV_INSTALL_DIR}/debug/HDPS.exe>)

	# Automatically set the debug environment (command + working directory) for MSVC in release mode
	set_property(TARGET ${ENVIPROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY  $<$<CONFIG:RELEASE>:${MV_INSTALL_DIR}/release>)
	set_property(TARGET ${ENVIPROJECT} PROPERTY VS_DEBUGGER_COMMAND $<$<CONFIG:RELEASE>:${MV_INSTALL_DIR}/release/HDPS.exe>)
endif()
