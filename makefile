# Shell
SHELL := /bin/bash

# Compiler flags
CXXFLAGS = -c -Wall -std=gnu++11 -fexceptions -fPIC -pedantic

# Check for CCACHE
CCACHE_EXISTS := $(shell command -v ccache)
ifneq ($(CCACHE_EXISTS),)
  ifeq ($(filter ccache,$(CXX)),)
    CXX := ccache $(CXX)
  endif

  ifeq ($(filter ccache,$(GCC)),)
    GCC := ccache $(GCC)
  endif
endif

MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
PROJ_ROOT_DIR := $(patsubst %/,%,$(dir $(MAKEFILE_PATH)))

# Directories with project components
CMP_DIR = schd_common  \
          schd_report  \
          schd_systemc \
          schd_pref    \
          schd_trace


# Include folders
INC_LIB_DIR := -I$(SYSTEMC_HOME)/include \
			   -I$(SYSTEMC_HOME)/include/sysc/utils \
			   -I$(SYSTEMC_HOME)/include/sysc/kernel \
			   -I$(BOOST_HOME)/include
             
INC_CMP_DIR := $(addsuffix /include,$(addprefix -I$(PROJ_ROOT_DIR)/,$(CMP_DIR)))

INC_CXX_DIR := $(INC_LIB_DIR) $(INC_CMP_DIR)

ifneq ($(MAKECMDGOALS),link)
  ifndef CONFIG
    CONFIG = Release
  endif
endif

ifeq "$(CONFIG)" "Release"
  OBJ_DIR   = $(PROJ_ROOT_DIR)/build/Release/obj
  DEP_DIR   = $(PROJ_ROOT_DIR)/build/Release/dep
  EXE_DIR   = $(PROJ_ROOT_DIR)/build/Release/out
  CXXFLAGS += -O -DNDEBUG -fomit-frame-pointer
  LDFLAGS   = 
  LDLIBS    =  
endif

ifeq "$(CONFIG)" "Profile"
  OBJ_DIR   = $(PROJ_ROOT_DIR)/build/Profile/obj
  DEP_DIR   = $(PROJ_ROOT_DIR)/build/Profile/dep
  EXE_DIR   = $(PROJ_ROOT_DIR)/build/Profile/out
  CXXFLAGS += -pg -O -DNDEBUG -fno-omit-frame-pointer
  LDFLAGS   = -pg
  LDLIBS    =
endif

ifeq "$(CONFIG)" "Debug"
  OBJ_DIR   = $(PROJ_ROOT_DIR)/build/Debug/obj
  DEP_DIR   = $(PROJ_ROOT_DIR)/build/Debug/dep
  EXE_DIR   = $(PROJ_ROOT_DIR)/build/Debug/out
  CXXFLAGS += -gdwarf-2 -O0 -fno-omit-frame-pointer
  LDFLAGS   = -gdwarf-2
  LDLIBS    =  
endif

# Filenames
EXE_OUT = stschd
OBJ_LIST_FILE := $(EXE_DIR)/$(EXE_OUT).list

# Export variables
ifneq ($(MAKECMDGOALS),link)
  export CXX
  export CXXFLAGS
  export INC_CXX_DIR
  export PROJ_ROOT_DIR
  export OBJ_DIR
  export DEP_DIR
  export OBJ_LIST_FILE
  export LDFLAGS 
  export LDLIBS 
  export EXE_DIR
  export EXE_OUT
endif

.PHONY: clean all

## Default rule
all:
	@mkdir -p $(EXE_DIR)
	@rm -f $(OBJ_LIST_FILE)
	@for dir in $(CMP_DIR) ; do \
		$(MAKE) -C $$dir obj ; \
	done
	@$(MAKE) -C . link

## Clean Rule
clean:
	@rm -f $(EXE_DIR)/*
	@rm -f $(OBJ_DIR)/*
	@rm -f $(DEP_DIR)/*

ifeq ($(MAKECMDGOALS),link)
# Linker flags and library directories
LDFLAGS += -w -Wl,--no-undefined \
           -L$(LD_LIBRARY_PATH) \
           -L$(BOOST_HOME)/lib \
           -L$(SYSTEMC_HOME)/lib-linux64 

# Libraries to link against
LDLIBS += -lstdc++ \
          -lm \
          -lpthread \
          -lsystemc \
		  -lboost_system \
		  -lboost_regex \
		  -lmatio
		  	
OBJ_LIST := $(shell cat $(OBJ_LIST_FILE))

link: $(EXE_DIR)/$(EXE_OUT)
	@true
	
$(EXE_DIR)/$(EXE_OUT): $(OBJ_LIST)
	@echo "LD  $(EXE_OUT)"
	@$(GCC) $(LDFLAGS) $(OBJ_LIST) -o $(EXE_DIR)/$(EXE_OUT) $(LDLIBS)
	
endif
