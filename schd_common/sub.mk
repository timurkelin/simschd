# Sources definition
SRC_CXX_DIR = ./src

SRC_CXX = schd_conv_ptree.cpp \
          schd_common.cpp

# build and link sc_main only for the top level of the project          
ifeq "${EXE_OUT}" "simschd"
	SRC_CXX += schd_main.cpp
endif

# Specify variables for Boilermake
SOURCES := $(addprefix $(SRC_CXX_DIR)/,$(SRC_CXX))
