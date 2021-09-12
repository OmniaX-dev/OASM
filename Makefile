.DEFAULT_GOAL := all

# Version
#---------
OASM_MAJOR_VERSION ?= 0
OASM_MINOR_VERSION ?= 3

# Targets
#----------
T_LIB_NAME ?= oasm
T_OASM_LIB ?= lib$(T_LIB_NAME).so
T_VM_EXEC ?= oasm-vm
T_AS_EXEC ?= oasm-as
T_DBG_EXEC ?= oasm-dbg

# Directories
#--------------
BUILD_DIR ?= ./bin/linux
BUILD_LIB_DIR ?= shared
FRONTEND_BUILD_DIR ?= frontend
SRC_DIRS ?= ./src
BNR_FILE = build.nr
LIB_DIR = $(BUILD_DIR)/$(BUILD_LIB_DIR)

# Magic
#--------
SRCS := $(shell find $(SRC_DIRS) -name *.cpp)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Options
#----------
COMMON_SRC = $(SRC_DIRS)/common
VM_SRC = $(SRC_DIRS)/interpreter
AS_SRC = $(SRC_DIRS)/assembler
DBG_SRC = $(SRC_DIRS)/debugger
FRONTEND_SRC = $(SRC_DIRS)/frontend
DBG_FLAGS = -D__DEBUG__ -g
CXX = g++
APP_ARGS = -std=c++17
BNR = $(shell ./inc_bnr.sh $(BNR_FILE))
BNR_DEF = -D__BUILD_NUMBER__=$(BNR)
VERSION_DEF = -D__MAJOR_VER__=$(OASM_MAJOR_VERSION) -D__MINOR_VER__=$(OASM_MINOR_VERSION)
LDFLAGS = $(APP_ARGS) $(DBG_FLAGS) $(BNR_DEF) $(VERSION_DEF)
MAKE_LIB  = -shared -fPIC 
CPPFLAGS = -m32 $(INC_FLAGS) -MMD -MP $(LDFLAGS) -Wall
COMPILE_AS = -D__COMPILE_AS__
COMPILE_VM = -D__COMPILE_VM__
COMPILE_DBG = -D__COMPILE_DBG__

# obj files
$(BUILD_DIR)/$(BUILD_LIB_DIR)/$(T_OASM_LIB): $(OBJS)
	$(RM) $(BUILD_DIR)/$(BUILD_LIB_DIR)/$(T_OASM_LIB)
	$(MKDIR_P) $(BUILD_DIR)/$(BUILD_LIB_DIR)
	$(CXX) $(MAKE_LIB) -o $@ $(OBJS) $(CPPFLAGS)

$(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_VM_EXEC): $(OBJS)
	$(RM) $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_VM_EXEC)
	$(MKDIR_P) $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)
	$(CXX) $(COMPILE_VM) -o $@ $(OBJS) $(CPPFLAGS) -L$(LIB_DIR) -l$(T_LIB_NAME)

$(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_AS_EXEC): $(OBJS)
	$(RM) $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_AS_EXEC)
	$(MKDIR_P) $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)
	$(CXX) $(COMPILE_AS) -o $@ $(OBJS) $(CPPFLAGS) -L$(LIB_DIR) -l$(T_LIB_NAME)

$(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_DBG_EXEC): $(OBJS)
	$(RM) $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_DBG_EXEC)
	$(MKDIR_P) $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)
	$(CXX) $(COMPILE_DBG) -o $@ $(OBJS) $(CPPFLAGS) -L$(LIB_DIR) -l$(T_LIB_NAME)

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

_oasm-lib: $(BUILD_DIR)/$(BUILD_LIB_DIR)/$(T_OASM_LIB)
	$(CXX) $(MAKE_LIB) $(VM_SRC)/*.cpp $(AS_SRC)/*.cpp $(DBG_SRC)/*.cpp $(COMMON_SRC)/*.cpp -o $(BUILD_DIR)/$(BUILD_LIB_DIR)/$(T_OASM_LIB) $(CPPFLAGS)

_vm-frontend: $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_VM_EXEC)
	$(CXX) $(COMPILE_VM) $(FRONTEND_SRC)/*.cpp $(COMMON_SRC)/*.cpp -o $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_VM_EXEC) -L$(LIB_DIR) -l$(T_LIB_NAME) $(CPPFLAGS)

_as-frontend: $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_AS_EXEC)
	$(CXX) $(COMPILE_AS) $(FRONTEND_SRC)/*.cpp $(COMMON_SRC)/*.cpp -o $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_AS_EXEC) -L$(LIB_DIR) -l$(T_LIB_NAME) $(CPPFLAGS)

_dbg-frontend: $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_DBG_EXEC)
	$(CXX) $(COMPILE_DBG) $(FRONTEND_SRC)/*.cpp $(COMMON_SRC)/*.cpp -o $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_DBG_EXEC) -L$(LIB_DIR) -l$(T_LIB_NAME) $(CPPFLAGS)



run_args ?= 

run-vm:
	LD_LIBRARY_PATH='$(shell pwd)/$(BUILD_DIR)/$(BUILD_LIB_DIR)/' ./$(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_VM_EXEC) $(run_args)

run-as:
	LD_LIBRARY_PATH='$(shell pwd)/$(BUILD_DIR)/$(BUILD_LIB_DIR)/' ./$(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_AS_EXEC) $(run_args)

run-dbg:
	LD_LIBRARY_PATH='$(shell pwd)/$(BUILD_DIR)/$(BUILD_LIB_DIR)/' ./$(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_DBG_EXEC) $(run_args)



debug-as:
	LD_LIBRARY_PATH='$(shell pwd)/$(BUILD_DIR)/$(BUILD_LIB_DIR)/' gdb $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_AS_EXEC) $(run_args)

debug-vm:
	LD_LIBRARY_PATH='$(shell pwd)/$(BUILD_DIR)/$(BUILD_LIB_DIR)/' gdb $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_VM_EXEC) $(run_args)

debug-dbg:
	LD_LIBRARY_PATH='$(shell pwd)/$(BUILD_DIR)/$(BUILD_LIB_DIR)/' gdb $(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_DBG_EXEC) $(run_args)



run-vm-step_exec:
	LD_LIBRARY_PATH='$(shell pwd)/$(BUILD_DIR)/$(BUILD_LIB_DIR)/' ./$(BUILD_DIR)/$(FRONTEND_BUILD_DIR)/$(T_VM_EXEC) $(run_args) --step-execution



.PHONY: clean
.PHONY: __save_bnr

__save_bnr:
	echo $(BNR) > $(BNR_FILE)

clean:
	$(RM) -r $(BUILD_DIR)

all: _oasm-lib _as-frontend _vm-frontend _dbg-frontend __save_bnr
oasm-lib: _oasm-lib __save_bnr
vm-frontend: _vm-frontend __save_bnr
as-frontend: _as-frontend __save_bnr
dbg-frontend: _dbg-frontend __save_bnr

-include $(DEPS)

MKDIR_P ?= mkdir -p