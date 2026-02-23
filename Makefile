######################################
# Project
######################################

PROJECT     := message_sdk
LIB_NAME    := lib$(PROJECT).so

######################################
# Compiler
######################################

CXX         := g++
AR          := ar
RM          := rm -rf

######################################
# Flags
######################################

CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -fPIC   #-fsanitize=address -g 

# Debug: make DEBUG=1
ifeq ($(DEBUG),1)
    CXXFLAGS := -std=c++17 -Wall -g -O0 -fPIC
endif


######################################
# Directories
######################################

ROOT_DIR    := $(shell pwd)

INC_DIRS := \
    -I$(ROOT_DIR)/include \
    -I$(ROOT_DIR)/src \
	-I$(ROOT_DIR)  

SRC_DIRS := \
    src/core \
    src/transport/zmq

BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/obj
BIN_DIR   := $(BUILD_DIR)/bin
LIB_DIR   := $(BUILD_DIR)/lib


######################################
# Libraries
######################################

LIBS := \
    -lzmqpp \
    -lzmq \
    -lpthread


######################################
# Sources
######################################

# 自动找 cpp
SRC := \
    $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))

OBJ := \
    $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))


######################################
# Examples
######################################

SERVER_SRC := examples/rep_server.cpp
CLIENT_SRC := examples/req_client.cpp
SERVER2_SRC := examples/rep_server2.cpp

SERVER_BIN := $(BIN_DIR)/rep_server
CLIENT_BIN := $(BIN_DIR)/req_client
SERVER2_BIN := $(BIN_DIR)/rep_server2

######################################
# Default
######################################

all: prepare $(LIB_DIR)/$(LIB_NAME) examples
lib: $(LIB_DIR)/$(LIB_NAME)

######################################
# Prepare dirs
######################################

prepare:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(OBJ_DIR)/src/core
	@mkdir -p $(OBJ_DIR)/src/transport/zmq


######################################
# Build library
######################################

$(LIB_DIR)/$(LIB_NAME): $(OBJ)
	$(CXX) -shared -o $@ $^ $(LIBS)


######################################
# Compile objects
######################################

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@


######################################
# Examples
######################################

examples: $(SERVER_BIN) $(CLIENT_BIN) $(SERVER2_BIN)


$(SERVER_BIN): $(SERVER_SRC) $(LIB_DIR)/$(LIB_NAME)
	$(CXX) $(CXXFLAGS) $(INC_DIRS) $< \
	-L$(LIB_DIR) -l$(PROJECT) $(LIBS) \
	-o $@

# 添加 server2 编译规则（在 $(CLIENT_BIN) 规则之后）
$(SERVER2_BIN): $(SERVER2_SRC) $(LIB_DIR)/$(LIB_NAME)    
	$(CXX) $(CXXFLAGS) $(INC_DIRS) $< \
	-L$(LIB_DIR) -l$(PROJECT) $(LIBS) \
	-o $@

$(CLIENT_BIN): $(CLIENT_SRC) $(LIB_DIR)/$(LIB_NAME)
	$(CXX) $(CXXFLAGS) $(INC_DIRS) $< \
	-L$(LIB_DIR) -l$(PROJECT) $(LIBS) \
	-o $@


######################################
# Clean
######################################

clean:
	$(RM) $(BUILD_DIR)


######################################
# Install (optional)
######################################

install:
	cp $(LIB_DIR)/$(LIB_NAME) /usr/local/lib
	cp -r include/message /usr/local/include/
	sudo ldconfig

