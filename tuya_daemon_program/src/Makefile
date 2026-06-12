SRC_DIR := .
BUILD_DIR := build
INSTALL_DIR := /usr/local/bin
TARGET_DIR ?= .

# OpenWrt/rutOS SDK exports STAGING_DIR for target headers and libraries.
CPPFLAGS += -I$(CURDIR)/include 
CFLAGS += -Wall -Wextra -std=gnu99 -Werror -D_GNU_SOURCE

TUYA_LIBS := -llink_core -lplatform_port -lutils_modules -lmiddleware_implementation

LDLIBS += $(TUYA_LIBS) -lubus -lblobmsg_json -lubox -luci -largp -ljson-c -lm 

TARGET := tuya_daemon
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $(TARGET_DIR)/$@ $^ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

clean:
	rm -rf $(BUILD_DIR) $(DEPS) $(TARGET_DIR)/$(TARGET)

.PHONY: all clean

install: $(TARGET)
	mkdir -p $(INSTALL_DIR)
	install -m 755 $(TARGET_DIR)/$(TARGET) $(INSTALL_DIR)

uninstall:
	rm -f $(INSTALL_DIR)/$(TARGET)

.PHONY: install uninstall

-include $(DEPS)