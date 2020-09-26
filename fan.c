/* Hardware PWM fan control */
/* CGI client */
/* https://github.com/dstmar/WiringPi-PWM-Fan-Control */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
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
  int addr_size = sizeof(serv_addr);
 
  printf("Content-Type:text/plain\n\n");

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    error("Create socket");
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_port = htons(port);
  
  char msg[256] = "fan.cgi";
  char buffer[256];

  sendto(sock, msg, strlen(msg), 0, (struct sockaddr*) &serv_addr, addr_size);
  int n = recvfrom(sock, buffer, 255, 0, (struct sockaddr *) &serv_addr, &addr_size);
  if (n < 0) error("Receiving message from server");
  else
  {
    buffer[n] = '\0';
    printf("%s\n", buffer);
  }
  return 0;
}
