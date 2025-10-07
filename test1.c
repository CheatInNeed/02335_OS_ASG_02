// test1.c — Task 3: blocking alarm send + alarm priority + FIFO (single consumer)
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>     // usleep
#include <stdint.h>     // intptr_t
#include "aq.h"

static AlarmQueue Q;

static void *receiver(void *arg) {
    (void)arg;
    void *msg = NULL;
    int k;

    // 1) First receive: main sends NORMAL 100 to wake us from empty
    k = aq_recv(Q, &msg);
    printf("[R] kind=%d val=%ld (expect NORMAL 100)\n", k, (long)(intptr_t)msg);
    assert(k == AQ_NORMAL && (long)(intptr_t)msg == 100);

    // 2) Next: ALARM 200 (already present)
    k = aq_recv(Q, &msg);
    printf("[R] kind=%d val=%ld (expect ALARM 200)\n", k, (long)(intptr_t)msg);
    assert(k == AQ_ALARM && (long)(intptr_t)msg == 200);

    // Receiving ALARM 200 should unblock the sender of ALARM 201,
    // so the very next receive should be ALARM 201 (priority over normals).
    k = aq_recv(Q, &msg);
    printf("[R] kind=%d val=%ld (expect ALARM 201)\n", k, (long)(intptr_t)msg);
    assert(k == AQ_ALARM && (long)(intptr_t)msg == 201);

    // 4) Drain remaining normals in FIFO order: 101, then 102
    k = aq_recv(Q, &msg);
    printf("[R] kind=%d val=%ld (expect NORMAL 101)\n", k, (long)(intptr_t)msg);
    assert(k == AQ_NORMAL && (long)(intptr_t)msg == 101);

    k = aq_recv(Q, &msg);
    printf("[R] kind=%d val=%ld (expect NORMAL 102)\n", k, (long)(intptr_t)msg);
    assert(k == AQ_NORMAL && (long)(intptr_t)msg == 102);

    return NULL;
}

static void *alarm_sender(void *arg) {
    (void)arg;

    // Put first alarm (200) — succeeds immediately
    int rc = aq_send(Q, (void*)(intptr_t)200, AQ_ALARM);
    printf("[SA] put alarm 200 rc=%d\n", rc);
    assert(rc == 0);

    // Try to put another alarm (201) — MUST BLOCK until 200 is received
    printf("[SA] attempt alarm 201 (will block until alarm 200 consumed)...\n");
    rc = aq_send(Q, (void*)(intptr_t)201, AQ_ALARM);
    printf("[SA] alarm 201 unblocked rc=%d\n", rc);
    assert(rc == 0);

    return NULL;
}

int main(void) {
    Q = aq_create();
    if (!Q) {
        fprintf(stderr, "create failed\n");
        return 1;
    }

    pthread_t tR, tA;

    // Start the single receiver thread; it will block in aq_recv() initially
    pthread_create(&tR, NULL, receiver, NULL);
    usleep(150 * 1000); // let receiver start and block

    // Send one normal to wake receiver from empty
    int rc = aq_send(Q, (void*)(intptr_t)100, AQ_NORMAL);
    printf("[M] put normal 100 rc=%d\n", rc);
    assert(rc == 0);

    // Start alarm sender: inserts 200, then blocks on 201
    pthread_create(&tA, NULL, alarm_sender, NULL);

    // Give alarm 200 time to be sent; alarm 201 is now blocked
    usleep(200 * 1000);

    // While alarm sender is blocked, normals must NOT block and must queue FIFO
    rc = aq_send(Q, (void*)(intptr_t)101, AQ_NORMAL);
    printf("[M] put normal 101 rc=%d\n", rc);
    assert(rc == 0);

    rc = aq_send(Q, (void*)(intptr_t)102, AQ_NORMAL);
    printf("[M] put normal 102 rc=%d\n", rc);
    assert(rc == 0);

    // Do NOT receive from main — single consumer ensures deterministic order
    pthread_join(tA, NULL);
    pthread_join(tR, NULL);

    printf("[END] size=%d alarms=%d\n", aq_size(Q), aq_alarms(Q));
    assert(aq_size(Q) == 0 && aq_alarms(Q) == 0);
    return 0;
}
