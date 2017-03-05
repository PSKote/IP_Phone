/***
 client.c -----
 save the recorded audio to a file. 
 ./client > <filename>
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>

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

#define BUFSIZE 1024

int sock;

void my_handler_for_sigint(int signumber)//signal handler
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

/* A simple routine calling UNIX write() in a loop */
static ssize_t loop_write(int fd, const void*data, size_t size) {
    ssize_t ret = 0;

    while (size > 0) {
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

int main(int argc, char*argv[]) 
{

     /* Registering the Signal handler */
  if (signal(SIGINT, my_handler_for_sigint) == SIG_ERR)
      printf("\ncan't catch SIGINT\n");

	int t, len;
	struct sockaddr_in client;
	//char str[MAXDATASIZE];		/* buffer for message */

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


   	/* The sample type to use */
    	static const pa_sample_spec ss = {
        	.format = PA_SAMPLE_S16LE,
        	.rate = 44100,
        	.channels = 2
    	};
    	pa_simple *sc = NULL;
    	int ret = 1;
    	int error;

    	/* Create the recording stream */
    	if (!(sc = pa_simple_new(NULL, argv[0], PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) 
	{
        	fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        	goto finish;
    	}


	printf("Start your conversation with %s\n", argv[1]);

    	for (;;) 
	{
	        uint8_t buf[BUFSIZE];

        	/* Record some data ... */
        	if (pa_simple_read(sc, buf, sizeof(buf), &error) < 0) 
		{
            		fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
            		goto finish;
        	}

		if (loop_write(sock, buf, sizeof(buf)) != sizeof(buf)) 
		{
            		fprintf(stderr, __FILE__": write() failed: %s\n", strerror(errno));
           		goto finish;
        	}
    	}

    	ret = 0;

	finish:
    	if (sc)
        	pa_simple_free(sc);

    	return ret;
	close(sock);
}
