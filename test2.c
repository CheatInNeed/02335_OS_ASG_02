// test2.c
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include "aq.h"

static AlarmQueue Q;

static void *receiverA(void *arg) {
    (void)arg;
    void *m = NULL;
    int k = aq_recv(Q, &m);
    printf("[RA] kind=%d val=%ld (expect ALARM 999)\n", k, (long)(intptr_t)m);
    assert(k == AQ_ALARM && (long)(intptr_t)m == 999);
    return NULL;
}

static void *receiverB(void *arg) {
    (void)arg;
    void *m = NULL; int k;

    k = aq_recv(Q, &m);
    printf("[RB] kind=%d val=%ld (expect NORMAL 10)\n", k, (long)(intptr_t)m);
    assert(k == AQ_NORMAL && (long)(intptr_t)m == 10);

    k = aq_recv(Q, &m);
    printf("[RB] kind=%d val=%ld (expect NORMAL 11)\n", k, (long)(intptr_t)m);
    assert(k == AQ_NORMAL && (long)(intptr_t)m == 11);

    return NULL;
}

int main(void) {
    Q = aq_create();
    if (!Q) { fprintf(stderr, "create failed\n"); return 1; }

    // Start two receivers; both will block initially
    pthread_t tA, tB;
    pthread_create(&tA, NULL, receiverA, NULL);
    pthread_create(&tB, NULL, receiverB, NULL);

    usleep(150 * 1000);

    // Enqueue normals first (while receivers are waiting)
    assert(aq_send(Q, (void*)(intptr_t)10, AQ_NORMAL) == 0);
    assert(aq_send(Q, (void*)(intptr_t)11, AQ_NORMAL) == 0);

    // Now enqueue an alarm; the next wake should deliver the alarm first
    assert(aq_send(Q, (void*)(intptr_t)999, AQ_ALARM) == 0);

    pthread_join(tA, NULL);
    pthread_join(tB, NULL);

    printf("[END] size=%d alarms=%d\n", aq_size(Q), aq_alarms(Q));
    assert(aq_size(Q) == 0 && aq_alarms(Q) == 0);
    return 0;
}
