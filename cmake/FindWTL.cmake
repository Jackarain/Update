include(FindPackageHandleStandardArgs)
find_path(WTL_ROOT_DIR 
	atlapp.h
	PATHS ${WTL_ROOT}
	DOC "WTL root directory")
	

if(WTL_ROOT_DIR)
	set(WTL_INCLUDE_DIRS "${WTL_ROOT_DIR}")
	message("found wtl at ${WTL_ROOT_DIR}")
else()
	set(WTL-NOTFOUND)
	if(WTL_FIND_REQUIRED)
		message(SEND_ERROR "cannot find WTL, Please install WTL 8.1 and set WTL_ROOT to the WTL include path")
	endif()
endif()