#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>

#include "libhttp.h"

#define MAX_PATH 4096
#define MAX_BUFF 8192

#define min(a, b)   ((a) < (b) ? (a) : (b))
#define max(a, b)   ((a) > (b) ? (a) : (b))

/*
 * Global configuration variables.
 * You need to use these in your implementation of handle_files_request and
 * handle_proxy_request. Their values are set up in main() using the
 * command line arguments (already implemented for you).
 */
int server_port;
char *server_files_directory;
char *server_proxy_hostname;
int server_proxy_port;


void send_request_data(int client_fd, int req_fd) {  
  char* buffer = (char*) malloc(MAX_BUFF + 1);
  ssize_t nread;
  while ((nread = read(req_fd, buffer, MAX_BUFF)) > 0) {
      http_send_data(client_fd, buffer, nread);
  }
  free(buffer);
}

int has_index(const char* path, struct stat *file_stat, char *file_path) {
    int fd;
    char* filename = (char*) malloc(MAX_PATH + 1);
    strcpy(filename, path);
    strcat(filename, "index.html");
    fd = open(filename, O_RDONLY);
    stat(filename, file_stat);
    if (fd != -1) {
      strcpy(file_path, filename);
    }
    free(filename);
    return fd;
}

void http_not_found(int fd) {
  http_start_response(fd, 404);
  http_send_header(fd, "Content-type", "text/html");
  http_send_header(fd, "Server", "httpserver/1.0");
  http_end_headers(fd);
  http_send_string(fd,
      "<center>"
      "<h1>404 Not Found</h1>"
      "</center>");
  return;
}

void http_send_file(int fd, int req_fd, struct stat *file_stat, char *file_path) {
  char file_size[64];
  sprintf(file_size, "%lu", file_stat->st_size);
  if (req_fd != -1) {
    http_start_response(fd, 200);
    http_send_header(fd, "Content-type", http_get_mime_type(file_path));
    http_send_header(fd, "Server", "httpserver/1.0");
    http_send_header(fd, "Content-Length", file_size);
    http_end_headers(fd);
    send_request_data(fd, req_fd);
  } else {
    http_not_found(fd);
  }
}

void http_send_directory(int fd, char *req_dir) {
  DIR *dir = opendir(req_dir);
  struct dirent *dir_entry;
  struct stat file_stat;
  char *full_path = (char*) malloc(MAX_PATH + 1);
  char *entry_link = (char*) malloc(MAX_PATH + 1);

  http_start_response(fd, 200);
  http_send_header(fd, "Content-type", "text/html");
  http_send_header(fd, "Server", "httpserver/1.0");
  http_end_headers(fd);
  http_send_string(fd, "<h2>Index of ");
  http_send_string(fd, req_dir);
  http_send_string(fd, " </h2><br>\n");

  if (dir != NULL) {
    while ((dir_entry = readdir(dir)) != NULL) {
      strcpy(full_path, req_dir);
      if (full_path[strlen(full_path) - 1] != '/') {
        strcat(full_path, "/");
      }
      strcat(full_path, dir_entry->d_name);
      stat(full_path, &file_stat);
      if (S_ISDIR(file_stat.st_mode)) {
        snprintf(entry_link, MAX_PATH, "<a href='%s/'>%s/</a><br>\n", dir_entry->d_name, dir_entry->d_name);
      } else {
        snprintf(entry_link, MAX_PATH, "<a href='./%s'>%s/</a><br>\n", dir_entry->d_name, dir_entry->d_name);
      }
      http_send_string(fd, entry_link);
    }
    closedir(dir);
  }
  free(entry_link);
  free(full_path);
}


/*
 * Reads an HTTP request from stream (fd), and writes an HTTP response
 * containing:
 *
 *   1) If user requested an existing file, respond with the file
 *   2) If user requested a directory and index.html exists in the directory,
 *      send the index.html file.
 *   3) If user requested a directory and index.html doesn't exist, send a list
 *      of files in the directory with links to each.
 *   4) Send a 404 Not Found response.
 */
