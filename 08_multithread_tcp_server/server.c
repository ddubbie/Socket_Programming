#include "server.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>

#define READ_FAIL	-1
#define	READ_OK		-2
#define WRITE_FAIL	-1
#define WRITE_OK	-2

static worker_context_t *_worker;
static bool *_run;
static int _n_threads;

static int
_LinuxRead(worker_context_t *w, connection_t *c){
	int len;
	int err;

	do {
		len = read(c->sockfd, c->buf, c->buf_size);
		err = errno;
		if (len < 0) {
			if (err == EAGAIN)	
				return READ_OK;
			else if (err != EINTR)
				return READ_FAIL;
		}
	} while (err == EINTR);

	return len;
}

static int
_LinuxWrite(worker_context_t *w, connection_t *c) {
	int len;
	int err;

	do {
		err = errno;
		len = write(c->sockfd, c->buf, c->len);
		if (len < 0) {
			if (err == EAGAIN)
				return WRITE_OK;
			else if (err != EINTR)
				return WRITE_FAIL;
		}
	} while(err == EINTR);

	return len;
}

static int
_SetSocketNonBlocking(int sockfd) {
	int flags = fcntl(sockfd, F_GETFL);
	flags |= O_NONBLOCK;
	return fcntl(sockfd, F_SETFL, flags);
}

static connection_t *
_CreateConnection(const int sockfd, const unsigned short buf_size) {
	connection_t *c = malloc(sizeof(connection_t));
	if (!c) {
		return NULL;
	}
	c->buf = malloc(buf_size);
	c->buf_size = buf_size;
	if (!c->buf) {
		free(c);
		return NULL;
	}
	return c;
}

static void
_DestroyConnection(connection_t *c) {
	free(c->buf);
	free(c);
}

static void
sigint_handler(int signo) {
	int i;
	for (i = 0; i < _n_threads; i++)
		_run[i] = false;
}

void
InitializeWorkerContext(server_context_t *s, const size_t buf_size, const unsigned char n_threads) {
	int i;
	int ret;
	struct epoll_event ee;

	_worker = malloc(sizeof(worker_context_t) * n_threads);
	if (!_worker) {
		log_error("malloc() error");
		exit(EXIT_FAILURE);
	}
	_run = malloc(sizeof(bool) * n_threads);
	if (!_run) {
		log_error("malloc() error");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < n_threads; i++) {
		_run[i] = true;
		_worker[i].thread_no = i;
		_worker[i].buf_size = buf_size;

		_worker[i].worker_max_concurrency = s->server_max_concurrency / s->n_cpus;
		_worker[i].worker_cur_concurrency = 0;
		_worker[i].s = s;

		_worker[i].ep = epoll_create(1024);
		if (_worker[i].ep < 0) {
			log_error("epoll_create() error");
			exit(EXIT_FAILURE);
		}
		ee.events = EPOLLIN;
		ee.data.ptr = s;
		ret = epoll_ctl(_worker[i].ep, EPOLL_CTL_ADD, s->listener_fd, &ee);
		if (ret < 0) {
			log_error("epoll_ctl() error");
			exit(EXIT_FAILURE);
		}
	}
}

void
DestroyWorkerContext(void) {
	int i;
	for (i = 0; i < _n_threads; i++) {
		close(_worker[i].ep);
	}
}

void
SetupServer(server_context_t *s, const unsigned short sport_number, const int n_cpus, const unsigned short server_max_concurrency) {
	int listener_fd;
	int ret;
	struct sockaddr_in saddr;

	s->sport_number = sport_number;
	listener_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listener_fd < 0) {
		log_error("socket() error");
		exit(EXIT_FAILURE);
	}
	ret = _SetSocketNonBlocking(listener_fd);
	if (ret < 0) {
		log_error("set nonblocking error");
		exit(EXIT_FAILURE);
	}
	memset(&saddr, 0, sizeof(struct sockaddr_in));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(sport_number);

	ret = bind(listener_fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		log_error("bind() error, errno : %d", errno);
		exit(EXIT_FAILURE);
	}
	ret = listen(listener_fd, s->server_max_concurrency);
	if (ret < 0) {
		log_error("listen() error, errno : %d", errno);
		exit(EXIT_FAILURE);
	}

	s->listener_fd = listener_fd;
	s->sport_number = sport_number;
	s->server_max_concurrency = server_max_concurrency;
	s->n_cpus = n_cpus;
	_n_threads = n_cpus;

	signal(SIGINT, sigint_handler);
}

