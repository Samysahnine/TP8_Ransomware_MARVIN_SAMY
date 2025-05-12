#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <openssl/evp.h>

#define SERVER_IP "127.0.0.1"
#define PORT 4444
#define KEY_LEN 32
#define IV_LEN 16

int decrypter_fichier(const char *filepath, const unsigned char *key, const unsigned char *iv) {
    FILE *in = fopen(filepath, "rb");
    if (!in) return -1;

    char outpath[512];
    snprintf(outpath, sizeof(outpath), "%.*s", (int)(strlen(filepath) - 4), filepath);

    FILE *out = fopen(outpath, "wb");
    if (!out) {
        fclose(in);
        return -1;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    unsigned char buffer[1024], outbuf[1040];
    int len, outlen;

    while ((len = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        EVP_DecryptUpdate(ctx, outbuf, &outlen, buffer, len);
        fwrite(outbuf, 1, outlen, out);
    }

    EVP_DecryptFinal_ex(ctx, outbuf, &outlen);
    fwrite(outbuf, 1, outlen, out);

    EVP_CIPHER_CTX_free(ctx);
    fclose(in);
    fclose(out);

    remove(filepath);
    return 0;
}

int main() {
    char excuse[256];
    printf("Entrez une excuse : ");
    fgets(excuse, sizeof(excuse), stdin);
    excuse[strcspn(excuse, "\n")] = 0;

    if (strlen(excuse) <= 20) {
        printf("Excuse trop courte (%lu caracteres). Minimum requis : 21\n", strlen(excuse));
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        return 1;
    }

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "EXCUSE:%s", excuse);
    send(sock, buffer, strlen(buffer), 0);

    unsigned char key[KEY_LEN], iv[IV_LEN];
    int n = recv(sock, key, KEY_LEN, MSG_WAITALL);
    if (n != KEY_LEN) {
        printf("Erreur reception cle\n");
        close(sock);
        return 1;
    }

    n = recv(sock, iv, IV_LEN, MSG_WAITALL);
    if (n != IV_LEN) {
        printf("Erreur reception iv\n");
        close(sock);
        return 1;
    }

    DIR *dir = opendir("TP/Projet");
    struct dirent *entry;
    char fullpath[512];

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".enc")) {
            snprintf(fullpath, sizeof(fullpath), "TP/Projet/%s", entry->d_name);
            decrypter_fichier(fullpath, key, iv);
            printf("-> %s dechiffre.\n", entry->d_name);
        }
    }

    closedir(dir);
    close(sock);
    printf("Dechiffrement termine.\n");
    return 0;
}
