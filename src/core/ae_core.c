#include "core/ae_core.h"

void ae_arena_init(ae_arena *a, void *buffer, size_t size)
{
    a->base = (uint8_t *)buffer;
    a->size = size;
    a->used = 0;
    a->peak = 0;
}

void *ae_arena_alloc(ae_arena *a, size_t size, size_t align)
{
    if (align == 0 || (align & (align - 1)) != 0) {
        return NULL;
    }
    uintptr_t base = (uintptr_t)a->base;
    uintptr_t aligned = (base + a->used + (align - 1)) & ~(uintptr_t)(align - 1);
    size_t offset = (size_t)(aligned - base);
    if (offset > a->size || size > a->size - offset) {
        return NULL;
    }
    a->used = offset + size;
    if (a->used > a->peak) {
        a->peak = a->used;
    }
    return a->base + offset;
}

void ae_arena_reset(ae_arena *a)
{
    a->used = 0;
}

void ae_events_init(ae_event_queue *q)
{
    q->head = 0;
    q->count = 0;
    q->dropped = 0;
}

ae_err ae_events_push(ae_event_queue *q, ae_event ev)
{
    if (q->count == AE_EVENT_QUEUE_LEN) {
        q->dropped++;
        return AE_ERR_FULL;
    }
    q->items[(q->head + q->count) % AE_EVENT_QUEUE_LEN] = ev;
    q->count++;
    return AE_OK;
}

ae_err ae_events_pop(ae_event_queue *q, ae_event *out)
{
    if (q->count == 0) {
        return AE_ERR_EMPTY;
    }
    *out = q->items[q->head];
    q->head = (uint8_t)((q->head + 1u) % AE_EVENT_QUEUE_LEN);
    q->count--;
    return AE_OK;
}

void ae_timers_init(ae_timers *t)
{
    for (uint32_t i = 0; i < AE_TIMER_SLOTS; i++) {
        t->slots[i].active = false;
    }
}

ae_err ae_timers_start(ae_timers *t, uint32_t id, uint64_t now_ms, uint32_t delay_ms, bool repeat)
{
    ae_timer *slot = NULL;
    for (uint32_t i = 0; i < AE_TIMER_SLOTS; i++) {
        if (t->slots[i].active && t->slots[i].id == id) {
            slot = &t->slots[i];
            break;
        }
        if (!t->slots[i].active && slot == NULL) {
            slot = &t->slots[i];
        }
    }
    if (slot == NULL) {
        return AE_ERR_FULL;
    }
    slot->id = id;
    slot->due_ms = now_ms + delay_ms;
    slot->period_ms = repeat ? delay_ms : 0;
    slot->active = true;
    return AE_OK;
}

void ae_timers_stop(ae_timers *t, uint32_t id)
{
    for (uint32_t i = 0; i < AE_TIMER_SLOTS; i++) {
        if (t->slots[i].active && t->slots[i].id == id) {
            t->slots[i].active = false;
        }
    }
}

void ae_timers_poll(ae_timers *t, uint64_t now_ms, ae_event_queue *q)
{
    for (uint32_t i = 0; i < AE_TIMER_SLOTS; i++) {
        ae_timer *s = &t->slots[i];
        if (!s->active || now_ms < s->due_ms) {
            continue;
        }
        ae_event ev = {AE_EV_TIMER, s->id};
        (void)ae_events_push(q, ev);
        if (s->period_ms == 0) {
            s->active = false;
        } else {
            s->due_ms = now_ms + s->period_ms;
        }
    }
}

void ae_log_init(ae_log *l)
{
    l->next = 0;
    l->written = 0;
}

static void log_putc(ae_log *l, char c)
{
    l->buf[l->next] = c;
    l->next = (l->next + 1u) % AE_LOG_BYTES;
    l->written++;
}

void ae_log_write(ae_log *l, const char *line)
{
    if (line == NULL) {
        return;
    }
    while (*line != '\0') {
        log_putc(l, *line == '\n' ? ' ' : *line);
        line++;
    }
    log_putc(l, '\n');
}

uint32_t ae_log_read(const ae_log *l, char *out, uint32_t cap)
{
    uint32_t stored = l->written < AE_LOG_BYTES ? l->written : AE_LOG_BYTES;
    uint32_t start = (l->next + AE_LOG_BYTES - stored) % AE_LOG_BYTES;
    uint32_t n = 0;
    if (cap == 0) {
        return 0;
    }
    if (stored == AE_LOG_BYTES) {
        /* The first retained line may be cut: skip to the next complete one. */
        while (n < stored && l->buf[(start + n) % AE_LOG_BYTES] != '\n') {
            n++;
        }
        n = n < stored ? n + 1u : 0;
    }
    uint32_t len = 0;
    for (; n < stored && len + 1u < cap; n++) {
        out[len++] = l->buf[(start + n) % AE_LOG_BYTES];
    }
    out[len] = '\0';
    return len;
}
