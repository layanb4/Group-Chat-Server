#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 1024

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

typedef struct {
  int socket;
  struct sockaddr_in addr;
} clientobj;

// Shared counters for: total # messages, and counter of clients (used for
// assigning client IDs)
int client_counter = 0;
int iwanttoleave = 0;

// Mutexs to protect above global state.
pthread_mutex_t client_id_mutex = PTHREAD_MUTEX_INITIALIZER;

clientobj clientobjarr[100];

/*void handletype0(int event, int currfiledes, char *buffy) {
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(struct sockaddr_in);
  memset(&addr, 0, sizeof(struct sockaddr_in));
  pthread_mutex_lock(&client_id_mutex);
  for (int i = 0; i < total_message_count; i++) {
    // int tmp_cnt = ++total_message_count;
    // if (clientobjarr[i].socket == event) {
    // addr = clientobjarr[i].arr;
    //}
  }
  pthread_mutex_unlock(&client_id_mutex);

  char newerbuffy[100];
  newerbuffy[0] = buffy[0];
  uint32_t ipnum = addr.sin_addr.s_addr;
  uint16_t portnum = addr.sin_port;

  memcpy(newerbuffy + 1, &ipnum, sizeof(uint32_t));
  memcpy(newerbuffy + 5, &portnum, sizeof(uint16_t));
  memcpy(newerbuffy + 7, buffy + 1, sizeof(uint8_t));

  pthread_mutex_lock(&client_id_mutex);
  if (total_message_count == client_counter) {
    for (int i = 0; i < client_counter; i++) {
      // check lab if im writing the correct thing
      if (write(clientobjarr[i].socket, buffy, event) != event) {
        handle_error("write");
      }
    }
  }
  pthread_mutex_unlock(&client_id_mutex);
  // close(client);
}

void handletype1(char *buf, ssize_t num_read, int client) {
  pthread_mutex_lock(&client_id_mutex);
  total_message_count += 1;
  pthread_mutex_unlock(&client_id_mutex);

  pthread_mutex_lock(&client_id_mutex);
  if (total_message_count == client_counter) {
    for (int i = 0; i < client_counter; i++) {
      // check lab if im writing the correct thing
      if (write(clientobjarr[i].socket, buf, num_read) != num_read) {
        handle_error("write");
      }
    }
  }
  pthread_mutex_unlock(&client_id_mutex);
  close(client);
}*/

