CXX := g++
INC_DIR := ./src
CFLAGS := -g -std=c++20 -Wall
CPPFLAGS = $(addprefix -I ,$(INC_DIR)) 

BIN_DIR := ./bin
BUILD_DIR := ./build

.PHONY: 
ALL_TARGETS := 
# =========================================================
#						Help
# =========================================================
HELP_TARGET := help
ALL_TARGETS += $(HELP_TARGET)

.PHONY : $(HELP_TARGET)
$(HELP_TARGET):
	@echo "\t\033[0;37m Express Server project build makefile.\033[0m"
	@echo "To build any available target, type :make <target>"
	@echo "All available targets until now are listed below:"
	@echo $(foreach target,$(ALL_TARGETS),"\r* $(target)\n")


# ==========================================================
#						Utility			
# Targets: test-util, util
# ==========================================================
# TODO This target is deprecated and remaines to be updated.
UTIL_TEST_TARGET := test-util
UTIL_SRC_DIR := ./src/utility/

ALL_TARGETS += $(UTIL_TEST_TARGET)

UTIL_TEST_SRC := test/test.cc ipc/message.cc 
UTIL_TEST_SRCS := $(addprefix $(UTIL_SRC_DIR),$(UTIL_TEST_SRC))
UTIL_TEST_OBJS = $(UTIL_TEST_SRCS:%=$(BUILD_DIR)/%.o)

.PHONY += UTIL_TEST_TARGET
$(UTIL_TEST_TARGET) : $(UTIL_TEST_OBJS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(UTIL_TEST_OBJS) -o $(BIN_DIR)/$@
	@echo "\033[0;37mGenerated utility test program $(BIN_DIR)/$@\033[0m"

# ==========================================================
#						Utility-IPC			
# Targets: test-util-ipc, ipc
# ==========================================================
UTIL_IPC_TEST_TARGET := test-util-ipc
UTIL_IPC_SRC_DIR := ./src/utility/ipc/

ALL_TARGETS += $(UTIL_IPC_TEST_TARGET)

UTIL_IPC_TEST_SRC := test/test.cc message_queue.cc tcp_client.cc tcp_socket.cc \
					tcp_server.cc socket.cc
UTIL_IPC_TEST_SRCS := $(addprefix $(UTIL_IPC_SRC_DIR),$(UTIL_IPC_TEST_SRC))
UTIL_IPC_TEST_OBJS = $(UTIL_IPC_TEST_SRCS:%=$(BUILD_DIR)/%.o)

.PHONY += UTIL_IPC_TEST_TARGET
$(UTIL_IPC_TEST_TARGET) : $(UTIL_IPC_TEST_OBJS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(UTIL_IPC_TEST_OBJS) -o $(BIN_DIR)/$@
	@echo "\033[0;37mGenerated utility test program $(BIN_DIR)/$@\033[0m"

# =========================================================
#						Log
# Targets: test-log, log
# =========================================================
LOG_TEST_TARGET := test-log
LOG_TEST_CLIENT_TARGET := test-log-client

ALL_TARGETS += $(LOG_TEST_TARGET)
ALL_TARGETS += $(LOG_TEST_CLIENT_TARGET)
LOG_SRC_DIR := ./src/log/

LOG_RELY_ON_UTIL = ipc/message.cc ipc/message_queue.cc ipc/signal.cc

LOG_TEST_SRC_FILES := test/server.cc
LOG_TEST_SRCS = $(addprefix $(LOG_SRC_DIR),$(LOG_TEST_SRC_FILES)) \
				$(addprefix $(UTIL_SRC_DIR),$(LOG_RELY_ON_UTIL))

LOG_TEST_CLIENT_SRC_FILES := test/client.cc
LOG_TEST_CLIENT_SRCS = $(addprefix $(LOG_SRC_DIR),$(LOG_TEST_CLIENT_SRC_FILES)) \
						$(addprefix $(UTIL_SRC_DIR),$(LOG_RELY_ON_UTIL))

LOG_TEST_OBJS := $(LOG_TEST_SRCS:%=$(BUILD_DIR)/%.o)
LOG_TEST_CLIENT_OBJS := $(LOG_TEST_CLIENT_SRCS:%=$(BUILD_DIR)/%.o)

.PHONY += $(LOG_TEST_TARGET)
$(LOG_TEST_TARGET) : $(LOG_TEST_OBJS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(LOG_TEST_OBJS) -o $(BIN_DIR)/$@
	@echo "\033[0;37mGenerated log test program $(BIN_DIR)/$@\033[0m"

.PHONY += $(LOG_TEST_CLIENT_TARGET)
$(LOG_TEST_CLIENT_TARGET): $(LOG_TEST_CLIENT_OBJS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(LOG_TEST_CLIENT_OBJS) -o $(BIN_DIR)/$@
	@echo "\033[0;37mGenerated log test program $(BIN_DIR)/$@\033[0m"


# ==========================================================
#						Data Base 
# Targets: test-db, db
# ==========================================================
DB_TEST_TARGET := test-db 
ALL_TARGETS += $(DB_TEST_TARGET)

DB_SRC_DIR := ./src/database/

DB_TEST_SRC := database.cc datalist.cc test/test.cc

DB_TEST_SRCS := ./src/utility/data/rawdata.cc $(addprefix $(DB_SRC_DIR),$(DB_TEST_SRC))
DB_TEST_OBJS = $(DB_TEST_SRCS:%=$(BUILD_DIR)/%.o)

.PHONY += $(DB_TEST_TARGET)
$(DB_TEST_TARGET): $(DB_TEST_OBJS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(DB_TEST_OBJS) -o $(BIN_DIR)/$@
	@echo "\033[0;37mGenerated data base test program $(BIN_DIR)/$@\033[0m"


# ==========================================================
#				Generic Targets
# ==========================================================

$(BUILD_DIR)/%.o: %
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

CLEAN_ALL_TARGET := clean-all

ALL_TARGETS += $(CLEAN_ALL_TARGET)
.PHONY += $(CLEAN_ALL_TARGET)

$(CLEAN_ALL_TARGET):
	rm -rf $(BUILD_DIR)