#include "board_demo.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    uint32_t now;
    uint8_t regs[16];
    bool busy, error, reject_start;
    bool adc_is_ready;
    uint16_t adc_value;
    unsigned starts, i2c_calls, adc_starts;
    uint8_t trace_reg[16], trace_value[16];
    bool trace_write[16];
    const uint8_t *held_tx;
    uint8_t *held_rx;
    uint8_t held_tx_copy[2];
    size_t held_tx_len;
    unsigned corrupt_readback_reg;
} MOCK;

static uint32_t Now(void *p) { return ((MOCK *)p)->now; }
static bool I2cWriteRead(void *p, uint8_t addr, const uint8_t *tx, size_t ntx,
                         uint8_t *rx, size_t nrx)
{
    MOCK *m = p;
    assert(addr == 0x22 && ntx == 1 && nrx == 1);
    assert(m->i2c_calls < 16);
    m->trace_reg[m->i2c_calls] = tx[0];
    m->trace_write[m->i2c_calls] = false;
    ++m->i2c_calls;
    if (m->reject_start) return false;
    m->held_tx = tx; m->held_rx = rx; m->held_tx_len = ntx;
    memcpy(m->held_tx_copy, tx, ntx);
    m->busy = true; m->error = false; ++m->starts;
    return true;
}
static bool I2cWrite(void *p, uint8_t addr, const uint8_t *tx, size_t n)
{
    MOCK *m = p;
    assert(addr == 0x22 && n == 2);
    assert(m->i2c_calls < 16);
    m->trace_reg[m->i2c_calls] = tx[0];
    m->trace_value[m->i2c_calls] = tx[1];
    m->trace_write[m->i2c_calls] = true;
    ++m->i2c_calls;
    if (m->reject_start) return false;
    m->held_tx = tx; m->held_rx = NULL; m->held_tx_len = n;
    memcpy(m->held_tx_copy, tx, n);
    m->busy = true; m->error = false; ++m->starts;
    return true;
}
static bool I2cBusy(void *p) { return ((MOCK *)p)->busy; }
static bool I2cError(void *p) { return ((MOCK *)p)->error; }
static bool AdcStart(void *p)
{
    MOCK *m = p; ++m->adc_starts; m->adc_is_ready = false; return true;
}
static bool AdcReady(void *p) { return ((MOCK *)p)->adc_is_ready; }
static uint16_t AdcResult(void *p) { return ((MOCK *)p)->adc_value; }

static const BOARD_DEMO_OPS ops = {
    Now, I2cWriteRead, I2cWrite, I2cBusy, I2cError, AdcStart, AdcReady, AdcResult
};

static void CompleteI2c(MOCK *m)
{
    assert(m->busy);
    assert(memcmp(m->held_tx, m->held_tx_copy, m->held_tx_len) == 0);
    if (m->held_rx != NULL)
    {
        uint8_t reg = m->held_tx_copy[0];
        uint8_t value = m->regs[reg];
        if (m->corrupt_readback_reg == (unsigned)reg && m->i2c_calls == 5)
            value ^= 0x01;
        *m->held_rx = value;
    }
    else
        m->regs[m->held_tx_copy[0]] = m->held_tx_copy[1];
    m->busy = false;
}

static void InitializeMock(MOCK *m)
{
    memset(m, 0, sizeof(*m));
    m->regs[0x0a] = 0x85;
    m->regs[0x00] = 0x9f;
    BOARD_DEMO_Initialize(&ops, m);
}

static void CompleteStartup(MOCK *m)
{
    unsigned guard = 20;
    while (!BOARD_DEMO_Ready() && BOARD_DEMO_Fault() == BOARD_DEMO_FAULT_NONE && guard--)
    {
        BOARD_DEMO_Tasks();
        if (m->busy)
        {
            if (m->i2c_calls < 6U)
                assert(!BOARD_DEMO_Ready());
            CompleteI2c(m);
            if (m->i2c_calls < 6U)
                assert(!BOARD_DEMO_Ready());
        }
    }
    assert(guard != 0);
    assert(BOARD_DEMO_Ready());
}

static void TestStartupPreservesAndReadback(void)
{
    MOCK m; InitializeMock(&m); CompleteStartup(&m);
    assert(m.i2c_calls == 6);
    const uint8_t expected_regs[] = {0x0a,0x0a,0x00,0x00,0x0a,0x00};
    const bool expected_write[] = {false,true,false,true,false,false};
    for (unsigned i=0; i<6; ++i) {
        assert(m.trace_reg[i] == expected_regs[i]);
        assert(m.trace_write[i] == expected_write[i]);
    }
    assert(m.trace_value[1] == 0xe5);
    assert(m.trace_value[3] == 0x9f);
    assert(m.regs[0x0a] == 0xe5 && m.regs[0x00] == 0x9f);
    assert(BOARD_DEMO_Fault() == BOARD_DEMO_FAULT_NONE);
}

