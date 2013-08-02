set(TINYXML_SRC 
	tinystr.cpp 
	tinyxml.cpp 
	tinyxmlerror.cpp
	tinyxmlparser.cpp)
	
set(TINYXML_HDR
	tinystr.h
	tinyxml.h)

add_library(tinyxml STATIC ${TINYXML_SRC} ${TINYXML_HDR})