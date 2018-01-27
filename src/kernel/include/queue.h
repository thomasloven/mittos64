#pragma once

#define _QUEUE_DECL(queue, entry, type) \
  struct queue{ \
    type *first; \
    type *last; \
  }
#define QUEUE_DECL(...) _QUEUE_DECL(__VA_ARGS__)
#define _QUEUE_HEAD(queue, entry, type) \
  struct queue queue
#define QUEUE_HEAD(...) _QUEUE_HEAD(__VA_ARGS__)
#define QUEUE_HEAD_EMPTY(...) _QUEUE_HEAD(__VA_ARGS__) = {0,0}

#define _QUEUE_SPOT(queue, entry, type) \
  type *entry
#define QUEUE_SPOT(...) _QUEUE_SPOT(__VA_ARGS__)

#define _QUEUE_EMPTY(queue, entry, type) \
  (!(queue.last))
#define QUEUE_EMPTY(...) _QUEUE_EMPTY(__VA_ARGS__)
#define QUEUE_NOT_EMPTY(...) (!(_QUEUE_EMPTY(__VA_ARGS__)))

#define _QUEUE_ADD(queue, entry, type, item) \
  if(!queue.last) \
    queue.first = item; \
  else \
    queue.last->entry = item; \
  queue.last = item; \
  item->entry = 0;
#define QUEUE_ADD(...) _QUEUE_ADD(__VA_ARGS__)

#define _QUEUE_DROP(queue, entry, type) \
    if(!(queue.first = queue.first->entry)) \
      queue.last = 0;
#define QUEUE_DROP(...) _QUEUE_DROP(__VA_ARGS__)

#define _QUEUE_PEEK(queue, entry, type) \
    (queue.first)
#define QUEUE_PEEK(...) _QUEUE_PEEK(__VA_ARGS__)
