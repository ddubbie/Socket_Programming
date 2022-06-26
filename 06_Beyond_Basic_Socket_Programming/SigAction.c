#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

inline static void
InterruptSignalHandler(int signlaType)
{
    puts("Interrupt Received. Exiting program");
    exit(EXIT_FAILURE);
}

int 
main(int argc, char *argv[])
{
    struct sigaction handler;   // Signal handler specification structure
    // Set InterruptSignalHandler() as handler function
    handler.sa_handler = InterruptSignalHandler;

    // Create mask that blocks all signals
    if(sigfillset(&handler.sa_mask) < 0)
    {
        fprintf(stderr, "sigfillset() failed\n");
        exit(EXIT_FAILURE);
    }
    handler.sa_flags = 0;   // No flags

    // Set signal handling for interrupt signal
    if(sigaction(SIGINT, &handler, 0) < 0)
    {
        fprintf(stderr, "sigaction() failed for SIGINT\n");
        exit(EXIT_FAILURE);
    }
    for(;;)
    {
        pause();
    }
    return 0;
}