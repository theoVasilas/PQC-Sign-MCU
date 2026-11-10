# Select one portable PQClean implementation: make test ALGORITHM=ml-dsa-44
.DEFAULT_GOAL := all
CC = gcc
AR = ar
CFLAGS = -O2 -std=c99 -Wall -Wextra
CPPFLAGS =
LDFLAGS =
LDLIBS = -lm
# default algorithm
ALGORITHM ?= falcon-512 

PREFIX_falcon-512 := PQCLEAN_FALCON512_CLEAN_
PREFIX_falcon-1024 := PQCLEAN_FALCON1024_CLEAN_
PREFIX_falcon-padded-512 := PQCLEAN_FALCONPADDED512_CLEAN_
PREFIX_falcon-padded-1024 := PQCLEAN_FALCONPADDED1024_CLEAN_
PREFIX_ml-dsa-44 := PQCLEAN_MLDSA44_CLEAN_
PREFIX_ml-dsa-65 := PQCLEAN_MLDSA65_CLEAN_
PREFIX_ml-dsa-87 := PQCLEAN_MLDSA87_CLEAN_
PREFIX_sphincs-sha2-128f-simple := PQCLEAN_SPHINCSSHA2128FSIMPLE_CLEAN_
PREFIX_sphincs-sha2-128s-simple := PQCLEAN_SPHINCSSHA2128SSIMPLE_CLEAN_
PREFIX_sphincs-sha2-192f-simple := PQCLEAN_SPHINCSSHA2192FSIMPLE_CLEAN_
PREFIX_sphincs-sha2-192s-simple := PQCLEAN_SPHINCSSHA2192SSIMPLE_CLEAN_
PREFIX_sphincs-sha2-256f-simple := PQCLEAN_SPHINCSSHA2256FSIMPLE_CLEAN_
PREFIX_sphincs-sha2-256s-simple := PQCLEAN_SPHINCSSHA2256SSIMPLE_CLEAN_
PREFIX_sphincs-shake-128f-simple := PQCLEAN_SPHINCSSHAKE128FSIMPLE_CLEAN_
PREFIX_sphincs-shake-128s-simple := PQCLEAN_SPHINCSSHAKE128SSIMPLE_CLEAN_
PREFIX_sphincs-shake-192f-simple := PQCLEAN_SPHINCSSHAKE192FSIMPLE_CLEAN_
PREFIX_sphincs-shake-192s-simple := PQCLEAN_SPHINCSSHAKE192SSIMPLE_CLEAN_
PREFIX_sphincs-shake-256f-simple := PQCLEAN_SPHINCSSHAKE256FSIMPLE_CLEAN_
PREFIX_sphincs-shake-256s-simple := PQCLEAN_SPHINCSSHAKE256SSIMPLE_CLEAN_
SUPPORTED := falcon-512 falcon-1024 falcon-padded-512 falcon-padded-1024 \
             ml-dsa-44 ml-dsa-65 ml-dsa-87 \
             sphincs-sha2-128f-simple sphincs-sha2-128s-simple \
             sphincs-sha2-192f-simple sphincs-sha2-192s-simple \
             sphincs-sha2-256f-simple sphincs-sha2-256s-simple \
             sphincs-shake-128f-simple sphincs-shake-128s-simple \
             sphincs-shake-192f-simple sphincs-shake-192s-simple \
             sphincs-shake-256f-simple sphincs-shake-256s-simple
ifeq ($(filter $(ALGORITHM),$(SUPPORTED)),)
$(error Unsupported ALGORITHM '$(ALGORITHM)'; run make list)
endif

ALGORITHM_DIR := external/PQClean/crypto_sign/$(ALGORITHM)/clean
COMMON_DIR := external/PQClean/common
BUILD_DIR := build/host/$(ALGORITHM)
EXEEXT :=
ifeq ($(OS),Windows_NT)
EXEEXT := .exe
LDLIBS += -ladvapi32
endif

ALGORITHM_SOURCES := $(wildcard $(ALGORITHM_DIR)/*.c)
ALGORITHM_OBJECTS := $(patsubst $(ALGORITHM_DIR)/%.c,$(BUILD_DIR)/algorithm_%.o,$(ALGORITHM_SOURCES))
COMMON_NAMES := randombytes
ifneq ($(filter sphincs-sha2-%,$(ALGORITHM)),)
COMMON_NAMES += sha2
else
COMMON_NAMES += fips202
endif
COMMON_OBJECTS := $(addprefix $(BUILD_DIR)/common_,$(addsuffix .o,$(COMMON_NAMES)))
TEST_OBJECT := $(BUILD_DIR)/test_main.o
OBJECTS := $(ALGORITHM_OBJECTS) $(COMMON_OBJECTS) $(TEST_OBJECT)
LIBRARY := $(BUILD_DIR)/lib$(ALGORITHM)_clean.a
TARGET := $(BUILD_DIR)/signature_test$(EXEEXT)
INCLUDES := -I$(ALGORITHM_DIR) -I$(COMMON_DIR) -Itests/common
TEST_DEFINES := -DTEST_PREFIX=$(PREFIX_$(ALGORITHM))

.PHONY: all lib test list test-all clean
all: $(TARGET)
lib: $(LIBRARY)
test: $(TARGET)
	./$(TARGET)
list:
	@echo $(SUPPORTED)

# Recursive targets remain compatible with both Windows cmd and Unix shells.
test-all: $(addprefix test-,$(SUPPORTED))
.PHONY: $(addprefix test-,$(SUPPORTED))
$(addprefix test-,$(SUPPORTED)):
	$(MAKE) test ALGORITHM=$(patsubst test-%,%,$@)

$(TARGET): $(TEST_OBJECT) $(LIBRARY) $(COMMON_OBJECTS)
	$(CC) $(LDFLAGS) $(TEST_OBJECT) $(LIBRARY) $(COMMON_OBJECTS) $(LDLIBS) -o $@

# Applications supply SHAKE/SHA-2 and platform randomness separately.
$(LIBRARY): $(ALGORITHM_OBJECTS)
	$(AR) rcs $@ $^

$(BUILD_DIR):
ifeq ($(OS),Windows_NT)
	cmd /C "if not exist $(subst /,\,$@) mkdir $(subst /,\,$@)"
else
	mkdir -p $@
endif

$(BUILD_DIR)/algorithm_%.o: $(ALGORITHM_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/common_%.o: $(COMMON_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -MMD -MP -c $< -o $@

$(TEST_OBJECT): tests/signatures/main.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(INCLUDES) $(TEST_DEFINES) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
ifeq ($(OS),Windows_NT)
	-cmd /C del /Q $(subst /,\,$(OBJECTS) $(OBJECTS:.o=.d) $(LIBRARY) $(TARGET))
else
	$(RM) $(OBJECTS) $(OBJECTS:.o=.d) $(LIBRARY) $(TARGET)
endif

-include $(OBJECTS:.o=.d)
