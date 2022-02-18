#
# Try to find the FreeImage library and include path.
# Once done this will define
#
# FREEIMAGE_FOUND
# FREEIMAGE_INCLUDE_PATH
# FREEIMAGE_LIBRARY
# 

IF (WIN32)
	FIND_PATH(FREEIMAGE_INCLUDE_PATH FreeImage.h
		${FREEIMAGE_ROOT_DIR}/include
		${FREEIMAGE_ROOT_DIR}
		DOC "The FreeImage header location")
	FIND_PATH(FREEIMAGE_DLL FreeImage.dll
		${FREEIMAGE_ROOT_DIR}/bin
		${FREEIMAGE_ROOT_DIR}
		DOC "The FreeImage dynamic link library location")
	FIND_LIBRARY(FREEIMAGE_LIBRARY
		NAMES FreeImage freeimage
		PATHS
		${FREEIMAGE_ROOT_DIR}/lib
		${FREEIMAGE_ROOT_DIR}
		DOC "The FreeImage library")
ELSE (WIN32)
	FIND_PATH(FREEIMAGE_INCLUDE_PATH FreeImage.h
        ${FREEIMAGE_ROOT_DIR}/include
		/usr/include
		/usr/local/include
		/sw/include
		/opt/local/include
		DOC "The directory where FreeImage.h resides")
	FIND_LIBRARY(FREEIMAGE_LIBRARY
		NAMES FreeImage freeimage
		PATHS
        ${FREEIMAGE_ROOT_DIR}/bin
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		DOC "The FreeImage library")
ENDIF (WIN32)

SET(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBRARY})

IF (FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY)
	SET( FREEIMAGE_FOUND TRUE CACHE BOOL "Set to TRUE if FreeImage is found, FALSE otherwise" FORCE )
	message(STATUS "Found FreeImage")
ELSE (FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY)
	SET( FREEIMAGE_FOUND FALSE CACHE BOOL "Set to TRUE if FreeImage is found, FALSE otherwise")
	message(WARNING "Failed to find FreeImage")
ENDIF (FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY)

MARK_AS_ADVANCED(
	FREEIMAGE_FOUND 
	FREEIMAGE_LIBRARY
	FREEIMAGE_LIBRARIES
	FREEIMAGE_INCLUDE_PATH
	FREEIMAGE_DLL
)
