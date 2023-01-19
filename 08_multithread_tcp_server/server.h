#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdlib.h>

#define log_error(f, m...) {\
	fprintf(stderr, "[%10s:%4d]\n" f, __FUNCTION__, __LINE__, ##m);\
}

typedef struct connection_s {
	int sockfd;
	unsigned int len;
	void *buf;
	unsigned short buf_size;
	unsigned char to_read : 1;
	unsigned char to_write : 1;
	unsigned char padding : 6;
} connection_t;

typedef struct server_context_s {
	unsigned short sport_number;
	unsigned short server_max_concurrency;
	int n_cpus;
	int listener_fd;
} server_context_t;

typedef struct worker_context_s {
	int thread_no;
	size_t buf_size;
	unsigned short worker_max_concurrency;
	unsigned short worker_cur_concurrency;
	int ep;
	server_context_t *s;
} worker_context_t;

/* SetupServer --> InitializeWorkerContext */

void InitializeWorkerContext(server_context_t *s, const size_t buf_size, const unsigned char n_threads);
void DestroyWorkerContext(void);
void SetupServer(server_context_t *s, const unsigned short sport_number, const int n_cpus, unsigned short server_max_concurrency);
void TeardownServer(server_context_t *s);
void AcceptConnection(const server_context_t *s, worker_context_t *w);
void HandleReadEvent(worker_context_t *w, connection_t *c);
void HandleWriteEvent(worker_context_t *w, connection_t *c);
void CloseConnection(worker_context_t *w, connection_t *c);
void *WorkerMainLoop(void *arg);
void RunServer(void);
#endif
