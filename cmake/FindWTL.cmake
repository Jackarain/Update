include(FindPackageHandleStandardArgs)
find_path(WTL_ROOT_DIR 
	atlapp.h
	PATHS ${WTL_ROOT}
	DOC "WTL root directory")
	

if(WTL_ROOT_DIR)
	set(WTL_INCLUDE_DIRS "${WTL_ROOT_DIR}/Include")
	message("found wtl at ${WTL_ROOT_DIR}")
else()
	set(WTL-NOTFOUND)
	message("cannot found WTL")
endif()