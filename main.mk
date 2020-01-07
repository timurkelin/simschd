MAKEFILE_PATH = $(abspath $(lastword $(MAKEFILE_LIST)))
PROJ_ROOT_DIR = $(patsubst %/,%,$(dir $(MAKEFILE_PATH)))

# Directories with project components
CMP_DIRS = schd_common  \
           schd_core    \
		     schd_dump    \
           schd_report  \
           schd_systemc \
           schd_trace   \
           schd_pref    \
		     schd_time

# Include folders
INC_LIB_DIRS = $(SYSTEMC_HOME)/include \
			      $(SYSTEMC_HOME)/include/sysc/utils \
			      $(SYSTEMC_HOME)/include/sysc/kernel \
			      $(BOOST_HOME)/include \
			      $(MATIO_HOME)/include

INC_CMP_DIRS = $(addsuffix /include,$(addprefix $(PROJ_ROOT_DIR)/,$(CMP_DIRS)))

# Compiler flags
CXXFLAGS = -c -Wall -std=gnu++11 -fexceptions -fPIC -pedantic

ifndef CONFIG
  CONFIG = Release
endif
  
ifeq "$(CONFIG)" "Release"
  OBJ_DIR   = $(PROJ_ROOT_DIR)/build/Release/obj
  EXE_DIR   = $(PROJ_ROOT_DIR)/build/Release/out
  CXXFLAGS += -O -DNDEBUG -fomit-frame-pointer
  LDFLAGS   = 
  LDLIBS    =  
endif

ifeq "$(CONFIG)" "Profile"
  OBJ_DIR   = $(PROJ_ROOT_DIR)/build/Profile/obj
  EXE_DIR   = $(PROJ_ROOT_DIR)/build/Profile/out
  CXXFLAGS += -pg -O -DNDEBUG -fno-omit-frame-pointer
  LDFLAGS   = -pg
  LDLIBS    =
endif

ifeq "$(CONFIG)" "Debug"
  OBJ_DIR   = $(PROJ_ROOT_DIR)/build/Debug/obj
  EXE_DIR   = $(PROJ_ROOT_DIR)/build/Debug/out
  CXXFLAGS += -gdwarf-2 -O0 -fno-omit-frame-pointer
  LDFLAGS   = -gdwarf-2
  LDLIBS    =  
endif

LDFLAGS += -w -Wl,--no-undefined \
           -L$(LD_LIBRARY_PATH) \
           -L$(BOOST_HOME)/lib \
           -L$(SYSTEMC_HOME)/lib-linux64 \
           -L$(MATIO_HOME)/lib

# Libraries to link against
LDLIBS += -lstdc++ \
          -lm \
          -lpthread \
          -lsystemc \
		    -lboost_system \
		    -lboost_regex \
		    -lmatio

# Specify variables for Boilermake
TARGET := simschd
BUILD_DIR := ${OBJ_DIR}
TARGET_DIR := ${EXE_DIR}
TGT_CXXFLAGS := ${CXXFLAGS}
TGT_LDFLAGS := ${LDFLAGS}
TGT_LDLIBS  := ${LDLIBS}
INCDIRS := ${INC_LIB_DIRS} ${INC_CMP_DIRS}
SUBMAKEFILES := $(addsuffix /sub.mk,$(addprefix $(PROJ_ROOT_DIR)/,$(CMP_DIRS)))

# Exports
EXE_OUT := ${TARGET}
export EXE_OUT

# Check for CCACHE
CCACHE_EXISTS := $(shell command -v ccache)
ifneq ($(CCACHE_EXISTS),)
  ifeq ($(filter ccache,$(CXX)),)
    CXX := ccache $(CXX)
    export CXX
  endif

  ifeq ($(filter ccache,$(GCC)),)
    GCC := ccache $(GCC)
    export GCC
  endif
endif
