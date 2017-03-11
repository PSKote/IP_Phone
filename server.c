/*
** server.c -----
** When obtained connection from client.
** will reproduce the audio.
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

#define BUFSIZE 1024	/* buffer size for data 		*/

#define BACKLOG 10	/* clients waiting is queue		*/

int s2;			/* socket 				*/

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
			close(s2);      
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


	int sock, t, len;
	struct sockaddr_in server, client;
	struct sigaction sa;

    	/* the Sample format to use */
    	static const pa_sample_spec ss = {
        	.format = PA_SAMPLE_S16LE,
        	.rate = 44100,
        	.channels = 2
    	};

    	pa_simple *s = NULL;
    	int ret = 1;
    	int error;

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


	/* registering the signal handler */
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
		int n;
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
		
        		uint8_t bufs[BUFSIZE];
        		ssize_t r;

			#if 0
        		pa_usec_t latency;

        		if ((latency = pa_simple_get_latency(s, &error)) == (pa_usec_t) -1) {
            			fprintf(stderr, __FILE__": pa_simple_get_latency() failed: %s\n", pa_strerror(error));
            			goto finish;
        		}

        		fprintf(stderr, "%0.0f usec    \r", (float)latency);
			#endif

			/* receiving message from client */
			n = recv(s2, bufs, sizeof(bufs), 0);
			if(n == -1)
			{
        			perror("recv");
       				exit(1);
			}

        		if (pa_simple_write(s, bufs, sizeof(bufs), &error) < 0) 
			{
            			fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
            			goto finish;
        		}
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
