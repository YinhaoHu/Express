CXX := g++
INC_DIR := ./src
CFLAGS := -g -std=c++20 -Wall
CPPFLAGS = $(addprefix -I ,$(INC_DIR)) 

BIN_DIR := ./bin
BUILD_DIR := ./build

.PHONY: 
ALL_TARGETS :=  

# =========================================================
#				Ancillary Variables
# =========================================================

# Define ANSI escape codes for text color
FONT_BLACK 		:= \033[0;30m
FONT_RED 		:= \033[0;31m
FONT_GREEN 		:= \033[0;32m
FONT_YELLOW 	:= \033[0;33m
FONT_BLUE 		:= \033[0;34m
FONT_MAGENTA 	:= \033[0;35m
FONT_CYAN 		:= \033[0;36m
FONT_WHITE 		:= \033[0;37m 

# Define ANSI escape codes for font styles
FONT_BOLD 		:= \033[1m
FONT_UNDERLINE 	:= \033[4m
FONT_INVERSE 	:= \033[7m

FONT_RESET 		:= \033[0m

 
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
#						Utility-IPC			
# Targets: test-util-ipc, ipc
# ==========================================================
UTIL_IPC_TEST_TARGET := test-util-ipc
UTIL_IPC_SRC_DIR := ./src/utility/ipc/

ALL_TARGETS += $(UTIL_IPC_TEST_TARGET)

UTIL_IPC_TEST_SRC := test/test.cc message_queue.cc tcp_client.cc  basic_socket.cc\
					tcp_server.cc socket.cc unix_domain_socket.cc sent_message.cc
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

LOG_RELY_ON_UTIL = message_queue.cc signal.cc  

LOG_TEST_SRC_FILES := test/server.cc
LOG_TEST_SRCS = $(addprefix $(LOG_SRC_DIR),$(LOG_TEST_SRC_FILES)) \
				$(addprefix $(UTIL_IPC_SRC_DIR),$(LOG_RELY_ON_UTIL))

LOG_TEST_CLIENT_SRC_FILES := test/client.cc
LOG_TEST_CLIENT_SRCS = $(addprefix $(LOG_SRC_DIR),$(LOG_TEST_CLIENT_SRC_FILES)) \
						$(addprefix $(UTIL_IPC_SRC_DIR),$(LOG_RELY_ON_UTIL))

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
# All targets about data base component.
# Supported targets: db-server, db-client, db-test
# ==========================================================
DB_SERVER_TARGET 		:=	db-server  
DB_TEST_TARGET			:=	db-test

ALL_TARGETS 			+=	$(DB_SERVER_TARGET) $(DB_TEST_TARGET)
.PHONY 					+=  $(DB_SERVER_TARGET) $(DB_TEST_TARGET)

DB_SRC_DIR 				:=	./src/database/

# About db-server

DB_SERVER_DEP_FILES		:=	./src/utility/data/rawdata.cc				\
					 		./src/utility/ipc/basic_socket.cc			\
					 		./src/utility/ipc/sent_message.cc			\
							./src/utility/ipc/socket.cc 	 			\
					 		./src/utility/ipc/unix_domain_socket.cc		\
							./src/utility/ipc/signal.cc 				\

DB_SERVER_SRC_FILES 	:=	database.cc		\
							datalist.cc 	\
							server.cc		\
							handler.cc      

DB_SERVER_SRCS 			:= 	$(DB_SERVER_DEP_FILES) \
							$(addprefix $(DB_SRC_DIR),$(DB_SERVER_SRC_FILES))

DB_SERVER_OBJS 			:= 	$(DB_SERVER_SRCS:%=$(BUILD_DIR)/%.o)

$(DB_SERVER_TARGET): $(DB_SERVER_OBJS)
	mkdir -p $(BIN_DIR)
	$(CXX) $^ -o $(BIN_DIR)/$@
	@echo "$(FONT_WHITE)Generated program $(BIN_DIR)/$@$(FONT_RESET)"

# About db-client

DB_CLIENT_DEP_FILES		:=	./src/utility/data/rawdata.cc		\
					 		./src/utility/ipc/basic_socket.cc	\
					 		./src/utility/ipc/sent_message.cc	\
							./src/utility/ipc/socket.cc 	 	\
					 		./src/utility/ipc/unix_domain_socket.cc

DB_CLIENT_SRC_FILES		:=	client.cc

DB_CLIENT_SRCS			:= 	$(DB_CLIENT_DEP_FILES) \
							$(addprefix $(DB_SRC_DIR),$(DB_CLIENT_SRC_FILES))  

DB_CLIENT_OBJS			:=	$(DB_CLIENT_SRCS:%=$(BUILD_DIR)/%.o) 

# About db-test

DB_TEST_SRC_FILES		:=	test/test.cc

DB_TEST_SRCS  			:=	$(addprefix $(DB_SRC_DIR),$(DB_TEST_SRC_FILES)) 

DB_TEST_OBJS			:=	$(DB_CLIENT_OBJS)	\
							$(DB_TEST_SRCS:%=$(BUILD_DIR)/%.o) 

$(DB_TEST_TARGET): $(DB_TEST_OBJS) 
	$(MAKE)	$(DB_SERVER_TARGET)
	mkdir -p $(BIN_DIR)
	$(CXX) $^ -o $(BIN_DIR)/$@
	@echo "$(FONT_WHITE)\rGenerated program $(FONT_BOLD)$(BIN_DIR)/$@$(FONT_RESET)"
	@echo "$(FONT_CYAN)$(FONT_BOLD)$(FONT_UNDERLINE)NOTE: You should run this"\
		  "program in the Express directory.$(FONT_RESET)"



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
