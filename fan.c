/* Hardware PWM fan control */
/* CGI client */
/* https://github.com/dstmar/WiringPi-PWM-Fan-Control */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

const char* host = "127.0.0.1";  /* wpi server */
const uint16_t port = 8089;      /* Port used by wpi server */

/* Print error message and exit */
int error(char *message)
{
  printf("Error : %s\n", message);
  return 1;
}

int main (void)
{
  printf("Content-Type:text/plain\n\n");

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    return error("Create socket");
  
  struct sockaddr_in serv_addr;
  int addr_size = sizeof(serv_addr);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(host);
  serv_addr.sin_port = htons(port);
  
  char* msg = "fan.cgi";
  sendto(sock, msg, strlen(msg), 0, (struct sockaddr*) &serv_addr, addr_size);
  
  int buf_size = 256;
  char buffer[buf_size];
  int n = recvfrom(sock, buffer, buf_size-1, 0, (struct sockaddr *) &serv_addr, &addr_size);
  if (n < 0)
    return error("Receiving message from server");
  
  close(sock);

  buffer[n] = '\0';
  printf("%s\n", buffer);
  
  return 0;
}
