#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "api.h"
#include "helper.h"

int main(void)
{
    /* Allocate buffers using the sizes defined in api.h */
    uint8_t public_key[PQCLEAN_FALCON512_CLEAN_CRYPTO_PUBLICKEYBYTES];
    uint8_t secret_key[PQCLEAN_FALCON512_CLEAN_CRYPTO_SECRETKEYBYTES];

    uint8_t signature[PQCLEAN_FALCON512_CLEAN_CRYPTO_BYTES];
    size_t signature_len;

    const uint8_t message[] = "Hello Falcon-512!";
    size_t message_len = strlen((const char *)message);

    printf("=== Falcon-512 Test ===\n\n");

    /* Generate key pair */
    printf("Generating keypair...\n");

    if (PQCLEAN_FALCON512_CLEAN_crypto_sign_keypair(public_key, secret_key) != 0) {
        printf("Key generation FAILED\n");
        return 1;
    }

    printf("Key generation OK\n");
    print_hex("public_key", public_key, PQCLEAN_FALCON512_CLEAN_CRYPTO_PUBLICKEYBYTES);
    print_hex("secret_key", secret_key, PQCLEAN_FALCON512_CLEAN_CRYPTO_SECRETKEYBYTES);

    /* Sign message */
    printf("Signing message...\n");

    if (PQCLEAN_FALCON512_CLEAN_crypto_sign_signature(
            signature,
            &signature_len,
            message,
            message_len,
            secret_key) != 0) {

        printf("Signing FAILED\n");
        return 1;
    }

    printf("Signing OK\n");
    printf("Signature length: %zu bytes\n", signature_len);
    print_hex("Signature", signature, signature_len);

    /* Verify signature */
    printf("Verifying signature...\n");

    if (PQCLEAN_FALCON512_CLEAN_crypto_sign_verify(
            signature,
            signature_len,
            message,
            message_len,
            public_key) != 0) {

        printf("Verification FAILED\n");
        return 1;
    }

    printf("Verification SUCCESS\n");

    return 0;
}