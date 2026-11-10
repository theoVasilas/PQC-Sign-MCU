#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "signature_api.h"

/* Static buffers keep large SPHINCS+ signatures off the application stack. */
static uint8_t public_key[TEST_PUBLICKEY_BYTES];
static uint8_t secret_key[TEST_SECRETKEY_BYTES];
static uint8_t signature[TEST_SIGNATURE_BYTES];
static uint8_t message[1024];

static int check_message(size_t message_len)
{
    size_t signature_len = 0;

    //test 1
    if (test_sign(signature, &signature_len, message, message_len,
                  secret_key) != 0) {
        fprintf(stderr, "FAIL: signing (%zu-byte message)\n", message_len);
        return 1;
    }
    if (signature_len == 0 || signature_len > sizeof signature) {
        fprintf(stderr, "FAIL: invalid signature length %zu\n", signature_len);
        return 1;
    }

    //test 2
    if (test_verify(signature, signature_len, message, message_len,
                    public_key) != 0) {
        fprintf(stderr, "FAIL: valid signature rejected\n");
        return 1;
    }

    //test 3
    /* bitwise XOR. It flips the least significant bit of the first message byte */
    message[0] ^= 1;
    if (test_verify(signature, signature_len, message,
                    message_len == 0 ? 1 : message_len, public_key) == 0) {
        fprintf(stderr, "FAIL: modified message accepted\n");
        return 1;
    }
    message[0] ^= 1; // Restore the byte

    //test 4
    signature[signature_len / 2] ^= 1;
    if (test_verify(signature, signature_len, message, message_len,
                    public_key) == 0) {
        fprintf(stderr, "FAIL: modified signature accepted\n");
        return 1;
    }
    signature[signature_len / 2] ^= 1;

    //test 5
    if (test_verify(signature, signature_len - 1, message, message_len,
                    public_key) == 0) {
        fprintf(stderr, "FAIL: truncated signature accepted\n");
        return 1;
    }
    printf("PASS: message=%zu bytes, signature=%zu bytes; tampering rejected\n",
           message_len, signature_len); 
    fflush(stdout); // requests that buffered output be written immediately
    return 0;
}

int main(void)
{
//      0 bytes	    Empty-message edge case
//      1 byte	    Smallest nonempty message
//      32 bytes	Typical digest-sized input, although the API treats it as a message
//      1024 bytes	Larger input to exercise message-length handling
    const size_t lengths[] = {0, 1, 32, sizeof message};

    printf("=== %s desktop test ===\n", TEST_NAME);

    printf("Public key: %zu bytes; secret key: %zu bytes; signature capacity: %zu bytes\n",
           sizeof public_key, sizeof secret_key, sizeof signature);

    puts("Generating key pair...");

    fflush(stdout);

    if (test_keypair(public_key, secret_key) != 0) {
        fprintf(stderr, "FAIL: key generation\n");
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < sizeof message; ++i) {
        message[i] = (uint8_t)i;
    }
    unsigned int elements = sizeof lengths / sizeof lengths[0];
    for (size_t i = 0; i < elements; ++i) {
        if (check_message(lengths[i]) != 0) {
            return EXIT_FAILURE;
        }
    }

    puts("PASS: all checks completed");
    return EXIT_SUCCESS;
}
