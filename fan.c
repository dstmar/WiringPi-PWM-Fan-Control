/* Hardware PWM fan control */
/* CGI client */
/* https://github.com/dstmar/WiringPi-PWM-Fan-Control */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int port = 8089;     /* Port used by wpi server */

/* Print error message and exit */
void error(char *message)
{
  printf("Error : %s\n", message);
  exit(1);
}

int main (void)
{
  struct sockaddr_in serv_addr;
  char buffer[256];
  int client_sock, n;

  printf("Content-Type:text/plain\n\n");

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    error("opening socket");
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_port = htons(port);
  if (connect(sock,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    error("connecting");
  send(sock, "1", 1, 0);
  while ((n = recv(sock, buffer, 255, 0)) > 0)
  {
    buffer[n] = 0;
    printf("%s", buffer);
  }
  printf("\n");
  return 0;
}
