#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>

// A buffer large enough to contain the longest allowed string 
#define BUFSIZE 256

#define USAGE                                                          \
  "usage:\n"                                                           \
  "  echoclient [options]\n"                                           \
  "options:\n"                                                         \
  "  -s                  Server (Default: localhost)\n"                \
  "  -p                  Port (Default: 10823)\n"                      \
  "  -m                  Message to send to server (Default: \"Hello " \
  "Summer.\")\n"                                                       \
  "  -h                  Show this help message\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
    {"server", required_argument, NULL, 's'},
    {"port", required_argument, NULL, 'p'},
    {"message", required_argument, NULL, 'm'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}};

/* Main ========================================================= */
int main(int argc, char **argv) {
  unsigned short portno = 10823;
  int option_char = 0;
  char *message = "Hello Summer!!";
  char *hostname = "localhost";

  // Parse and set command line arguments
  while ((option_char =
              getopt_long(argc, argv, "p:s:m:hx", gLongOptions, NULL)) != -1) {
    switch (option_char) {
      default:
        fprintf(stderr, "%s", USAGE);
        exit(1);
      case 's':  // server
        hostname = optarg;
        break;
      case 'p':  // listen-port
        portno = atoi(optarg);
        break;
      case 'h':  // help
        fprintf(stdout, "%s", USAGE);
        exit(0);
        break;
      case 'm':  // message
        message = optarg;
        break;
    }
  }

  setbuf(stdout, NULL);  // disable buffering

  if ((portno < 1025) || (portno > 65535)) {
    fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__,
            portno);
    exit(1);
  }

  if (NULL == message) {
    fprintf(stderr, "%s @ %d: invalid message\n", __FILE__, __LINE__);
    exit(1);
  }

  if (NULL == hostname) {
    fprintf(stderr, "%s @ %d: invalid host name\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Socket Code Here */
  // Reference: Beej's Guide to Network Programming
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv, numbytes;
  int res;
  char buf[BUFSIZE];
  char portnum[6];

  sprintf(portnum, "%d", portno);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // getaddrinfo
  if ((rv = getaddrinfo(hostname, portnum, &hints, &servinfo)) != 0) {
	  fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	  exit(1);
  }

  for (p = servinfo;  p != NULL; p = p->ai_next) {
	  // socket
	  if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
		  perror("client: socket");
		  continue;
	  }

	  // connect
	  if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
		  perror("client: connect");
		  close(sockfd);
		  continue;
	  }
	  break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL) {
	  fprintf(stderr, "client: failed to connect\n");
	  return 2;
  }

  // send
  res = send(sockfd, message, strlen(message), 0);
  if (res == -1) {
	  perror("client: send");
	  exit(1);
  }

  // recv
  numbytes = recv(sockfd, buf, BUFSIZE, 0);
  if (numbytes == -1) {
	  perror("client: recv");
	  exit(1);
  }
  buf[numbytes] = '\0';
  fprintf(stdout, "%s", buf);
  close(sockfd);
}
