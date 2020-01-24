cmake_minimum_required( VERSION 3.16 )
project( "simschd"
   LANGUAGES CXX )

find_program( CCACHE_FOUND ccache )
if( CCACHE_FOUND )
	set( CMAKE_CXX_COMPILER_LAUNCHER ccache )
endif()

# Directories with project components
set(
   CMP_DIRS
		"schd_common"  
		"schd_core"    
		"schd_dump"    
		"schd_report"  
		"schd_systemc" 
		"schd_trace"   
		"schd_pref"    
		"schd_time"
)

# Include directories
message( "Boost   home: $ENV{BOOST_HOME}" )
message( "SystemC home: $ENV{SYSTEMC_HOME}" )
message( "matio   home: $ENV{MATIO_HOME}" )

# Initialize lists of source files and include directories
set( PRIVATE_CXX        "" )
set( PRIVATE_INC_DIR    "" )

# Parse directories
foreach( SRC_DIR ${CMP_DIRS} )
   include(
      "${CMAKE_CURRENT_LIST_DIR}/${SRC_DIR}/CMakeLists.txt"
   )
endforeach()

add_executable( "${PROJECT_NAME}" )

target_sources( "${PROJECT_NAME}"
   PRIVATE
      "${PRIVATE_CXX}"
)

target_include_directories( "${PROJECT_NAME}"
   PRIVATE
      "$ENV{SYSTEMC_HOME}/include"
      "$ENV{SYSTEMC_HOME}/include/sysc/utils"
      "$ENV{SYSTEMC_HOME}/include/sysc/kernel"
      "$ENV{BOOST_HOME}/include"
      "$ENV{MATIO_HOME}/include"
      "${PRIVATE_INC_DIR}"
)

target_link_directories( "${PROJECT_NAME}"
   PRIVATE
      "$ENV{LD_LIBRARY_PATH}"
      "$ENV{BOOST_HOME}/lib"
      "$ENV{SYSTEMC_HOME}/lib-linux64"
      "$ENV{MATIO_HOME}/lib"
)

target_link_libraries( "${PROJECT_NAME}"
   PRIVATE
      "stdc++"
      "m"
      "pthread"
      "systemc"
      "boost_system"
      "boost_regex"
      "matio"
)

set_target_properties( "${PROJECT_NAME}"
   PROPERTIES
      CXX_STANDARD 11
      CXX_STANDARD_REQUIRED YES
      CXX_EXTENSIONS YES
)

if( CMAKE_COMPILER_IS_GNUCXX )
# Compiler 
	target_compile_options( "${PROJECT_NAME}" 
		PRIVATE 
			-Wall -Wpedantic -fexceptions -fPIC 
	)
	
	set( CMAKE_CXX_FLAGS_DEBUG 
		"${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -O0"
	)
	
	set( CMAKE_CXX_FLAGS_RELEASE 
		"${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer"
	)
	
# Linker	
	target_link_options( "${PROJECT_NAME}" 
		PRIVATE 
			-w -Wl,--no-undefined
	)
endif()

