/*
** server.c -----
** When obtained connection from client.
** will reproduce the audio.
** Uses Periodic Scheduling to reproduce the audio received. 
** Sampling rate is 1sec. Bandwidth = 0.685Mbps
** ./server <port_number>
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>		/* file control options 			*/
#include <pulse/simple.h>	/* synchronous playback and recording API 	*/
#include <pulse/error.h>	/* error management 				*/
#include <pulse/gccmacro.h>	/* GCC attribute macros 			*/

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

#include "g711.c"

#define BUFSIZE 1024	/* buffer size for data 		*/

#define BACKLOG 10	/* clients waiting is queue		*/

int s2;			/* socket 				*/
int n;
int sock;

struct rtp_header 
{
	uint8_t v:2;
	uint8_t p:1;
	uint8_t x:1;
	uint8_t cc:4;
	uint8_t m:1;
	uint8_t pt:7;
	uint16_t seqno;
	uint32_t ts;
	uint32_t ssrc;
	uint8_t bufs[BUFSIZE]; 
}rtp_packet;

/* the Sample format to use */
static const pa_sample_spec ss = {
	.format = PA_SAMPLE_S16LE,
	.rate = 44100,
	.channels = 2
};

pa_simple *s = NULL;
int ret = 1;
int error;

void 
periodic_task  (int signum)
{
		
	#if 0
	pa_usec_t latency;

	if ((latency = pa_simple_get_latency(s, &error)) == (pa_usec_t) -1) {
		fprintf(stderr, __FILE__": pa_simple_get_latency() failed: %s\n", pa_strerror(error));
		goto finish;
	}

	fprintf(stderr, "%0.0f usec    \r", (float)latency);
	#endif
	/* receiving message from client */
	n = recv(s2, &rtp_packet, sizeof(rtp_packet), 0);
	if(n == -1)
	{
		perror("recv");
		exit(1);
	}

	printf("Sequence Number is: %d\nSent at time %d\n\n", rtp_packet.seqno, rtp_packet.ts);

	if (pa_simple_write(s, rtp_packet.bufs, sizeof(rtp_packet.bufs), &error) < 0) 
	{
		fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
		if (s)
			pa_simple_free(s);
		exit(1);
	}
}

/* signal Handler */
void 
my_handler_for_sigint(int signumber)
{
  	char ans[2];
  	if (signumber == SIGINT)
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
}

int main(int argc, char*argv[]) 
{


	int t, len;
	struct sockaddr_in server, client;
	struct sigaction sa;
 	struct itimerval timer;

 	/* Install periodic_task  as the signal handler for SIGVTALRM. */
 	memset (&sa, 0, sizeof (sa));
 	sa.sa_handler = &periodic_task ;
 	sigaction (SIGVTALRM, &sa, NULL);

 	/* Configure the timer to expire after 6 sec... */
 	timer.it_value.tv_sec = 0;
 	timer.it_value.tv_usec = 6000;

 	/* ... and every 6 sec after that. */
 	timer.it_interval.tv_sec = 0;
 	timer.it_interval.tv_usec = 6000;

 	/* Start a virtual timer. It counts down whenever this process is    executing. */
 	setitimer (ITIMER_VIRTUAL, &timer, NULL);

	/* creating socket stream */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("socket");
		exit(1);
	}

	server.sin_family = AF_INET;
	server.sin_port=htons(atoi(argv[1]));		/* port number		  */                      
	server.sin_addr.s_addr=htonl(INADDR_ANY); 	/* Connect to any address */

	if (bind(sock, (struct sockaddr *)&server, sizeof(server)) == -1) {
		perror("bind");
		exit(1);
	}

	/* listen to 10 clients */
	if (listen(sock, BACKLOG) == -1) 
	{
		perror("listen");
		exit(1);
	}

     	/* registering the Signal handler */
  	if (signal(SIGINT, my_handler_for_sigint) == SIG_ERR)
    		printf("\ncan't catch SIGINT\n");

    	/* create a new playback stream */
    	if (!(s = pa_simple_new(NULL, argv[0], PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) 
	{
        	fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        	goto finish;
    	}

	for(;;) 
	{
		printf("Waiting for a connection\n");

		/* accept connection fron client */
		t = sizeof(client);
		if ((s2 = accept(sock, (struct sockaddr *)&client, &t)) == -1) {
			perror("accept");
			exit(1);
		}
		printf("Connected. Conversation in progress\n");
	
		for(;;)
		{
		}
	}


    	/* make sure that every single sample was played */
    	if (pa_simple_drain(s, &error) < 0) 
	{
        	fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(error));
        	goto finish;
    	}

    	ret = 0;

	/* close and free the connection to the server */
	finish:
    	if (s)
        	pa_simple_free(s);

    	return ret;

}
