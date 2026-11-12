/* Windows host latency benchmark. stdout is CSV; stderr is the run summary. */
#ifndef _WIN32
#error "This benchmark uses Windows QueryPerformanceCounter; build with MinGW-w64."
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "signature_api.h"

// macros for retrieving the implementation name from the build system. The build system
#define STRINGIFY_INNER(x) #x
#define STRINGIFY(x) STRINGIFY_INNER(x)

static uint8_t public_key[TEST_PUBLICKEY_BYTES];
static uint8_t secret_key[TEST_SECRETKEY_BYTES];
static uint8_t signature[TEST_SIGNATURE_BYTES];
static double ticks_per_us;

// The sample struct holds the measured latencies and signature size for a single iteration in miliseconds
typedef struct {
    double keygen_us;
    double sign_us;
    double verify_us;
    size_t signature_bytes;
} sample;

static LONGLONG timestamp(void)
{
    LARGE_INTEGER counter;
    if (!QueryPerformanceCounter(&counter)) { //Windows high-resolution performance counter
        fputs("FAIL: QueryPerformanceCounter\n", stderr);
        exit(EXIT_FAILURE);
    }
    return counter.QuadPart;
}

/* Parse bounded decimal integers, rejecting signs, whitespace and overflow. */
static int parse_number(const char *text, size_t maximum, size_t *result)
{
    size_t value = 0;
    if (*text == '\0') {
        return 0;
    }
    for (; *text; ++text) {
        if (*text < '0' || *text > '9') {
            return 0;
        }
        size_t digit = (size_t)(*text - '0');
        if (value > (maximum - digit) / 10) {
            return 0;
        }
        value = value * 10 + digit;
    }
    *result = value;
    return 1;
}

static int measure(const uint8_t *message, size_t message_bytes, sample *out)
{
    LONGLONG start, end;
    int status;
    size_t signature_bytes = 0;

    /* Each iteration signs with a fresh key and verifies its new signature.
     * Return checks and all output are outside the timed intervals.
     * Platform randomness requested inside PQClean is included in latency. */
    start = timestamp();
    status = test_keypair(public_key, secret_key);
    end = timestamp();
    if (status != 0) {
        fputs("FAIL: key generation\n", stderr);
        return 0;
    }
    out->keygen_us = (double)(end - start) / ticks_per_us;

    start = timestamp();
    status = test_sign(signature, &signature_bytes, message, message_bytes,
                       secret_key);
    end = timestamp();
    if (status != 0 || signature_bytes == 0 || signature_bytes > sizeof signature) {
        fputs("FAIL: signing or invalid signature length\n", stderr);
        return 0;
    }
    out->sign_us = (double)(end - start) / ticks_per_us;
    out->signature_bytes = signature_bytes;

    start = timestamp();
    status = test_verify(signature, signature_bytes, message, message_bytes,
                         public_key);
    end = timestamp();
    if (status != 0) {
        fputs("FAIL: valid signature rejected\n", stderr);
        return 0;
    }
    out->verify_us = (double)(end - start) / ticks_per_us;
    return 1;
}

static int compare_double(const void *left, const void *right)
{
    double a = *(const double *)left;
    double b = *(const double *)right;
    return (a > b) - (a < b);
}

static void summarize(const char *operation, double *values, size_t count)
{
    double total = 0;
    for (size_t i = 0; i < count; ++i) {
        total += values[i];
    }
    qsort(values, count, sizeof *values, compare_double);
    double median = count % 2 ? values[count / 2]
        : (values[count / 2 - 1] + values[count / 2]) / 2;
    fprintf(stderr, "%-8s %12.3f %12.3f %12.3f %12.3f\n", operation,
            values[0], median, total / (double)count, values[count - 1]);
}

