/*
** client.c -----
** Sends the audio to server to which it is connected. 
** Implemented using Periodic Scheduling to sample the audio. 
** Sampling rate is 1sec.
** ./client <IP_address> <port_number>
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>		/* file control options 			*/
#include <pulse/simple.h>	/* synchronous playback and recording API */
#include <pulse/error.h>	/* error management */
#include <pulse/gccmacro.h>	/* GCC attribute macros */

#include <stdio.h>	/* standard I/O routines.               */
#include <stdlib.h>	/* rand(), macros, etc.                 */
#include <errno.h>	/* defines integer value for errorno	*/
#include <string.h>	/* handling string operations 		*/
#include <sys/types.h>	/* various type definitions.            */
#include <sys/socket.h>	/* sockaddr structure			*/
#include <netinet/in.h>	/* htons()				*/
#include <arpa/inet.h>	/* defines internet operation		*/
#include <unistd.h>	/* fork()                               */
#include <signal.h>	/* handle signals			*/
#include <sys/time.h>	/* timer				*/

#define BUFSIZE 1024	/* buffer size for data 		*/

int sock;		/* socket 				*/

/* the sample type to use */
static const pa_sample_spec ss = {
	.format = PA_SAMPLE_S16LE,
	.rate = 44100,
	.channels = 2
};
pa_simple *sc = NULL;
int ret = 1;
int error;

/* A simple routine calling UNIX write() in a loop */
static ssize_t loop_write(int fd, const void*data, size_t size) 
{
    	ssize_t ret = 0;

    	while (size > 0) 
	{
        	ssize_t r;

        	if ((r = write(fd, data, size)) < 0)
            		return r;

        	if (r == 0)
            		break;

        	ret += r;
        	data = (const uint8_t*) data + r;
        	size -= (size_t) r;
    	}	

    return ret;
}

void 
periodic_task  (int signum)
{
	char ans[2];
	uint8_t buf[BUFSIZE];
	switch(signum)
	{
		case SIGINT:
		  	if (signum == SIGINT)
		  	{
		    		printf("received SIGINT\n");
		    		printf("Program received a CTRL-C\n");
		    		printf("Terminate Y/N : "); 
		    		scanf("%s", ans);
		    		if (strcmp(ans,"Y") == 0)
		    		{
		       			printf("Exiting ....\n");
		       			close(sock);       
		       			exit(0); 
		    		}
		    		else
		    		{
		       			printf("Continung ..\n");
		    		}
		  	}
			break;
		case SIGVTALRM:
			/* record some data */
			if (pa_simple_read(sc, buf, sizeof(buf), &error) < 0) 
			{
			   	fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
			   	if (sc)
					pa_simple_free(sc);
				exit(1);
			}

			/* writing audio data to socket */
			if (loop_write(sock, buf, sizeof(buf)) != sizeof(buf)) 
			{
			    	fprintf(stderr, __FILE__": write() failed: %s\n", strerror(errno));
			   	if (sc)
					pa_simple_free(sc);
				exit(1);
			}
			break;
	}
}

int main(int argc, char*argv[]) 
{

	struct sigaction sa;
 	struct itimerval timer;

 	/* Install periodic_task  as the signal handler for SIGVTALRM. */
 	memset (&sa, 0, sizeof (sa));
 	sa.sa_handler = &periodic_task ;
 	sigaction (SIGVTALRM, &sa, NULL);

 	/* Configure the timer to expire after 1 sec... */
 	timer.it_value.tv_sec = 0;
 	timer.it_value.tv_usec = 1000;

 	/* ... and every 1 sec after that. */
 	timer.it_interval.tv_sec = 0;
 	timer.it_interval.tv_usec = 1000;

 	/* Start a virtual timer. It counts down whenever this process is    executing. */
 	setitimer (ITIMER_VIRTUAL, &timer, NULL);

     	/* registering the Signal handler */
  	if (signal(SIGINT, periodic_task) == SIG_ERR)
      		printf("\ncan't catch SIGINT\n");


	int t, len;
	struct sockaddr_in client;

	/* creating socket stream */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("socket");
		exit(1);
	}
	printf("Welcome to My Voice Call\n");
	client.sin_family=AF_INET;
        client.sin_port=htons(atoi(argv[2]));		/* port address */
	client.sin_addr.s_addr=inet_addr(argv[1]);	/* IP address   */

	if (connect(sock, (struct sockaddr *)&client, sizeof(client)) == -1) 
	{
		perror("connect");
		exit(1);
	}




    	/* create the recording stream */
    	if (!(sc = pa_simple_new(NULL, argv[0], PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) 
	{
        	fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        	goto finish;
    	}


	printf("Start your conversation with %s\n", argv[1]);

    	for (;;) 
	{
    	}

    	ret = 0;

	finish:
	/* close and free the connection to the server */
    	if (sc)
        	pa_simple_free(sc);

    	return ret;
}