void *handle_client(void *arg) {

  // TODO: print the message received from client
  // TODO: increase total_message_count per message
  // int cfd = client->cfd;
  // int client_id = client->client_id;
  int client = *(int *)arg;
  free(arg);
  // char buf[BUF_SIZE];
  uint8_t buf[BUF_SIZE];
  int blen = sizeof(buf);

  for (;;) {
    ssize_t num_read = read(client, buf, blen);
    // buf[num_read] = '\0';

    if (num_read == -1) {
      handle_error("read");
    }

    if (num_read < 0 || num_read == 0) {
      if (close(client) == -1) {
        handle_error("close");
      }

      pthread_mutex_lock(&client_id_mutex);
      for (int i = 0; i < client_counter; i++) {
        if (clientobjarr[i].socket == client) {
          for (int j = i; j < client_counter - 1; j++) {
            clientobjarr[j] = clientobjarr[j + 1];
          }
          client_counter--;
          break;
        }
      }
      pthread_mutex_unlock(&client_id_mutex);
      return NULL;
    }

    if (buf[0] == 0) {
      // replace code in function once this actually starts working
      //     handletype0(client, 0, buf);
      struct sockaddr_in addr;
      socklen_t addrlen = sizeof(struct sockaddr_in);
      memset(&addr, 0, sizeof(struct sockaddr_in));

      pthread_mutex_lock(&client_id_mutex);
      for (int i = 0; i < client_counter; i++) {
        if (clientobjarr[i].socket == client) {
          addr = clientobjarr[i].addr;
          break;
        }
      }
      pthread_mutex_unlock(&client_id_mutex);

      // char newerbuffy[100];
      uint8_t newerbuffy[BUF_SIZE];
      newerbuffy[0] = buf[0];
      uint32_t ipnum = addr.sin_addr.s_addr;
      uint16_t portnum = addr.sin_port;

      memcpy(newerbuffy + 1, &ipnum, sizeof(uint32_t));
      memcpy(newerbuffy + 5, &portnum, sizeof(uint16_t));
      memcpy(newerbuffy + 7, buf + 1, num_read - 1);

      pthread_mutex_lock(&client_id_mutex);
      // if (total_message_count == client_counter) {
      for (int i = 0; i < client_counter; i++) {
        // check lab if im writing the correct thing
        if (write(clientobjarr[i].socket, newerbuffy, num_read + 6) == -1) {
          handle_error("write");
          exit(EXIT_FAILURE);
        }
      }
      //}
      pthread_mutex_unlock(&client_id_mutex);
      // close(client);
    }

    else if (buf[0] == 1) {
      pthread_mutex_lock(&client_id_mutex);
      iwanttoleave += 1;
      pthread_mutex_unlock(&client_id_mutex);

      // think about moving it outside
      if (iwanttoleave == client_counter) {
        pthread_mutex_lock(&client_id_mutex);
        for (int i = 0; i < client_counter; i++) {
          // check lab if im writing the correct thing
          if (write(clientobjarr[i].socket, buf, num_read) == -1) {
            handle_error("write");
            exit(EXIT_FAILURE);
            // exit;
          }
        }
        pthread_mutex_unlock(&client_id_mutex);
        if (close(client) == -1) {
          handle_error("close");
        }
        printf("all good to go!!\n");
        exit(EXIT_SUCCESS);
      } else {
        break;
      }
      // close(client);
      //  handletype1(buf, num_read, client);
    }
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <port number> <max clients>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int MAX_EVENTS = atoi(argv[2]);
  // int PORT = atoi(argv[1]);

  int sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(argv[1]));
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // bind to the socket
  if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
    handle_error("bind");
  }

  // listen to the socket
  if (listen(sfd, MAX_EVENTS) == -1) {
    handle_error("listen");
    // close(sfd);
  }

  uint16_t PORT = atoi(argv[1]);
  uint16_t PORT1 = PORT;
  printf("Connecting on port #: %d\n", PORT1);
  for (;;) {
    // look at code example for client from repo to fix client
    struct sockaddr_in client;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    memset(&client, 0, sizeof(struct sockaddr_in));
    int sockaccept = accept(sfd, (struct sockaddr *)&client, &addrlen);

    if (sockaccept == -1) {
      handle_error("accept");
    }

    if (client_counter < MAX_EVENTS) {

      pthread_mutex_lock(&client_id_mutex);
      // possibly replace with function???
      clientobjarr[client_counter].socket = sockaccept;
      clientobjarr[client_counter].addr = client;
      client_counter += 1;
      pthread_mutex_unlock(&client_id_mutex);

      // TODO: create a new thread when a new connection is encountered
      // TODO: call handle_client() when launching a new thread, and provide
      // client_info
      pthread_t tid;
      int *client_t_item = malloc(sizeof(int));
      *client_t_item = sockaccept;

      if (client_t_item == NULL) {
        handle_error("malloc problem");
      }
      if (pthread_create(&tid, NULL, handle_client, client_t_item) == -1) {
        handle_error("pthread_create");
        close(sockaccept);
        close(sfd);

        // free(client_t_item);
      }
      // pthread_detach(tid);
    }

    else {
      if (close(sockaccept) == -1) {
        handle_error("close");
      }
    }
  }

  if (close(sfd) == -1) {
    handle_error("close");
  }

  exit(EXIT_SUCCESS);
}
