# Sources definition
SRC_CXX_DIR = ./src

SRC_CXX = schd_time.cpp

# Specify variables for Boilermake
SOURCES := $(addprefix $(SRC_CXX_DIR)/,$(SRC_CXX))
