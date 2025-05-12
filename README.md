# TP Final – Ransomware pédagogique

## Objectif

Ce projet simule un ransomware pédagogique pour apprendre les bases :
- du chiffrement AES-256 en mode CBC
- des sockets TCP entre client et serveur
- de la surveillance de fichiers
- de la gestion de délai

---

## Fonctionnement

### 1. ransomware
- Se lance en tâche de fond.
- Attend que le dossier `TP/Projet` soit créé.
- Démarre un minuteur de 1h.
- Une fois le temps écoulé, chiffre tous les fichiers `.txt`, `.md`, `.c` du dossier.
- Crée un fichier `RANCON.txt` avec des instructions.
- Envoie la clé AES + IV au serveur.

### 2. serveur_pardon
- Attend les connexions entrantes.
- Reçoit la clé AES + IV.
- Lorsqu’un client envoie une excuse > 20 caractères, il renvoie la clé et IV.

### 3. client_decrypt
- Se connecte au serveur.
- Envoie un message d’excuse.
- Si l’excuse est acceptée, récupère la clé + IV.
- Déchiffre tous les fichiers `.enc`.

---

## Structure du projet
<pre><code> 📁 TP/ ├── ransomware.c ├── client_decrypt.c ├── serveur_pardon.c ├── README.md └── Projet/ ├── secret.txt ├── password.md └── main.c </code></pre>


---

## Compilation

```bash
gcc ransomware.c -o ransomware -lssl -lcrypto
gcc client_decrypt.c -o client_decrypt -lssl -lcrypto
gcc serveur_pardon.c -o serveur_pardon



---

### 4. (Optionnel) Créer une archive `.zip` pour ton prof

```bash
cd ..
zip -r TP-Final-Ransomware.zip TP-Final-Ransomware