void
TeardownServer(server_context_t *s) {
	close(s->listener_fd);
	free(_run);
}

void
AcceptConnection(const server_context_t *s, worker_context_t *w) {
	int sockfd;
	struct epoll_event ee;
	connection_t *c;
	
	int listener_fd = s->listener_fd;

	if (w->worker_cur_concurrency == w->worker_max_concurrency)
		return;
	sockfd = accept(listener_fd, NULL, NULL);
	if (sockfd < 0) {
		if (errno == EAGAIN)	return;
		else {
			log_error("accept error()");
			return;
		}
	}
	if (_SetSocketNonBlocking(sockfd) < 0) {
		close(sockfd);
		return;
	}
	c = _CreateConnection(sockfd, w->buf_size);
	if (!c) {
		close(sockfd);
		return;
	}

	c->sockfd = sockfd;
	c->to_read = 1;
	c->to_write = 0;

	ee.data.ptr = c;
	ee.events = EPOLLIN;	

	if (epoll_ctl(w->ep, EPOLL_CTL_ADD, sockfd, &ee) < 0) {
		log_error("epoll_ctl() error, errno : %d", errno);
		return;
	}
}
void
CloseConnection(worker_context_t *w, connection_t *c) {
	w->worker_cur_concurrency--;
	if (epoll_ctl(w->ep, EPOLL_CTL_DEL, c->sockfd, NULL) < 0) 
		log_error("epoll_ctl error, errno : %d", errno);
	close(c->sockfd);
	_DestroyConnection(c);
}

void 
HandleReadEvent(worker_context_t *w, connection_t *c) {
	int len;	
	struct epoll_event ee;

	len = _LinuxRead(w, c);
	if (len == READ_FAIL) {
		CloseConnection(w, c);
	} else if(len == READ_OK) {
		return;
	} else if (len == 0) {
		CloseConnection(w, c);
		return;
	}
	c->len = len;
	c->to_read = 0;
	c->to_write = 1;

	ee.events = EPOLLOUT | EPOLLIN;	
	ee.data.ptr = c;

	if (epoll_ctl(w->ep, EPOLL_CTL_MOD, c->sockfd, &ee) < 0) {
		log_error("epoll_ctl error, errno : %d", errno);
		CloseConnection(w, c);
	}
}

void
HandleWriteEvent(worker_context_t *w, connection_t *c) {
	int len;
	struct epoll_event ee;

	len = _LinuxWrite(w, c);
	if (len == WRITE_FAIL) {
		CloseConnection(w, c);
	} else if (len == WRITE_OK) {
		return;
	}
	c->to_read = 1;
	c->to_write = 0;

	ee.events = EPOLLIN;
	ee.data.ptr = c;
	if (epoll_ctl(w->ep, EPOLL_CTL_MOD, c->sockfd, &ee) < 0) {
		log_error("epoll_ctl error, errno : %d", errno);
		CloseConnection(w, c);
	}
}

void *
WorkerMainLoop(void *arg) {
	int i;
	int nevents;
	connection_t *c;

	worker_context_t *w = (worker_context_t *)arg;	

	int n_thread = w->thread_no;
	int ep = w->ep;
	int max_events = w->worker_max_concurrency;
	int listener_fd = w->s->listener_fd;
	void *srv_ctx = w->s;

	struct epoll_event ev[max_events];

	
	while (_run[n_thread]) {
		nevents = epoll_wait(ep, ev, max_events, 5000);
		for (i = 0; i < nevents; i++) {
			if (ev[i].data.ptr == srv_ctx) {
				AcceptConnection(srv_ctx, w);
			} else {
				c = ev[i].data.ptr;	
				if (c->to_read && !c->to_write) 
					HandleReadEvent(w, c);
				else if (!c->to_read && c->to_write)
					HandleWriteEvent(w, c);
			}
		}
	}
	return NULL;
		
}

void
RunServer(void) {
	int i;	
	pthread_t *worker_tid;

	worker_tid = malloc(sizeof(pthread_t) * _n_threads);
	if (!worker_tid) {
		log_error("malloc error");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < _n_threads; i++) {
		pthread_create(&worker_tid[i], NULL, WorkerMainLoop, &_worker[i]);
	}

	for (i = 0; i < _n_threads; i++) {
		pthread_join(worker_tid[i], NULL);
	}

	free(worker_tid);
}