int main(int argc, char **argv)
{
    size_t iterations = 100, message_bytes = 32, warmup = 3;
    LARGE_INTEGER frequency;
    FILE *csv = stdout;

    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        printf("Usage: %s [iterations [message_bytes [warmup [output.csv]]]]\n"
               "Defaults: 100 iterations, 32-byte message, 3 warmup runs.\n"
               "Limits: iterations 1..1000000, message 0..1048576 bytes, "
               "warmup 0..1000000.\n"
               "CSV goes to stdout or output.csv; summary goes to stderr.\n",
               argv[0]);
        return EXIT_SUCCESS;
    }
    if (argc > 5 ||
        (argc > 1 && !parse_number(argv[1], 1000000, &iterations)) ||
        iterations == 0 ||
        (argc > 2 && !parse_number(argv[2], 1048576, &message_bytes)) ||
        (argc > 3 && !parse_number(argv[3], 1000000, &warmup))) {
        fputs("Invalid arguments. Use --help for usage.\n", stderr);
        return EXIT_FAILURE;
    }
    if (!QueryPerformanceFrequency(&frequency) || frequency.QuadPart <= 0) {
        fputs("FAIL: QueryPerformanceFrequency\n", stderr);
        return EXIT_FAILURE;
    }
    ticks_per_us = (double)frequency.QuadPart / 1000000.0;

    uint8_t *message = malloc(message_bytes ? message_bytes : 1);
    sample *samples = calloc(iterations, sizeof *samples);
    double *values = malloc(iterations * sizeof *values);
    if (!message || !samples || !values) {
        fputs("FAIL: allocation\n", stderr);
        free(message);
        free(samples);
        free(values);
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < message_bytes; ++i) {
        message[i] = (uint8_t)i;
    }
    if (argc > 4) {
        csv = fopen(argv[4], "w");
        if (!csv) {
            perror("Cannot open CSV output");
            free(message);
            free(samples);
            free(values);
            return EXIT_FAILURE;
        }
    }

    fprintf(stderr, "%s Windows host benchmark; GCC %s; %zu-bit process\n"
            "Implementation: %s\n"
            "Iterations=%zu; message_bytes=%zu; warmup=%zu; QPC_Hz=%lld\n"
            "Public key=%zu bytes; secret key=%zu bytes; signature capacity=%zu bytes\n",
            TEST_NAME, __VERSION__, sizeof(void *) * 8, STRINGIFY(TEST_PREFIX),
            iterations, message_bytes, warmup, (long long)frequency.QuadPart,
            sizeof public_key, sizeof secret_key, sizeof signature);

    int result = EXIT_FAILURE;
    sample discarded;
    fputs("Warming up...\n", stderr);
    for (size_t i = 0; i < warmup; ++i) {
        if (!measure(message, message_bytes, &discarded)) {
            goto done;
        }
    }
    fputs("Measuring...\n", stderr);
    for (size_t i = 0; i < iterations; ++i) {
        if (!measure(message, message_bytes, &samples[i])) {
            goto done;
        }
    }

    /* Write after all measurements to avoid disk/terminal I/O between samples. */
    fputs("implementation,iteration,message_bytes,public_key_bytes,secret_key_bytes,"
          "signature_capacity_bytes,signature_bytes,keygen_us,sign_us,verify_us\n", csv);
    for (size_t i = 0; i < iterations; ++i) {
        fprintf(csv, "%s,%zu,%zu,%zu,%zu,%zu,%zu,%.3f,%.3f,%.3f\n",
                STRINGIFY(TEST_PREFIX), i + 1, message_bytes,
                sizeof public_key, sizeof secret_key, sizeof signature,
                samples[i].signature_bytes, samples[i].keygen_us,
                samples[i].sign_us, samples[i].verify_us);
    }
    if (fflush(csv) != 0 || ferror(csv)) {
        fputs("FAIL: writing CSV\n", stderr);
        goto done;
    }
    fprintf(stderr, "Times in microseconds (us); warmup excluded.\n"
            "%-8s %12s %12s %12s %12s\n", "Operation", "Min", "Median", "Mean", "Max");
    for (size_t i = 0; i < iterations; ++i) values[i] = samples[i].keygen_us;
    summarize("keygen", values, iterations);
    for (size_t i = 0; i < iterations; ++i) values[i] = samples[i].sign_us;
    summarize("sign", values, iterations);
    for (size_t i = 0; i < iterations; ++i) values[i] = samples[i].verify_us;
    summarize("verify", values, iterations);
    result = EXIT_SUCCESS;

done:
    if (csv != stdout && fclose(csv) != 0) {
        fputs("FAIL: closing CSV\n", stderr);
        result = EXIT_FAILURE;
    }
    free(message);
    free(samples);
    free(values);
    return result;
}
