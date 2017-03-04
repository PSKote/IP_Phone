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

int main(int argc, char*argv[]) 
{


	int sock, t, len;
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
    	pa_simple *s = NULL;
    	int ret = 1;
    	int error;
        uint8_t buf[BUFSIZE];

    	/* Create the recording stream */
    	if (!(s = pa_simple_new(NULL, argv[0], PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) 
	{
        	fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        	goto finish;
    	}


	printf("Start your conversation with %s\n", argv[1]);

    	for (;;) 
	{

        	/* Record some data ... */
        	if (pa_simple_read(s, buf, sizeof(buf), &error) < 0) 
		{
            		fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
            		goto finish;
        	}

		if (send(sock, buf, strlen(buf), 0) == -1) 
		{
			perror("send");
			exit(1);
		}
    	}

    	ret = 0;

	finish:
	close(sock);
    	if (s)
        	pa_simple_free(s);

    	return ret;
}