void handle_files_request(int fd) {

  /* YOUR CODE HERE (Feel free to delete/modify the existing code below) */

  struct http_request *request = http_request_parse(fd);
  if (request == NULL) { 
    http_not_found(fd);
  }

  int req_fd;
  struct stat file_stat;
  char *req_file_path = (char*) malloc(MAX_PATH + 1);

  strcpy(req_file_path, server_files_directory);
  strcat(req_file_path, request->path);
  
  if (stat(req_file_path, &file_stat) == 0) {
    if (S_ISREG(file_stat.st_mode)) {
      req_fd = open(req_file_path, O_RDONLY);
      http_send_file(fd, req_fd, &file_stat, req_file_path);
    } else if (S_ISDIR(file_stat.st_mode)) {
      req_fd = has_index(req_file_path, &file_stat, req_file_path);
      if (req_fd != -1) {
        http_send_file(fd, req_fd, &file_stat, req_file_path);
      } else {
        http_send_directory(fd, req_file_path);
      }
    } else {
      http_not_found(fd);
    }
  } else {
    http_not_found(fd);
  }
  free(req_file_path);
}

/*
 * get a tcp socket connect
 */
int tcp_connect(const char *hostname, const int serv_port) {
  int sock_fd, error_n;
  char serv[32];
  struct addrinfo hints, *res, *res_save;

  bzero(&hints, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  sprintf(serv, "%d", serv_port);
  printf("connect %s %s\n", hostname, serv);

  if ((error_n = getaddrinfo(hostname, serv, &hints, &res)) != 0) {
    fprintf(stderr, "tcp connect error for %s %s: %s\n", 
                hostname, serv, gai_strerror(error_n));
    exit(1);
  }
  res_save = res;

  do {
    sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock_fd < 0) {
      continue;
    }
    if (connect(sock_fd, res->ai_addr, res->ai_addrlen) == 0) {
      break;
    }

    if (close(sock_fd) != 0) {
      fprintf(stderr, "socket close error for %s %s\n", hostname, serv);
      exit(1);
    }
  } while((res = res->ai_next) != NULL);

  if (res == NULL) {
    fprintf(stderr, "tcp connect error for %s %s\n", hostname, serv);
    exit(1);
  }
  freeaddrinfo(res_save);
  return sock_fd;
}

/*
 * Opens a connection to the proxy target (hostname=server_proxy_hostname and
 * port=server_proxy_port) and relays traffic to/from the stream fd and the
 * proxy target. HTTP requests from the client (fd) should be sent to the
 * proxy target, and HTTP responses from the proxy target should be sent to
 * the client (fd).
 *
 *   +--------+     +------------+     +--------------+
 *   | client | <-> | httpserver | <-> | proxy target |
 *   +--------+     +------------+     +--------------+
 */
void handle_proxy_request(int client_fd) {

  /* YOUR CODE HERE */
  int proxy_fd = tcp_connect(server_proxy_hostname, server_proxy_port);
  int maxfdp1, client_eof;
  fd_set rset;
  char *req_buffer = (char*) malloc(MAX_BUFF + 1);
  char *proxy_buffer = (char*) malloc(MAX_BUFF + 1);

  client_eof = 0;
  FD_ZERO(&rset);
  while (1) {
    if (client_eof == 0) {
      FD_SET(client_fd, &rset);
    }
    FD_SET(proxy_fd, &rset);
    maxfdp1 = max(client_fd, proxy_fd) + 1;
    if (select(maxfdp1, &rset, NULL, NULL, NULL) < 0) {
      perror("select error");
      exit(1);
    }

    int nread;
    if (FD_ISSET(client_fd, &rset)) {
      if ((nread = read(client_fd, req_buffer, MAX_BUFF)) == 0) {
        client_eof = 1;
        if (shutdown(client_fd, SHUT_WR) < 0) {
          perror("shutdown client error");
          exit(1);
        }
        FD_CLR(client_fd, &rset);
        continue;
      } else if (nread < 0) {
        perror("read from client error");
        exit(1);
      }
      printf("\n%s\n", req_buffer);
      http_send_data(proxy_fd, req_buffer, nread);
    }

    if (FD_ISSET(proxy_fd, &rset)) {
      if ((nread = read(proxy_fd, proxy_buffer, MAX_BUFF)) == 0) {
        if (client_eof == 1) {
          break;
        } else {
          perror("proxy server has closed");
          exit(1);
        }
      } else if (nread < 0) {
        perror("read from proxy error");
        exit(1);
      } 
      printf("\n%s\n", proxy_buffer);
      http_send_data(client_fd, proxy_buffer, nread);
    }
  }
  free(req_buffer);
  free(proxy_buffer);
  if (close(proxy_fd) < 0) {
    perror("proxy connect close error");
    exit(1);
  }
}

