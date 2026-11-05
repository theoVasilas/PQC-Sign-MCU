# PQC-Sign-MCU

Desktop tests for PQClean signature implementations, with a future
NUCLEO-H753ZI / STM32CubeIDE target. Board integration is deferred.

## Layout

```text
external/PQClean/
  crypto_sign/              19 portable clean implementations
  common/                   SHAKE, SHA-2, randomness, compatibility header
tests/
  signatures/main.c         Shared desktop correctness smoke tests
  common/signature_api.h    Maps the selected PQClean API to the test runner
  common/helper.[ch]        Hex output helper for the original demo
  falcon512/main.c          Original Falcon-512 demo (not the default runner)
build/host/<algorithm>/     Generated objects, archive, and executable
Makefile                    Selects one algorithm and its dependencies
```

The build uses `external/PQClean/`, never `build/vendor-source/`. The downloaded
staging clone can be deleted after copying the required sources and licenses.
The vendor notes are in [external/PQClean/README.md](external/PQClean/README.md).

## PC prerequisites

Use GCC, GNU Make, and GNU Binutils (`ar`) from a compatible toolchain.
On Windows, use MinGW-w64/MSYS2; GNU Make may be named `mingw32-make`.
On Linux, run the same commands using the native development tools.
Run all commands from the repository root.

Verify the installed tools in your terminal:

```sh
gcc --version
make --version
ar --version
```

If Make is named `mingw32-make`, substitute that name below. No additional
cryptographic library download is needed for these tests.

## Build and test

```sh
make test                                 # Default: Falcon-512
make test ALGORITHM=ml-dsa-44
make test ALGORITHM=sphincs-shake-128f-simple
make test ALGORITHM=sphincs-sha2-128f-simple
make list                                 # All supported algorithm names
```

Use the directory names in `external/PQClean/crypto_sign/` as `ALGORITHM`:

| Family | Supported variants |
| --- | --- |
| Falcon | falcon-512, falcon-1024, falcon-padded-512, falcon-padded-1024 |
| ML-DSA | ml-dsa-44, ml-dsa-65, ml-dsa-87 |
| SPHINCS+ SHA-2 | sphincs-sha2-{128,192,256}{f,s}-simple |
| SPHINCS+ SHAKE | sphincs-shake-{128,192,256}{f,s}-simple |

Braces in the table abbreviate the names; use an actual name in commands.
`make test-all` builds and runs all 19 variants. SPHINCS+ `s` variants can take
substantially longer; start with the individual commands above.

Other targets:

```sh
make ALGORITHM=ml-dsa-44                   # Build without running
make lib ALGORITHM=ml-dsa-44               # Algorithm archive only
make clean ALGORITHM=ml-dsa-44             # Clean only that variant
```

Each variant has an isolated directory under `build/host/`, containing
`signature_test` (`signature_test.exe` on Windows), a library archive,
objects, and generated header dependencies. Archives contain algorithm code;
applications must also provide the required common hash and randomness code.
The Makefile links these dependencies into the desktop executable.
Windows builds also link Advapi32 for the vendored randomness implementation.

The default flags are `-O2 -std=c99 -Wall -Wextra`. Compiler tools and flags can
be overridden on the make command line. Clean the selected variant before
changing its compiler or flags. Avoid running concurrent builds of the same
variant into the same directory.

## Test coverage

For each selected implementation, the shared runner:

1. Generates a key pair.
2. Signs and verifies messages of 0, 1, 32, and 1024 bytes (including binary data).
3. Checks the signature length against the API capacity.
4. Requires rejection of a changed message, a changed signature, and a
   truncated signature for each message size.

Success prints `PASS: all checks completed` and returns exit status zero.
A failed check prints `FAIL` and returns a nonzero status. The runner does not
print secret keys. ML-DSA uses its no-context API. These are randomized smoke
checks, not official known-answer tests, exhaustive validation, or benchmarks.
The original Falcon demo still prints keys if built separately.

The test wiring has been checked against all 19 API headers. Compilation and
runtime verification require running the commands with the PC toolchain.
