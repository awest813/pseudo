/* Memory card unit tests: sector read/write via the SIO protocol. */

#include "Global.h"

#include <cstdio>
#include <cstring>


static int failed = 0;

static void check(bool cond, const char *name) {
    printf("%s %s\n", cond ? "ok  " : "FAIL", name);
    if (!cond) {
        failed++;
    }
}

static ub exchange(ub host) {
    memcard.hostByte(host);
    check(memcard.ready(), "response ready");
    return memcard.read();
}

int main() {
    const char *path = "memcard1.mcr";
    remove(path);

    memcard.reset();

    // Write a marker into sector 5 through the protocol.
    memcard.begin();
    check(exchange(0x57) == 0x08, "write: FLAG");
    check(exchange(0x00) == 0x5a, "write: ID1");
    check(exchange(0x00) == 0x5d, "write: ID2");
    check(exchange(0x05) == 0x00, "write: addr MSB");
    check(exchange(0x00) == 0x05, "write: addr LSB");

    ub checksum = (ub)(0x05 ^ 0x00);
    for (int i = 0; i < 128; i++) {
        const ub value = (ub)(0xa0 + i);
        checksum ^= value;
        exchange(value);
    }
    check(exchange(checksum) == checksum, "write: checksum echo");
    check(exchange(0x00) == 0x5c, "write: ACK1");
    check(exchange(0x00) == 0x5d, "write: ACK2");
    check(exchange(0x00) == 0x47, "write: end");

    // Read sector 5 back and verify the marker bytes.
    memcard.begin();
    check(exchange(0x52) == 0x08, "readback: FLAG");
    check(exchange(0x00) == 0x5a, "readback: ID1");
    check(exchange(0x00) == 0x5d, "readback: ID2");
    check(exchange(0x05) == 0x00, "readback: addr MSB ack");
    check(exchange(0x00) == 0x05, "readback: addr LSB echo");
    check(exchange(0x00) == 0x5c, "readback: ACK1");
    check(exchange(0x00) == 0x5d, "readback: ACK2");
    check(exchange(0x00) == 0x05, "readback: confirm MSB");
    check(exchange(0x00) == 0x00, "readback: confirm LSB");

    checksum = (ub)(0x05 ^ 0x00);
    for (int i = 0; i < 128; i++) {
        const ub got = exchange(0x00);
        checksum ^= got;
        check(got == (ub)(0xa0 + i), "readback: data byte");
    }
    check(exchange(0x00) == checksum, "readback: checksum");
    check(exchange(0x00) == 0x47,   "readback: end");

    // GET ID
    memcard.begin();
    check(exchange(0x53) == 0x08, "getid: FLAG");
    check(exchange(0x00) == 0x5a, "getid: ID1");
    check(exchange(0x00) == 0x5d, "getid: ID2");
    check(exchange(0x00) == 0x5c, "getid: ACK1");
    check(exchange(0x00) == 0x5d, "getid: ACK2");
    check(exchange(0x00) == 0x04, "getid: 04");
    check(exchange(0x00) == 0x00, "getid: 00");
    check(exchange(0x00) == 0x00, "getid: 00");
    check(exchange(0x00) == 0x80, "getid: 80");

    FILE *fp = fopen(path, "rb");
    check(fp != nullptr, "save file exists");
    if (fp) {
        ub marker = 0;
        fseek(fp, 5 * 128, SEEK_SET);
        fread(&marker, 1, 1, fp);
        check(marker == (ub)0xa0, "saved sector 5 byte 0");
        fclose(fp);
        remove(path);
    }

    if (failed) {
        printf("\nFAILED (%d failures)\n", failed);
        return 1;
    }

    printf("\nPASSED (0 failures)\n");
    return 0;
}
