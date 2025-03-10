/* queue.c */
#include "queue.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


struct queue *
make_queue ()
{
  // TODO: Allocate memory and initialize synchronization mechanisms
  struct queue *q = malloc (sizeof (struct queue));
  q->front = NULL;
  q->back = NULL;

  if (pthread_mutex_init (&q->lock, NULL) != 0)
    {				// if mutex init failed
      free (q);
      return NULL;
    }
  if (sem_init (&q->items, 0, 0) != 0)
    {				// if semaphore init failed
      pthread_mutex_destroy (&q->lock);
      free (q);
      return NULL;
    }

  return q;
}

void
enqueue (struct queue *q, void *item)
{
  // TODO: Add item to the queue and update necessary pointers
  queue_node_t *new_node = malloc (sizeof (queue_node_t));

  new_node->item = item;
  new_node->prev = NULL;
  new_node->next = NULL;
  pthread_mutex_lock (&q->lock);

  if (q->back)
    {
      q->back->next = new_node;
      new_node->prev = q->back;
    }
  else
    {
      q->front = new_node;
    }

  q->back = new_node;

  pthread_mutex_unlock (&q->lock);

  sem_post (&q->items);
}

void *
dequeue (struct queue *q)
{
  // TODO: Retrieve an item while ensuring thread safety
  sem_wait (&q->items);

  pthread_mutex_lock (&q->lock);

  queue_node_t *node = q->front;
  void *item = node->item;

  q->front = node->next;
  if (q->front)
    {
      q->front->prev = NULL;
    }
  else
    {
      q->back = NULL;
    }

  pthread_mutex_unlock (&q->lock);

  free (node);
  return item;
}

void
destroy_queue (struct queue *q)
{
  // TODO: Clean up allocated resources and synchronization mechanisms
  while (q->front)
    {
      queue_node_t *node = q->front;
      q->front = node->next;
      free (node);
    }

  pthread_mutex_destroy (&q->lock);
  sem_destroy (&q->items);
  free (q);
}
