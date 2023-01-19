#include "server.h"
#include <unistd.h>
#include <stdio.h>

#define BUF_SIZE	4096U
#define	PORT		8080U

static server_context_t serv_ctx;

int
main(int argc, char *argv[]) {
	int i;
	int opt;
	int n_cpus;
	int max_concurrency;
	int buf_size;
	unsigned short sport;

	if (argc != 9) {
		fprintf(stderr, "too few args %d\n", argc);
		exit(EXIT_SUCCESS);
	}

	
	while(-1 != (opt = getopt(argc, argv, "n:c:p:b:"))) {
		switch(opt) {
			case 'n' :
				n_cpus = atoi(optarg);
				break;
			case 'c' :
				max_concurrency = atoi(optarg);
				break;
			case 'p' :
				sport = atoi(optarg);
				break;
			case 'b' :
				buf_size = atoi(optarg);
				break;
			default :
				fprintf(stderr, "arg error\n");
				exit(EXIT_SUCCESS);
		}
	}

	SetupServer(&serv_ctx, sport, n_cpus, max_concurrency);
	InitializeWorkerContext(&serv_ctx, buf_size, n_cpus);

	RunServer();

	DestroyWorkerContext();
	TeardownServer(&serv_ctx);
}


