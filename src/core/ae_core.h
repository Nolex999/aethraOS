/* AETHRA core runtime: errors, arenas, event queue, timers, logging. No heap, no libc I/O. */
#ifndef AETHRA_CORE_H
#define AETHRA_CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    AE_OK = 0,
    AE_ERR_ARG = -1,
    AE_ERR_NOMEM = -2,
    AE_ERR_FULL = -3,
    AE_ERR_EMPTY = -4,
    AE_ERR_UNSUPPORTED = -5,
} ae_err;

/* Bump allocator. Everything is released at once with ae_arena_reset. */
typedef struct {
    uint8_t *base;
    size_t size;
    size_t used;
    size_t peak;
} ae_arena;

void ae_arena_init(ae_arena *a, void *buffer, size_t size);
/* Returns NULL when out of memory. `align` must be a power of two. */
void *ae_arena_alloc(ae_arena *a, size_t size, size_t align);
void ae_arena_reset(ae_arena *a);

typedef enum {
    AE_EV_NONE = 0,
    AE_EV_KEY_DOWN,
    AE_EV_KEY_UP,
    AE_EV_TIMER,
} ae_event_type;

typedef struct {
    ae_event_type type;
    uint32_t code;
} ae_event;

#define AE_EVENT_QUEUE_LEN 16u

typedef struct {
    ae_event items[AE_EVENT_QUEUE_LEN];
    uint8_t head;
    uint8_t count;
    uint32_t dropped; /* events lost to overflow, shown by diagnostics */
} ae_event_queue;

void ae_events_init(ae_event_queue *q);
ae_err ae_events_push(ae_event_queue *q, ae_event ev);
ae_err ae_events_pop(ae_event_queue *q, ae_event *out);

#define AE_TIMER_SLOTS 8u

typedef struct {
    uint64_t due_ms;
    uint32_t period_ms; /* 0 means one-shot */
    uint32_t id;
    bool active;
} ae_timer;

typedef struct {
    ae_timer slots[AE_TIMER_SLOTS];
} ae_timers;

void ae_timers_init(ae_timers *t);
ae_err ae_timers_start(ae_timers *t, uint32_t id, uint64_t now_ms, uint32_t delay_ms, bool repeat);
void ae_timers_stop(ae_timers *t, uint32_t id);
/* Posts one AE_EV_TIMER per expired timer. Missed periods collapse into one event. */
void ae_timers_poll(ae_timers *t, uint64_t now_ms, ae_event_queue *q);

#define AE_LOG_BYTES 2048u

/* Ring of NUL-separated lines. The oldest text is overwritten. */
typedef struct {
    char buf[AE_LOG_BYTES];
    uint32_t next;
    uint32_t written;
} ae_log;

void ae_log_init(ae_log *l);
void ae_log_write(ae_log *l, const char *line);
/* Copies the retained log, oldest first, with '\n' separators. Returns the length. */
uint32_t ae_log_read(const ae_log *l, char *out, uint32_t cap);

#endif
