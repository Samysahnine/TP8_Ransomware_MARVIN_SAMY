#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 4444
#define KEY_LEN 32
#define IV_LEN 16

unsigned char global_key[KEY_LEN];
unsigned char global_iv[IV_LEN];
int key_iv_set = 0;

void handle_connection(int client_sock) {
    char buffer[1024];
    int n = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        close(client_sock);
        return;
    }
    buffer[n] = '\0';

    if (strncmp(buffer, "SENDKEY:", 8) == 0) {
        memcpy(global_key, buffer + 8, KEY_LEN);
        memcpy(global_iv, buffer + 8 + KEY_LEN, IV_LEN);
        key_iv_set = 1;
    } else if (strncmp(buffer, "EXCUSE:", 7) == 0) {
        char *excuse = buffer + 7;
        if (strlen(excuse) > 20 && key_iv_set) {
            send(client_sock, global_key, KEY_LEN, 0);
            send(client_sock, global_iv, IV_LEN, 0);
        } else {
            char *msg = "Refuse\n";
            send(client_sock, msg, strlen(msg), 0);
        }
    }
    close(client_sock);
}

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sock, 5);

    while (1) {
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
        handle_connection(client_sock);
    }

    close(server_sock);
    return 0;
}
