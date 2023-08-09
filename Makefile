CXX := g++
INC_DIR := /home/hoo/project/express/src
CFLAGS := -g -std=c++20 
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
UTIL_TEST_TARGET := test-util
UTIL_SRC_DIR := ./src/utility/

ALL_TARGETS += $(UTIL_TEST_TARGET)

UTIL_TEST_SRC := test/test.cc ipc/message.cc ipc/message_queue.cc ipc/signal.cc
UTIL_TEST_SRCS := $(addprefix $(UTIL_SRC_DIR),$(UTIL_TEST_SRC))
UTIL_TEST_OBJS = $(UTIL_TEST_SRCS:%=$(BUILD_DIR)/%.o)

.PHONY += UTIL_TEST_TARGET
$(UTIL_TEST_TARGET) : $(UTIL_TEST_OBJS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(UTIL_TEST_OBJS) -o $(BIN_DIR)/$@
	@echo "\033[0;37mGenerated utility test program $(BIN_DIR)/$@\033[0m"



# =========================================================
#						Log
# Targets: test-log, log
# =========================================================
LOG_TEST_TARGET := test-log
LOG_SRC_DIR := ./src/log
ALL_TARGETS += $(LOG_TEST_TARGET)


LOG_RELY_ON_UTIL = ipc/message.cc ipc/message_queue.cc ipc/signal.cc
LOG_TEST_SRCS = $(shell find $(LOG_SRC_DIR) -name "*.cc") \
				$(addprefix $(UTIL_SRC_DIR),$(LOG_RELY_ON_UTIL))

LOG_TEST_OBJS := $(LOG_TEST_SRCS:%=$(BUILD_DIR)/%.o)

.PHONY += $(LOG_TEST_TARGET)
$(LOG_TEST_TARGET) : $(LOG_TEST_OBJS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(LOG_TEST_OBJS) -o $(BIN_DIR)/$@
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