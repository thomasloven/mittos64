#pragma once

// This header file contains macros for making FIFO queues from structures
// A queue is set up via a preprocessor definition
// #define QueueName header_name, spot_name, struct type
// where
// QueueName is the queue identifying name
// header_name is a unique name used for the queue head
// spot_name is the name of the queue placeholder in the struct
// struct type is the type of struct the queue is made from
//
// Each queue requires a declaration
// QUEUE_DECLARE(QueueName);
// and a definition
// QUEUE_DEFINE(QueueName);

#define _QUEUE_DECL(queue, entry, type) \
  struct queue{ \
    type *first; \
    type *last; \
  } queue
#define QUEUE_DECLARE(...) _QUEUE_DECL(__VA_ARGS__)
#define _QUEUE_HEAD(queue, entry, type) \
  struct queue queue = {0, 0}
#define QUEUE_DEFINE(...) _QUEUE_HEAD(__VA_ARGS__)

#define _QUEUE_SPOT(queue, entry, type) \
  type *entry
#define QUEUE_SPOT(...) _QUEUE_SPOT(__VA_ARGS__)

#define _QUEUE_EMPTY(queue, entry, type) \
  (!(queue.last))
#define queue_empty(...) _QUEUE_EMPTY(__VA_ARGS__)
#define queue_not_empty(...) (!(_QUEUE_EMPTY(__VA_ARGS__)))

#define _QUEUE_ADD(queue, entry, type, item) \
  if(!queue.last) \
    queue.first = (item); \
  else \
    queue.last->entry = (item); \
  queue.last = item; \
  (item)->entry = 0;
#define queue_add(...) _QUEUE_ADD(__VA_ARGS__)

#define _QUEUE_DROP(queue, entry, type) \
    if(queue.first && !(queue.first = queue.first->entry)) \
      queue.last = 0;
#define queue_drop(...) _QUEUE_DROP(__VA_ARGS__)

#define _QUEUE_PEEK(queue, entry, type) \
    (queue.first)
#define queue_peek(...) _QUEUE_PEEK(__VA_ARGS__)

#define _QUEUE_POP(queue, entry, type) \
  __extension__({ \
      type *_ret = _QUEUE_PEEK(queue, entry, type); \
   _QUEUE_DROP(queue, entry, type); \
   _ret; \
   })
#define queue_pop(...) _QUEUE_POP(__VA_ARGS__)
