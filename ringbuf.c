/**
 * @file    ringbuf.c
 * @brief   Ring Buffer (Circular FIFO) — Embedded C Utility
 *
 * A fixed-capacity circular buffer for uint8_t data, designed to model
 * the classic producer/consumer pattern found in UART receive paths.
 *
 * Compile:  gcc -Wall -std=c99 ringbuf.c -o ringbuf
 *
 * Embed Square Solutions Pvt. Ltd. — EmbedKit Assignment
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Constants                                                          */
/* ------------------------------------------------------------------ */

/**
 * BUFFER_SIZE must be a power of 2 so that the modulo operation
 * (index % BUFFER_SIZE) can be replaced by a single bitwise AND
 * (index & BUFFER_MASK).  See the "Bonus" comment below.
 */
#define BUFFER_SIZE     8U
#define BUFFER_MASK     (BUFFER_SIZE - 1U)

/* Return codes */
#define RINGBUF_OK      0
#define RINGBUF_ERR     (-1)

/* ------------------------------------------------------------------ */
/*  Data Types                                                         */
/* ------------------------------------------------------------------ */

/**
 * @struct RingBuf
 * @brief  Fixed-size circular buffer descriptor.
 *
 * head  — index where the next byte will be written (producer side).
 * tail  — index where the next byte will be read   (consumer side).
 * count — number of bytes currently stored in the buffer.
 * data  — the underlying storage array.
 */
typedef struct {
    uint8_t  data[BUFFER_SIZE];
    uint8_t  head;      /* write index  */
    uint8_t  tail;      /* read  index  */
    uint8_t  count;     /* bytes stored */
} RingBuf;

/* ------------------------------------------------------------------ */
/*  API — Function Prototypes                                          */
/* ------------------------------------------------------------------ */

void    ringbuf_init(RingBuf *rb);
int     ringbuf_write(RingBuf *rb, uint8_t byte);
int     ringbuf_read(RingBuf *rb, uint8_t *byte);
uint8_t ringbuf_count(const RingBuf *rb);
uint8_t ringbuf_is_full(const RingBuf *rb);
uint8_t ringbuf_is_empty(const RingBuf *rb);

/* ------------------------------------------------------------------ */
/*  API — Implementation                                               */
/* ------------------------------------------------------------------ */

/**
 * @brief Initialise the ring buffer to an empty state.
 *
 * Zeroes the storage array and resets head, tail, and count to 0.
 */
void ringbuf_init(RingBuf *rb)
{
    memset(rb->data, 0, BUFFER_SIZE);
    rb->head  = 0;
    rb->tail  = 0;
    rb->count = 0;
}

/**
 * @brief Write one byte into the buffer.
 *
 * @param rb   Pointer to the ring buffer.
 * @param byte The byte to store.
 * @return     RINGBUF_OK on success, RINGBUF_ERR if the buffer is full.
 *
 * The write will FAIL if the buffer is already full — existing unread
 * data is never silently overwritten.
 *
 * BONUS — Bitwise AND instead of modulo:
 *   head is advanced with: head = (head + 1) & BUFFER_MASK
 *   instead of:            head = (head + 1) % BUFFER_SIZE
 *
 *   Why faster?  The % operator compiles to a hardware division (or a
 *   software division loop on MCUs like ARM Cortex-M0 that lack a DIV
 *   instruction).  Division takes many clock cycles — unacceptable
 *   inside a tight ISR.  A bitwise AND is a single-cycle instruction
 *   on every processor.
 *
 *   Why it works only for powers of 2:
 *   A power-of-2 value N has exactly one bit set.  (N - 1) turns that
 *   bit off and sets all lower bits to 1, creating a perfect mask.
 *   Example: N = 8 = 0b00001000  →  N-1 = 7 = 0b00000111
 *   ANDing any index with 0b00000111 keeps only the lowest 3 bits,
 *   which is mathematically identical to index % 8.
 *   For non-power-of-2 sizes, (N - 1) does not form a valid mask
 *   and the AND trick produces incorrect wrap-around values.
 */
int ringbuf_write(RingBuf *rb, uint8_t byte)
{
    if (ringbuf_is_full(rb)) {
        return RINGBUF_ERR;
    }

    rb->data[rb->head] = byte;
    rb->head = (rb->head + 1U) & BUFFER_MASK;   /* Bonus: & instead of % */
    rb->count++;

    return RINGBUF_OK;
}

/**
 * @brief Read one byte from the buffer.
 *
 * @param rb   Pointer to the ring buffer.
 * @param byte Pointer where the read byte will be stored.
 * @return     RINGBUF_OK on success, RINGBUF_ERR if the buffer is empty.
 *
 * The read will FAIL if the buffer is empty — garbage data is never
 * returned.
 */
int ringbuf_read(RingBuf *rb, uint8_t *byte)
{
    if (ringbuf_is_empty(rb)) {
        return RINGBUF_ERR;
    }

    *byte = rb->data[rb->tail];
    rb->tail = (rb->tail + 1U) & BUFFER_MASK;   /* Bonus: & instead of % */
    rb->count--;

    return RINGBUF_OK;
}

/**
 * @brief Query how many bytes are currently stored.
 */
uint8_t ringbuf_count(const RingBuf *rb)
{
    return rb->count;
}

/**
 * @brief Check whether the buffer is completely full.
 * @return 1 if full, 0 otherwise.
 */