static void TestI2cTimeoutQuarantinesBuffers(void)
{
    MOCK m; InitializeMock(&m);
    BOARD_DEMO_Tasks();
    assert(m.busy && m.i2c_calls == 1);
    const uint8_t *tx = m.held_tx; uint8_t *rx = m.held_rx;
    uint8_t held[2]; memcpy(held, tx, m.held_tx_len);
    m.now = 100; BOARD_DEMO_Tasks();
    assert(BOARD_DEMO_Fault() == BOARD_DEMO_FAULT_I2C);
    assert(!BOARD_DEMO_Ready());
    for (unsigned i=0; i<20; ++i) { m.now += 100; BOARD_DEMO_Tasks(); }
    assert(m.i2c_calls == 1 && m.held_tx == tx && m.held_rx == rx);
    assert(memcmp(tx, held, m.held_tx_len) == 0);
}

static void TestNackTerminal(void)
{
    MOCK m; InitializeMock(&m);
    BOARD_DEMO_Tasks(); m.error = true; m.busy = false; BOARD_DEMO_Tasks();
    assert(BOARD_DEMO_Fault() == BOARD_DEMO_FAULT_I2C && m.i2c_calls == 1);
    BOARD_DEMO_Tasks(); assert(m.i2c_calls == 1);
}

static void TestMismatchTerminal(void)
{
    MOCK m; InitializeMock(&m); m.corrupt_readback_reg = 0x0a;
    /* Alter OLAT after write so the final readback differs. */
    for (unsigned i=0; i<20 && BOARD_DEMO_Fault()==BOARD_DEMO_FAULT_NONE; ++i) {
        BOARD_DEMO_Tasks();
        if (m.busy) {
            CompleteI2c(&m);
        }
    }
    assert(BOARD_DEMO_Fault() == BOARD_DEMO_FAULT_MUX_READBACK);
    assert(!BOARD_DEMO_Ready());
    unsigned calls = m.i2c_calls; BOARD_DEMO_Tasks(); assert(m.i2c_calls == calls);
}

static void TestAdcHysteresisAndTimeout(void)
{
    MOCK m; InitializeMock(&m); CompleteStartup(&m);
    /* First sample launches immediately; each later one is 10 ms apart. */
    for (unsigned i=0; i<3; ++i) {
        if (i) m.now += 10;
        BOARD_DEMO_Tasks(); assert(m.adc_starts == i+1);
        m.adc_value = 500; m.adc_is_ready = true; BOARD_DEMO_Tasks();
    }
    assert(BOARD_DEMO_CablePresent());
    for (unsigned i=0; i<2; ++i) {
        m.now += 10; BOARD_DEMO_Tasks(); m.adc_value = 300; m.adc_is_ready = true; BOARD_DEMO_Tasks();
        assert(BOARD_DEMO_CablePresent()); /* hysteresis band retains state */
    }
    for (unsigned i=0; i<3; ++i) {
        m.now += 10; BOARD_DEMO_Tasks(); m.adc_value = 200; m.adc_is_ready = true; BOARD_DEMO_Tasks();
    }
    assert(!BOARD_DEMO_CablePresent());

    /* A conversion that never becomes ready is terminal at 10 ms. */
    m.now += 10; BOARD_DEMO_Tasks(); assert(m.adc_starts > 0);
    m.now += 10; BOARD_DEMO_Tasks();
    assert(BOARD_DEMO_Fault() == BOARD_DEMO_FAULT_ADC_TIMEOUT);
    assert(!BOARD_DEMO_Ready() && !BOARD_DEMO_CablePresent());
    unsigned starts = m.adc_starts; m.now += 100; BOARD_DEMO_Tasks();
    assert(m.adc_starts == starts);
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    if (strcmp(argv[1], "startup") == 0) TestStartupPreservesAndReadback();
    else if (strcmp(argv[1], "timeout") == 0) TestI2cTimeoutQuarantinesBuffers();
    else if (strcmp(argv[1], "nack") == 0) TestNackTerminal();
    else if (strcmp(argv[1], "mismatch") == 0) TestMismatchTerminal();
    else if (strcmp(argv[1], "adc") == 0) TestAdcHysteresisAndTimeout();
    else assert(!"unknown test");
    puts("board_demo test: PASS");
    return 0;
}
