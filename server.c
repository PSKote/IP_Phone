/***
 server.c -----
 Play the recorded audio from the file.
 ./server <filename>
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

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

#define BACKLOG 5	/* clients waiting is queue		*/

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


int main(int argc, char*argv[]) {


	int sock, s2, t, len;
	struct sockaddr_in server, client;
	struct sigaction sa;

        uint8_t buf[BUFSIZE];
        ssize_t r;

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

	/* listen to 5 clients */
	if (listen(sock, BACKLOG) == -1) 
	{
		perror("listen");
		exit(1);
	}


    	/* The Sample format to use */
    	static const pa_sample_spec ss = {
        	.format = PA_SAMPLE_S16LE,
        	.rate = 44100,
        	.channels = 2
    	};

    	pa_simple *s = NULL;
    	int ret = 1;
    	int error;

    	/* Create a new playback stream */
    	if (!(s = pa_simple_new(NULL, argv[0], PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) 
	{
        	fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        	goto finish;
    	}

	for(;;) 
	{
		int done, n;
		printf("Waiting for a connection\n");

		/* accept connection fron client */
		t = sizeof(client);
		if ((s2 = accept(sock, (struct sockaddr *)&client, &t)) == -1) {
			perror("accept");
			exit(1);
		}
		printf("Connected. Start conversation\n");

		/* receiving message from client */
		done = 0;
		do 
		  {
			n = recv(s2, buf, BUFSIZE, 0);
			if (n <= 0) 
			{
				if (n < 0) 
					perror("recv");
				done = 1;
			}
			if (!done)
			{
				printf("Receiving\n");

				/* And write it to STDIN */
        			if (loop_write(STDIN_FILENO, buf, sizeof(buf)) != sizeof(buf)) 
				{
            				fprintf(stderr, __FILE__": write() failed: %s\n", strerror(errno));
            				goto finish;
        			}
			}

		  } 
		while (!done);
		close(s2);

//		for (;;) 
//		{
			printf("playing\n");

        		/* Read some data ... */
	       		if ((r = read(STDIN_FILENO, buf, sizeof(buf))) <= 0) 
			{
            			if (r == 0) /* EOF */
	               			break;

            			fprintf(stderr, __FILE__": read() failed: %s\n", strerror(errno));
            			goto finish;
	       		}

        		/* ... and play it */
        		if (pa_simple_write(s, buf, (size_t) r, &error) < 0) 
			{
            			fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
            			goto finish;
        		}
//    		}
	}


    	/* Make sure that every single sample was played */
    	if (pa_simple_drain(s, &error) < 0) 
	{
        	fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(error));
        	goto finish;
    	}

    	ret = 0;

	finish:
    	if (s)
        	pa_simple_free(s);

    	return ret;
}
