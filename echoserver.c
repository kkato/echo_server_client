#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <getopt.h>
#include <netdb.h>

#define BUFSIZE 256

#define USAGE                                                        \
  "usage:\n"                                                         \
  "  echoserver [options]\n"                                         \
  "options:\n"                                                       \
  "  -p                  Port (Default: 10823)\n"                    \
  "  -m                  Maximum pending connections (default: 5)\n" \
  "  -h                  Show this help message\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
    {"help", no_argument, NULL, 'h'},
    {"port", required_argument, NULL, 'p'},
    {"maxnpending", required_argument, NULL, 'm'},
    {NULL, 0, NULL, 0}};

int main(int argc, char **argv) {
  int maxnpending = 5;
  int option_char;
  int portno = 10823; /* port to listen on */

  // Parse and set command line arguments
  while ((option_char =
              getopt_long(argc, argv, "hx:m:p:", gLongOptions, NULL)) != -1) {
    switch (option_char) {
      case 'm':  // server
        maxnpending = atoi(optarg);
        break;
      case 'p':  // listen-port
        portno = atoi(optarg);
        break;
      case 'h':  // help
        fprintf(stdout, "%s ", USAGE);
        exit(0);
        break;
      default:
        fprintf(stderr, "%s ", USAGE);
        exit(1);
    }
  }

  setbuf(stdout, NULL);  // disable buffering

  if ((portno < 1025) || (portno > 65535)) {
    fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__,
            portno);
    exit(1);
  }
  if (maxnpending < 1) {
    fprintf(stderr, "%s @ %d: invalid pending count (%d)\n", __FILE__, __LINE__,
            maxnpending);
    exit(1);
  }

  // Socket Code Here
  // Reference: Beek's Guide to the Network Programming
  int sockfd, new_fd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr;
  socklen_t sin_size;
  int yes=1;
  int rv, numbytes;
  int res;
  char buf[BUFSIZE];
  char portnum[6];
  char *hostname = "localhost";

  sprintf(portnum, "%d", portno);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // getaddrinfo
  if ((rv = getaddrinfo(NULL, portnum, &hints, &servinfo)) != 0) {
	  fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	  return 1;
  }

  for (p = servinfo; p != NULL; p = p-> ai_next) {
	  // socket
	  if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
		  perror("server: socket");
		  continue;
	  }

	  // setsockopt
	  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		  perror("setsockopt");
		  exit(1);
	  }

	  // bind
	  if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
		  close(sockfd);
		  perror("server: bind");
		  continue;
	  }
	  break;
  }

  freeaddrinfo(servinfo);

  if (p==NULL) {
	  fprintf(stderr, "server: failed to bind\n");
	  exit(1);
  }

  // listen
  if (listen(sockfd, 10) == -1) {
	perror("server: listen");
	exit(1);
  }

  while(1) {
	  // accept
	  sin_size = sizeof their_addr;
	  new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	  if (new_fd == -1) {
		  perror("server: accept");
		  continue;
	  }

	  // recv
	  numbytes = recv(new_fd, buf, BUFSIZE, 0);
	  if (numbytes == -1) {
		  perror("server: recv");
		  continue;
	  }
	  buf[numbytes] = '\0';

	  // send
	  res = send(new_fd, buf, strlen(buf), 0);
	  if (res == -1) {
		  perror("server: send");
		  continue;
	  }
	  close(new_fd);
  }
  close(sockfd);

}
