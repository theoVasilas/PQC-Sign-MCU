CC = gcc

CFLAGS = -Wall -O2 -std=c99 # Optimization level 2. C99 standard

ROOT = ../../PQC-Sign-MCU/PQC-libs/FN-DSA/PQClean
FALCON_DIR = $(ROOT)/crypto_sign/falcon-512/clean
COMMON_DIR = $(ROOT)/common

SRC = main.c \
	$(FALCON_DIR)/codec.c \
	$(FALCON_DIR)/common.c \
	$(FALCON_DIR)/fft.c \
	$(FALCON_DIR)/fpr.c \
	$(FALCON_DIR)/keygen.c \
	$(FALCON_DIR)/pqclean.c \
	$(FALCON_DIR)/rng.c \
	$(FALCON_DIR)/sign.c \
	$(FALCON_DIR)/vrfy.c \
	$(COMMON_DIR)/fips202.c \
	$(COMMON_DIR)/randombytes.c

INCLUDES = -I$(FALCON_DIR) -I$(COMMON_DIR)

LIBS = -lm

TARGET = falcon_test

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) -o $@ $(LIBS)

clean:
	rm -f $(TARGET)