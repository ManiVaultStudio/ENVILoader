cmake_minimum_required(VERSION 3.17)

# -----------------------------------------------------------------------------
# ENVI Loader Plugin
# -----------------------------------------------------------------------------
set(ENVIPROJECT "ENVILoader")
PROJECT(${ENVIPROJECT})

# -----------------------------------------------------------------------------
# CMake Options
# -----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /DWIN32 /EHsc /MP /permissive- /Zc:__cplusplus")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
find_package(Qt6 COMPONENTS Widgets WebEngineWidgets REQUIRED)

find_package(ManiVault COMPONENTS Core PointData ImageData CONFIG)

set(INSTALL_SUBPROJECTS OFF)
add_subdirectory(external/mio)

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
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

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
add_library(${ENVIPROJECT} SHARED ${SOURCES} ${AUX})

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
target_include_directories(${ENVIPROJECT} PRIVATE "${ManiVault_INCLUDE_DIR}")

# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
target_compile_features(${ENVIPROJECT} PRIVATE cxx_std_17)

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
target_link_libraries(${ENVIPROJECT} PRIVATE mio::mio)

target_link_libraries(${ENVIPROJECT} PRIVATE Qt6::Widgets)
target_link_libraries(${ENVIPROJECT} PRIVATE Qt6::WebEngineWidgets)

target_link_libraries(${ENVIPROJECT} PRIVATE ManiVault::Core)
target_link_libraries(${ENVIPROJECT} PRIVATE ManiVault::PointData)
target_link_libraries(${ENVIPROJECT} PRIVATE ManiVault::ImageData)

# -----------------------------------------------------------------------------
# Target installation
# -----------------------------------------------------------------------------
install(TARGETS ${ENVIPROJECT}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${ENVIPROJECT} POST_BUILD
	COMMAND "${CMAKE_COMMAND}"
	--install ${CMAKE_CURRENT_BINARY_DIR}
	--config $<CONFIGURATION>
	--prefix ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>
)

set_target_properties(${ENVIPROJECT}
    PROPERTIES
    FOLDER LoaderPlugins
)

# -----------------------------------------------------------------------------
# Miscellaneous
# -----------------------------------------------------------------------------
if(MSVC)
	# Automatically set the debug environment (command + working directory) for MSVC in debug mode
	set_property(TARGET ${ENVIPROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY  $<$<CONFIG:DEBUG>:${ManiVault_INSTALL_DIR}/debug>)
	set_property(TARGET ${ENVIPROJECT} PROPERTY VS_DEBUGGER_COMMAND $<$<CONFIG:DEBUG>:"${ManiVault_INSTALL_DIR}/debug/ManiVault Studio.exe">)

	# Automatically set the debug environment (command + working directory) for MSVC in release mode
	set_property(TARGET ${ENVIPROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY  $<$<CONFIG:RELEASE>:${ManiVault_INSTALL_DIR}/release>)
	set_property(TARGET ${ENVIPROJECT} PROPERTY VS_DEBUGGER_COMMAND $<$<CONFIG:RELEASE>:"${ManiVault_INSTALL_DIR}/release/ManiVault Studio.exe">)
endif()
