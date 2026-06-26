# sffe build system
#
# Builds the real (double) back-end as a static library and runs the test
# suite. The complex back-ends (GSL / x87 asm) are currently out of sync with
# the refactored core (see CLAUDE.md "Known issues") and are intentionally not
# wired up here yet.

CC       ?= cc
AR       ?= ar
CFLAGS   ?= -Wall -Wextra -O2 -std=c99
CPPFLAGS += -D_GNU_SOURCE -Ilib/include

# Both macros are required. sffe_real.c guards its whole body on SFFE_DOUBLE
# *before* it includes sffe.h (which is what turns SFFE_REAL into SFFE_DOUBLE),
# so passing SFFE_REAL alone compiles that file to nothing.
DEFS     := -DSFFE_REAL -DSFFE_DOUBLE

BUILD    := build
LIB_SRC  := lib/src/sffe.c lib/src/sffe_real.c
LIB_OBJ  := $(LIB_SRC:%.c=$(BUILD)/%.o)
LIB      := $(BUILD)/libsffe.a

TEST_SRC := test/test_sffe.c
TEST_BIN := $(BUILD)/test_sffe

# Locate the Check unit-testing framework via pkg-config, with a plain
# -lcheck fallback if no .pc file is installed.
CHECK_CFLAGS := $(shell pkg-config --cflags check 2>/dev/null)
CHECK_LIBS   := $(shell pkg-config --libs check 2>/dev/null)
CHECK_LIBS   := $(if $(CHECK_LIBS),$(CHECK_LIBS),-lcheck)

.PHONY: all lib test clean

all: lib

lib: $(LIB)

$(LIB): $(LIB_OBJ)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEFS) -c $< -o $@

test: $(LIB)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEFS) $(CHECK_CFLAGS) \
		$(TEST_SRC) $(LIB) $(CHECK_LIBS) -lm -o $(TEST_BIN)
	$(TEST_BIN)

clean:
	rm -rf $(BUILD)
