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

AlarmQueue aq_create( ) {
  return (AlarmQueue)malloc(sizeof(struct AlarmQueueStruct));
}

int aq_send( AlarmQueue aq, void * msg, MsgKind k){
  if (msg == NULL) {
    return AQ_NULL_MSG;
  }
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
    newNode->payload = msg;
    newNode->next = NULL;
    if (queue->tail != NULL) { // hvis køen ikke er tom
      queue ->tail->next = newNode;
      queue -> tail = newNode;
    } else {
      queue -> head = newNode;
      queue -> tail = newNode;
    }
}








int aq_recv( AlarmQueue aq, void * * msg) {
  return AQ_NOT_IMPL;
}

int aq_size( AlarmQueue aq) {
  return 0;
}

int aq_alarms( AlarmQueue aq) {
  return 0;
}



