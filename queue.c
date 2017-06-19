#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct queue_t {
	struct linked_list_t *head;
	struct linked_list_t* tail;
	pthread_mutex_t mutex;
	pthread_cond_t wakeup;
} queue_t;

typedef struct linked_list_t {
	void *data;
	struct linked_list_t* next;
} linked_list_t;

queue_t* queue_create() {
	struct queue_t* queue = (struct queue_t*)malloc(sizeof(struct queue_t));
	queue->head = NULL;
	queue->tail = NULL;

	queue->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  queue->wakeup = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

  return queue;
}

void queue_push(queue_t* queue, void* x) {
	pthread_mutex_lock(&queue->mutex);

	struct linked_list_t* new_node = (struct linked_list_t*)malloc(sizeof(struct linked_list_t));
	new_node->data = x;
	new_node->next = NULL;

	if (queue->head == NULL && queue->tail == NULL) {
		queue->head = queue->tail = new_node;
	} else {
		queue->tail->next = new_node;
		queue->tail = new_node;
	}

	pthread_mutex_unlock(&queue->mutex);
	pthread_cond_signal(&queue->wakeup);
}

void *queue_pop(queue_t* queue) {
	pthread_mutex_lock(&queue->mutex);

	while(queue->head == NULL) {
		pthread_cond_wait(&queue->wakeup, &queue->mutex);
	}

	struct linked_list_t* current_head = queue->head;
	void* data = current_head->data;
	queue->head = current_head->next;

	if (queue->head == NULL) {
		// no head no tail
		queue->tail == NULL;
	}

	free(current_head);

	pthread_mutex_unlock(&queue->mutex);

	return data;
}

int main() {
	queue_t* my_queue = queue_create();
	char* hello = "hello";
	queue_push(my_queue, hello);
	char* world = "world";
	queue_push(my_queue, world);

	char *result;
	result = queue_pop(my_queue);
	printf("Popped: %s\n", result);

	result = queue_pop(my_queue);
	printf("Popped: %s\n", result);
}