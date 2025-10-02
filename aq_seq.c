/**
 * @file   aq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Alarm queue skeleton implementation
 */

#include "aq.h"
#include "stdbool.h"
#include "stdlib.h"

struct MsgNode {
  void *payload;
  struct MsgNode *next;
};

struct AlarmQueueStruct {
  struct MsgNode *head;
  struct MsgNode *tail;
  bool hasAlarm;
  void *alarm_payload;
  int size;
};

AlarmQueue aq_create(void) {
  struct AlarmQueueStruct *q = (struct AlarmQueueStruct *)malloc(sizeof(*q));
  if (!q) return NULL;
  q->head = q->tail = NULL;
  q->hasAlarm = false;
  q->alarm_payload = NULL;
  q->size = 0;
  return (AlarmQueue)q;
}

int aq_send( AlarmQueue aq, void * msg, MsgKind k) {
  if (!aq)  return AQ_UNINIT;
  if (!msg) return AQ_NULL_MSG;

  struct AlarmQueueStruct *queue = (struct AlarmQueueStruct *) aq;

  if (k == AQ_ALARM) {
    if (queue->hasAlarm){
      return AQ_NO_ROOM;
    }
    queue->alarm_payload = msg;
    queue->hasAlarm = true;
    queue->size++;
    return 0;
  } else {
    struct MsgNode *newNode = (struct MsgNode*)malloc(sizeof(struct MsgNode));
    if (!newNode) return AQ_NO_ROOM;
    newNode->payload = msg;
    newNode->next = NULL;

    if (queue->tail != NULL) {
      queue->tail->next = newNode;
    } else {
      queue->head = newNode;
    }
    queue->tail = newNode;
    queue->size++;
    return 0;
  }
}

int aq_recv(AlarmQueue aq, void **msg) {
  if (!aq)  return AQ_UNINIT;
  if (!msg) return AQ_NO_MSG;

  struct AlarmQueueStruct *queue = (struct AlarmQueueStruct *) aq;

  if (queue->size == 0) {
    return AQ_NO_MSG;
  }

  if (queue->hasAlarm) {
    *msg = queue->alarm_payload;
    queue->alarm_payload = NULL;
    queue->hasAlarm = false;
    queue->size--;
    return AQ_ALARM;
  } else {
    struct MsgNode *node = queue->head;
    queue->head = node->next;
    if (queue->head == NULL) {
      queue->tail = NULL;
    }
    *msg = node->payload;
    free(node);
    queue->size--;
    return AQ_NORMAL;
  }
}


int aq_size( AlarmQueue aq) {
  struct AlarmQueueStruct *queue = (struct AlarmQueueStruct *) aq;
  return queue->size;
}

int aq_alarms(AlarmQueue aq) {
  struct AlarmQueueStruct *queue = (struct AlarmQueueStruct *) aq;

  if (queue->hasAlarm) {
    return 1;
  }
  return 0;
}



