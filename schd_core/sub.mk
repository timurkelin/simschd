# Sources definition
SRC_CXX_DIR = ./src

SRC_CXX = schd_sig_ptree.cpp  \
			 schd_ptree_xbar.cpp \
			 schd_cres.cpp \
			 schd_exec.cpp \
			 schd_core.cpp \
			 schd_planner.cpp

# Specify variables for Boilermake
SOURCES := $(addprefix $(SRC_CXX_DIR)/,$(SRC_CXX))