uint8_t ringbuf_is_full(const RingBuf *rb)
{
    return (rb->count == BUFFER_SIZE) ? 1U : 0U;
}

/**
 * @brief Check whether the buffer is completely empty.
 * @return 1 if empty, 0 otherwise.
 */
uint8_t ringbuf_is_empty(const RingBuf *rb)
{
    return (rb->count == 0U) ? 1U : 0U;
}

/* ------------------------------------------------------------------ */
/*  Demonstration — main()                                             */
/* ------------------------------------------------------------------ */

int main(void)
{
    RingBuf rb;
    uint8_t byte_out;
    int     status;

    ringbuf_init(&rb);

    printf("=== Ring Buffer Demo (capacity = %u bytes) ===\n\n", BUFFER_SIZE);

    /* -------------------------------------------------------------- */
    /* Step 1: Write 8 bytes (0x41 .. 0x48) — fill the buffer         */
    /* -------------------------------------------------------------- */
    printf("--- Step 1: Write 8 bytes to fill the buffer ---\n");
    {
        const uint8_t fill_data[] = {
            0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48
        };
        uint8_t i;
        for (i = 0; i < BUFFER_SIZE; i++) {
            status = ringbuf_write(&rb, fill_data[i]);
            if (status == RINGBUF_OK) {
                printf("[WRITE] 0x%02X -> OK  (count=%u)%s\n",
                       fill_data[i],
                       ringbuf_count(&rb),
                       ringbuf_is_full(&rb) ? " FULL" : "");
            } else {
                printf("[WRITE] 0x%02X -> FAIL (buffer full)\n",
                       fill_data[i]);
            }
        }
        printf("Buffer full? %s | Count = %u\n\n",
               ringbuf_is_full(&rb) ? "YES" : "NO",
               ringbuf_count(&rb));
    }

    /* -------------------------------------------------------------- */
    /* Step 2: Attempt one more write (0x99) — must fail               */
    /* -------------------------------------------------------------- */
    printf("--- Step 2: Attempt write when full ---\n");
    status = ringbuf_write(&rb, 0x99);
    if (status == RINGBUF_ERR) {
        printf("[WRITE] 0x99 -> FAIL (buffer full)\n\n");
    } else {
        printf("[WRITE] 0x99 -> OK  (count=%u)\n\n", ringbuf_count(&rb));
    }

    /* -------------------------------------------------------------- */
    /* Step 3: Read 3 bytes — expect 0x41, 0x42, 0x43                  */
    /* -------------------------------------------------------------- */
    printf("--- Step 3: Read 3 bytes ---\n");
    {
        uint8_t i;
        for (i = 0; i < 3; i++) {
            status = ringbuf_read(&rb, &byte_out);
            if (status == RINGBUF_OK) {
                printf("[READ]        -> 0x%02X  (count=%u)\n",
                       byte_out, ringbuf_count(&rb));
            } else {
                printf("[READ]  (empty) -> FAIL (buffer empty)\n");
            }
        }
        printf("Count = %u\n\n", ringbuf_count(&rb));
    }

    /* -------------------------------------------------------------- */
    /* Step 4: Write 3 new bytes (0x49, 0x4A, 0x4B)                    */
    /*         The 3 freed slots are reused (wrap-around).             */
    /* -------------------------------------------------------------- */
    printf("--- Step 4: Write 3 bytes into freed slots ---\n");
    {
        const uint8_t refill_data[] = { 0x49, 0x4A, 0x4B };
        uint8_t i;
        for (i = 0; i < 3; i++) {
            status = ringbuf_write(&rb, refill_data[i]);
            if (status == RINGBUF_OK) {
                printf("[WRITE] 0x%02X -> OK  (count=%u)%s\n",
                       refill_data[i],
                       ringbuf_count(&rb),
                       ringbuf_is_full(&rb) ? " FULL" : "");
            } else {
                printf("[WRITE] 0x%02X -> FAIL (buffer full)\n",
                       refill_data[i]);
            }
        }
        printf("Count = %u\n\n", ringbuf_count(&rb));
    }

    /* -------------------------------------------------------------- */
    /* Step 5: Read all 8 remaining bytes                              */
    /* -------------------------------------------------------------- */
    printf("--- Step 5: Read all remaining bytes ---\n");
    while (!ringbuf_is_empty(&rb)) {
        status = ringbuf_read(&rb, &byte_out);
        if (status == RINGBUF_OK) {
            printf("[READ]        -> 0x%02X  (count=%u)\n",
                   byte_out, ringbuf_count(&rb));
        }
    }
    printf("Buffer empty? %s | Count = %u\n\n",
           ringbuf_is_empty(&rb) ? "YES" : "NO",
           ringbuf_count(&rb));

    /* -------------------------------------------------------------- */
    /* Step 6: Attempt read from empty buffer — must fail               */
    /* -------------------------------------------------------------- */
    printf("--- Step 6: Attempt read when empty ---\n");
    status = ringbuf_read(&rb, &byte_out);
    if (status == RINGBUF_ERR) {
        printf("[READ]  (empty) -> FAIL (buffer empty)\n");
    } else {
        printf("[READ]        -> 0x%02X  (count=%u)\n",
               byte_out, ringbuf_count(&rb));
    }

    printf("\n=== Demo Complete ===\n");

    return 0;
}
