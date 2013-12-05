/* Note:
 * The race condition (described in UNPv1's solution for exercise 7.4)
 * cause bind() to return EADDRINUSE on MAC OS X 10.9, while make connect()
 * to return EADDRNOTAVAIL on Ubuntu Linux 13.10 x86_64.
 *
 * # Start the TCP echo server on host 10.1.81.55 and ports 7000, 8000
 * $./serv 7000 &
 * $./serv 8000 &
 *
 * # The client bind to 10.1.81.51 and 1500 before connect().
 *
 * $tty
 * /dev/pts/0
 * $./cli 10.1.81.51 1500 10.1.81.55 7000
 *
 * $tty
 * /dev/pts/5
 * $./cli 10.1.81.51 1500 10.1.81.55 8000
 *
 * However, if the tuple (local-addr, local-port, foreign-addr, foreign-port)
 * is not unique, then on Mac OS X 10.9, connect() will return EADDRINUSE;
 * 
 */
#include	"unp.h"

int
main(int argc, char **argv)
{
	int					sockfd, optval;
	struct sockaddr_in	servaddr, cliaddr;
	if (argc != 5)
		err_quit("usage: tcpcli laddr lport srvaddr srvport");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	optval = 1;
	Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval,
											sizeof(optval));
	bzero(&cliaddr, sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons((short int)atoi(argv[2]));
	Inet_pton(AF_INET, argv[1], &cliaddr.sin_addr);

#ifdef __LINUX__
	Bind(sockfd, (const SA *)&cliaddr, sizeof(cliaddr));
#else
	while(bind(sockfd, (const SA *)&cliaddr, sizeof(cliaddr)) < 0 && errno == EADDRINUSE)
		continue;
#endif

	sleep(10); /* Sleep to give another instance to run and call bind() */

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons((short int)atoi(argv[4]));
	Inet_pton(AF_INET, argv[3], &servaddr.sin_addr);

#ifdef __LINUX__
	while(connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0 && errno == EADDRNOTAVAIL)
		continue;
#else
	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
#endif
	str_cli(stdin, sockfd);
	exit(0);
}
