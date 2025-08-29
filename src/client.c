// I acknowledge all the help I received from my peers, TA, chatgpt for getting
// the entire message, using write, adding the termination, shifting values, and
// debugging. I also referenced these repos and based my code structure off
// them:
// //github.com/npsadafule/CMPT201/re/main/-npsadafule-main
// https://github.com/SFU-CMPT-201/lecture-notes/blob/main/10-network-programming.md
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#define BUF_SIZE 1024

pthread_mutex_t client_id_mutex = PTHREAD_MUTEX_INITIALIZER;

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

typedef struct {
  int message;
  FILE *file;
} fileobj;

int convert(uint8_t *buf, ssize_t buf_size, char *str, ssize_t str_size) {
  if (buf == NULL || str == NULL || buf_size <= 0 ||
      str_size < (buf_size * 2 + 1)) {
    return -1;
  }
  for (int i = 0; i < buf_size; i++)
    sprintf(str + i * 2, "%02X", buf[i]);
  str[buf_size * 2] = '\0';

  return 0;
}
void *handle_message(void *arg) {
  fileobj *gottenmessage = (fileobj *)arg;
  FILE *fileitem = gottenmessage->file;
  int sockitem = gottenmessage->message;
  uint8_t buf[BUF_SIZE];
  ssize_t num_read;

  while (true) {
    ssize_t i = 0;
    while (i < BUF_SIZE - 1) {
      num_read = read(sockitem, buf + i, 1);
      if (num_read <= 0) {
        handle_error("read");
        break;
      }
      if (buf[i] == '\n') {
        i++;
        break;
      }
      i++;
    }

    if (i <= 0) {
      break;
    }

    buf[i - 1] = '\0';

    if (buf[0] == 0) {
      uint32_t ipnum;
      uint16_t portnum;

      memcpy(&ipnum, buf + 1, 4);
      memcpy(&portnum, buf + 5, 2);

      ipnum = ntohl(ipnum);
      portnum = ntohs(portnum);
      // ;
      struct sockaddr_in addr;
      memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = htonl(ipnum);
      addr.sin_port = htons(portnum);

      char iparr[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &(addr.sin_addr), iparr, INET_ADDRSTRLEN);

      fprintf(fileitem, "%-15s%-10u%s\n", iparr, portnum, buf + 7);
      fflush(fileitem);

    }

    else if (buf[0] == 1) {
      printf("Server terminating, bye bye\n");
      break;
    }
  }
  return NULL;
}
int main(int argc, char *argv[]) {
  if (argc < 5 || argc > 5) {
    write(1,
          "Usage: <IP address> <port number> <# of messages> log file"
          "path\n",
          strlen("Usage: <IP address> <port number> <# of messages> log file"
                 "path\n"));
    exit(EXIT_FAILURE);
  }

  char *ip = argv[1];
  int port = atoi(argv[2]);
  int messagenum = atoi(argv[3]);
  FILE *logfile = fopen(argv[4], "w");

  struct sockaddr_in server_addr;
  int serverfd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverfd == -1) {
    handle_error("socket");
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
    handle_error("inet_pton");
  }

  int res =
      connect(serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (res == -1) {
    handle_error("connect");
    close(serverfd);
  }

  pthread_t cid;
  fileobj fileobjitem;
  fileobjitem.message = serverfd;
  fileobjitem.file = logfile;

  if (pthread_create(&cid, NULL, handle_message, &fileobjitem) != 0) {
    handle_error("pthread_create");
  }

  for (int i = 0; i < messagenum; i++) {

    uint8_t randbuf[10];
    char str[10 * 2 + 1];
    getentropy(randbuf, 10);

    if (convert(randbuf, sizeof(randbuf), str, sizeof(str)) != 0) {
      exit(EXIT_FAILURE);
    };

    char mes[BUF_SIZE];
    memset(mes, 0, BUF_SIZE);
    mes[0] = 0;
    memcpy(mes + 1, str, strlen(str));
    mes[strlen(str) + 1] = '\n';

    if (write(serverfd, mes, strlen(str) + 2) == -1) {
      handle_error("write");
      break;
    };
  }

  uint8_t sendmsg1[2] = {1, '\n'};
  printf("Sending message of type 1\n");
  if (write(serverfd, sendmsg1, 2) == -1) {
    handle_error("write");
  }

  pthread_join(cid, NULL);
  fclose(logfile);
  close(serverfd);
  return 0;
}
