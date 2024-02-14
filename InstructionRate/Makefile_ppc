include ../Common/arch_detect.mk

CFLAGS ?= -O3 -Wall -pipe

all: $(TARGET)

amd64:
	$(CC) $(CFLAGS) x86_instructionrate.s x86_instructionrate.c -o x86_instructionrate $(LDFLAGS)

aarch64:
	$(CC) $(CFLAGS) -march=native -pthread arm_instructionrate.s arm_instructionrate.c -o aarch64_instructionrate $(LDFLAGS)

termux:
	clang -march=armv8+aes arm_instructionrate.s arm_instructionrate.c -o aarch64_instructionrate $(LDFLAGS)

amd64_fusion:
	$(CC) $(CFLAGS) x86_fusion.s x86_fusion.c -o x86_instructionratefusion $(LDFLAGS)

ppc64:
	$(CC) $(CFLAGS) -mcpu=native -c ppc64_instructionrate_tests.c -S ppc64_instructionrate_tests.s
	$(CC) $(CFLAGS) -mcpu=native ppc64_instructionrate_tests.s ppc64_instructionrate.c -o ppc64_instructionrate $(LDFLAGS)

w64:
	$(CC) $(CFLAGS) x86_instructionrate.c x86_instructionrate.s -o w64_instructionrate.exe $(LDFLAGS)

ci: amd64 amd64_fusion aarch64 w64

clean:
	rm -f *.o && find . -type f -executable -delete

.PHONY: all ci clean