/*
 * Opens a TCP stream socket on all interfaces with port number PORTNO. Saves
 * the fd number of the server socket in *socket_number. For each accepted
 * connection, calls request_handler with the accepted fd number.
 */
void serve_forever(int *socket_number, void (*request_handler)(int)) {

  struct sockaddr_in server_address, client_address;
  size_t client_address_length = sizeof(client_address);
  int client_socket_number;
  pid_t pid;

  *socket_number = socket(PF_INET, SOCK_STREAM, 0);
  if (*socket_number == -1) {
    perror("Failed to create a new socket");
    exit(errno);
  }

  int socket_option = 1;
  if (setsockopt(*socket_number, SOL_SOCKET, SO_REUSEADDR, &socket_option,
        sizeof(socket_option)) == -1) {
    perror("Failed to set socket options");
    exit(errno);
  }

  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(server_port);

  if (bind(*socket_number, (struct sockaddr *) &server_address,
        sizeof(server_address)) == -1) {
    perror("Failed to bind on socket");
    exit(errno);
  }

  if (listen(*socket_number, 1024) == -1) {
    perror("Failed to listen on socket");
    exit(errno);
  }

  printf("Listening on port %d...\n", server_port);

  while (1) {

    client_socket_number = accept(*socket_number,
        (struct sockaddr *) &client_address,
        (socklen_t *) &client_address_length);
    if (client_socket_number < 0) {
      perror("Error accepting socket");
      continue;
    }

    printf("Accepted connection from %s on port %d\n",
        inet_ntoa(client_address.sin_addr),
        client_address.sin_port);

    pid = fork();
    if (pid > 0) {
      close(client_socket_number);
    } else if (pid == 0) {
      // Un-register signal handler (only parent should have it)
      signal(SIGINT, SIG_DFL);
      close(*socket_number);
      request_handler(client_socket_number);
      close(client_socket_number);
      exit(EXIT_SUCCESS);
    } else {
      perror("Failed to fork child");
      exit(errno);
    }
  }

  close(*socket_number);

}

int server_fd;
void signal_callback_handler(int signum) {
  printf("Caught signal %d: %s\n", signum, strsignal(signum));
  printf("Closing socket %d\n", server_fd);
  if (close(server_fd) < 0) perror("Failed to close server_fd (ignoring)\n");
  exit(0);
}

char *USAGE =
  "Usage: ./httpserver --files www_directory/ --port 8000\n"
  "       ./httpserver --proxy inst.eecs.berkeley.edu:80 --port 8000\n";

void exit_with_usage() {
  fprintf(stderr, "%s", USAGE);
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  signal(SIGINT, signal_callback_handler);

  /* Default settings */
  server_port = 8000;
  server_files_directory = (char*) malloc(1024);
  getcwd(server_files_directory, 1024);
  server_proxy_hostname = "inst.eecs.berkeley.edu";
  server_proxy_port = 80;

  void (*request_handler)(int) = handle_files_request;

  int i;
  for (i = 1; i < argc; i++) {
    if (strcmp("--files", argv[i]) == 0) {
      request_handler = handle_files_request;
      free(server_files_directory);
      server_files_directory = argv[++i];
      if (!server_files_directory) {
        fprintf(stderr, "Expected argument after --files\n");
        exit_with_usage();
      }
    } else if (strcmp("--proxy", argv[i]) == 0) {
      request_handler = handle_proxy_request;

      char *proxy_target = argv[++i];
      if (!proxy_target) {
        fprintf(stderr, "Expected argument after --proxy\n");
        exit_with_usage();
      }

      char *colon_pointer = strchr(proxy_target, ':');
      if (colon_pointer != NULL) {
        *colon_pointer = '\0';
        server_proxy_hostname = proxy_target;
        server_proxy_port = atoi(colon_pointer + 1);
      } else {
        server_proxy_hostname = proxy_target;
        server_proxy_port = 80;
      }
    } else if (strcmp("--port", argv[i]) == 0) {
      char *server_port_string = argv[++i];
      if (!server_port_string) {
        fprintf(stderr, "Expected argument after --port\n");
        exit_with_usage();
      }
      server_port = atoi(server_port_string);
    } else if (strcmp("--help", argv[i]) == 0) {
      exit_with_usage();
    } else {
      fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
      exit_with_usage();
    }
  }

  serve_forever(&server_fd, request_handler);

  return EXIT_SUCCESS;
}
