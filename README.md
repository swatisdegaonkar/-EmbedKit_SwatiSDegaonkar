# EmbedKit\_Swati

**Embedded C Utility Library**  
Embed Square Solutions Pvt. Ltd. — Embedded Developer Fresher Assessment

**Author:** Swati S Degaonkar 
**Language:** C (C99)  
**Platform:** Windows (MinGW)

---

## 📦 Module Overview

| Module | File | Description |
|--------|------|-------------|
| Ring Buffer | `ringbuf.c` | A fixed-capacity (8-byte) circular FIFO buffer for `uint8_t` data, modelling the producer/consumer pattern used in UART receive paths on embedded systems. |

---

## 🔁 Ring Buffer — `ringbuf.c`

### What Is a Ring Buffer?

A ring buffer (circular buffer / FIFO queue) is the standard data structure for transferring data between a **producer** (e.g., a UART hardware interrupt) and a **consumer** (e.g., the main application loop) that run at different rates.

Two indices — **head** (write position) and **tail** (read position) — advance forward and wrap back to zero when they reach the end of the array, forming a logical circle.

### Features Implemented

| Feature | Description |
|---------|-------------|
| `ringbuf_init()` | Initialise the buffer to an empty state |
| `ringbuf_write()` | Write one byte; returns error if buffer is full (never overwrites unread data) |
| `ringbuf_read()` | Read one byte; returns error if buffer is empty (never returns garbage) |
| `ringbuf_count()` | Query how many bytes are currently stored |
| `ringbuf_is_full()` | Check whether the buffer is completely full |
| `ringbuf_is_empty()` | Check whether the buffer is completely empty |

### Demo Sequence (in `main()`)

The program runs the following steps and prints every operation:

1. **Fill** — Write 8 bytes (`0x41`–`0x48`). Confirm buffer is full, count = 8.
2. **Overflow** — Attempt to write `0x99`. Confirm write fails (buffer full).
3. **Partial Read** — Read 3 bytes. Expect `0x41`, `0x42`, `0x43`. Confirm count = 5.
4. **Wrap-Around Refill** — Write 3 new bytes (`0x49`, `0x4A`, `0x4B`) into the freed slots. Confirm count = 8.
5. **Drain** — Read all 8 remaining bytes. Confirm buffer is empty.
6. **Underflow** — Attempt to read from empty buffer. Confirm read fails.

### Bonus: Bitwise AND Optimisation

All modulo operations (`% BUFFER_SIZE`) are replaced with a single bitwise AND (`& (BUFFER_SIZE - 1)`):

```c
rb->head = (rb->head + 1U) & BUFFER_MASK;   /* instead of % BUFFER_SIZE */
```

**Why it's faster:** The `%` operator compiles to a hardware division (or a multi-cycle software division loop on MCUs like ARM Cortex-M0 that lack a DIV instruction). A bitwise AND is a **single-cycle** instruction on every processor.

**Why it only works for powers of 2:** A power-of-2 value `N` has exactly one bit set. `(N - 1)` clears that bit and sets all lower bits to 1, creating a perfect bit mask. For example: `8 = 0b00001000` → `7 = 0b00000111`. ANDing any index with this mask keeps only the lowest 3 bits, which is mathematically identical to `index % 8`. For non-power-of-2 sizes, `(N - 1)` does not form a valid mask and produces incorrect wrap-around.

---

## 🛠️ Build Instructions

### Prerequisites

- **Compiler:** GCC (any version supporting C99)
- **Platform:** Linux, macOS, or Windows (MinGW / WSL)
- **Dependencies:** Standard C library only (`<stdio.h>`, `<stdint.h>`, `<string.h>`) — no external libraries

### Compile

```bash
gcc -Wall -std=c99 ringbuf.c -o ringbuf
```

> The code compiles with **zero warnings** and **zero errors**.

### Run

**Linux / macOS:**
```bash
./ringbuf
```

**Windows (MinGW):**
```bash
./ringbuf.exe
```

---

## 📋 Expected Output

```
=== Ring Buffer Demo (capacity = 8 bytes) ===

--- Step 1: Write 8 bytes to fill the buffer ---
[WRITE] 0x41 -> OK  (count=1)
[WRITE] 0x42 -> OK  (count=2)
[WRITE] 0x43 -> OK  (count=3)
[WRITE] 0x44 -> OK  (count=4)
[WRITE] 0x45 -> OK  (count=5)
[WRITE] 0x46 -> OK  (count=6)
[WRITE] 0x47 -> OK  (count=7)
[WRITE] 0x48 -> OK  (count=8) FULL
Buffer full? YES | Count = 8

--- Step 2: Attempt write when full ---
[WRITE] 0x99 -> FAIL (buffer full)

--- Step 3: Read 3 bytes ---
[READ]        -> 0x41  (count=7)
[READ]        -> 0x42  (count=6)
[READ]        -> 0x43  (count=5)
Count = 5

--- Step 4: Write 3 bytes into freed slots ---
[WRITE] 0x49 -> OK  (count=6)
[WRITE] 0x4A -> OK  (count=7)
[WRITE] 0x4B -> OK  (count=8) FULL
Count = 8

--- Step 5: Read all remaining bytes ---
[READ]        -> 0x44  (count=7)
[READ]        -> 0x45  (count=6)
[READ]        -> 0x46  (count=5)
[READ]        -> 0x47  (count=4)
[READ]        -> 0x48  (count=3)
[READ]        -> 0x49  (count=2)
[READ]        -> 0x4A  (count=1)
[READ]        -> 0x4B  (count=0)
Buffer empty? YES | Count = 0

--- Step 6: Attempt read when empty ---
[READ]  (empty) -> FAIL (buffer empty)

=== Demo Complete ===
```

---

## ✅ Code Quality Checklist

- [x] Compiles with `gcc -Wall -std=c99` — zero warnings, zero errors
- [x] Uses `uint8_t` from `<stdint.h>` for all fixed-width data (no bare `int` or `char`)
- [x] Constants defined with `#define` — no magic numbers
- [x] Clear, descriptive function and variable names
- [x] Formatted output readable without parsing raw hex dumps
- [x] Standard C only — no external dependencies
- [x] Bonus: Bitwise AND optimisation with inline documentation

---

## 📂 Repository Structure

```
EmbedKit_Swati/
├── README.md        ← This file
└── ringbuf.c        ← Ring buffer module (standalone program with main())
```

---

*Embed Square Solutions Pvt. Ltd. — EmbedKit: Embedded C Utility Library*
