# Sources definition
SRC_CXX_DIR = ./src

SRC_CXX = schd_trace.cpp \
		    schd_trace_map.cpp \
          schd_trace_map_simvision.cpp \
          schd_trace_map_gtkwave.cpp

# Specify variables for Boilermake
SOURCES := $(addprefix $(SRC_CXX_DIR)/,$(SRC_CXX))
