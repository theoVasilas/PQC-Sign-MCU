#ifndef TEST_SIGNATURE_API_H
#define TEST_SIGNATURE_API_H

#include "api.h"

/* The build selects one PQClean implementation and its matching api.h. */
#ifndef TEST_PREFIX
#error "Define TEST_PREFIX to the selected PQClean namespace"
#endif
#define TEST_JOIN_INNER(a, b) a ## b
#define TEST_JOIN(a, b) TEST_JOIN_INNER(a, b)
#define TEST_API(name) TEST_JOIN(TEST_PREFIX, name)

#define TEST_NAME TEST_API(CRYPTO_ALGNAME)
#define TEST_PUBLICKEY_BYTES TEST_API(CRYPTO_PUBLICKEYBYTES)
#define TEST_SECRETKEY_BYTES TEST_API(CRYPTO_SECRETKEYBYTES)
#define TEST_SIGNATURE_BYTES TEST_API(CRYPTO_BYTES)
#define test_keypair TEST_API(crypto_sign_keypair)
#define test_sign TEST_API(crypto_sign_signature)
#define test_verify TEST_API(crypto_sign_verify)

#endif
