# Mini Git

A simplified version control system implemented in C, demonstrating the fundamental concepts of Git.

Un système de contrôle de version simplifié implémenté en C, démontrant les concepts fondamentaux de Git.

## 🚀 Features / Fonctionnalités

- **Repository initialization** (`init`) - Creates the `.minigit` directory structure
- **Staging area** (`add`) - Adds files to the index for the next commit  
- **Commit creation** (`commit`) - Records snapshots of your project
- **Repository status** (`status`) - Shows staged files
- **Commit history** (`log`) - Views modification history

---

- **Initialisation de dépôt** (`init`) - Crée la structure de répertoires `.minigit`
- **Zone de staging** (`add`) - Ajoute des fichiers à l'index pour le prochain commit
- **Création de commits** (`commit`) - Enregistre des instantanés de votre projet
- **Statut du dépôt** (`status`) - Affiche les fichiers en staging
- **Historique des commits** (`log`) - Visualise l'historique des modifications

## 📋 Prerequisites / Prérequis

- GCC Compiler / Compilateur GCC
- OpenSSL library for SHA-1 hashing / Bibliothèque OpenSSL pour le hachage SHA-1

### Installing dependencies / Installation des dépendances

**Ubuntu/Debian :**
```bash
sudo apt-get install build-essential libssl-dev
```

**macOS :**
```bash
brew install openssl
```

**Fedora/CentOS :**
```bash
sudo dnf install gcc openssl-devel
```

## 🔧 Compilation / Compilation

```bash
gcc -o minigit minigit.c -lssl -lcrypto
```

## 📖 Usage / Utilisation

### Initialize a new repository / Initialiser un nouveau dépôt
```bash
./minigit init
```

### Add files to staging / Ajouter des fichiers au staging
```bash
echo "Hello World!" > hello.txt
./minigit add hello.txt
```

### Check status / Vérifier le statut
```bash
./minigit status
```

### Create a commit / Créer un commit
```bash
./minigit commit "First commit"
```

### View history / Voir l'historique
```bash
./minigit log
```

## 📁 Project structure / Structure du projet

```
your-project/
├── .minigit/
│   ├── objects/          # Object database (files and commits)
│   ├── refs/
│   │   └── heads/
│   │       └── main      # Main branch reference
│   ├── HEAD              # Pointer to current branch
│   └── index             # Staging area
├── minigit.c             # Source code
├── minigit               # Compiled executable
└── your-files...
```

## ⚙️ How it works / Comment ça marche

### Content-addressable storage / Stockage adressé par contenu
Each file and commit is stored in `.minigit/objects/` with a name based on its SHA-1 hash, ensuring data integrity.

Chaque fichier et commit est stocké dans `.minigit/objects/` avec un nom basé sur son hash SHA-1, garantissant l'intégrité des données.

### Staging area (Index) / Zone de staging (Index)
The `.minigit/index` file contains the list of files ready for the next commit, in the format:

Le fichier `.minigit/index` contient la liste des fichiers prêts pour le prochain commit, au format :
```
filename1 hash1
filename2 hash2
```

### Commit format / Format des commits
Commits are stored as text objects containing:

Les commits sont stockés en tant qu'objets texte contenant :
```
message: My commit message
timestamp: 2025-08-01 14:30:25
parent: parent_commit_hash
files:
  file1.txt hash1
  file2.txt hash2
```

### Reference system / Système de références
Branches are simple files containing the HEAD commit hash.

Les branches sont des fichiers simples contenant le hash du commit HEAD.

## 🔍 Complete example / Exemple complet

```bash
# Initialize repository / Initialiser le dépôt
./minigit init

# Create files / Créer des fichiers
echo "# My Project" > README.md
echo "print('Hello World')" > main.py

# Add to staging / Ajouter au staging
./minigit add README.md
./minigit add main.py

# Check status / Vérifier le statut
./minigit status

# Create commit / Créer le commit
./minigit commit "Add initial files"

# Modify a file / Modifier un fichier
echo "print('Hello Mini Git!')" > main.py
./minigit add main.py
./minigit commit "Update message"

# View history / Voir l'historique
./minigit log
```

## 🎯 Git concepts demonstrated / Concepts Git démontrés

- **Cryptographic hashing** - Data integrity via SHA-1 / **Hachage cryptographique** - Intégrité des données via SHA-1
- **Object storage** - Files and commits as immutable objects / **Stockage d'objets** - Fichiers et commits comme objets immutables
- **Staging area** - Commit preparation / **Zone de staging** - Préparation des commits
- **Commit graph** - Parent-child links between commits / **Graphe de commits** - Liens parent-enfant entre commits
- **References** - Pointers to commits (branches) / **Références** - Pointeurs vers les commits (branches)

## ⚠️ Limitations / Limitations

This implementation is educational and does not support:

Cette implémentation est éducative et ne supporte pas :

- Multiple branches and merging / Les branches multiples et le merge
- Network operations (push/pull) / Les opérations réseau (push/pull)
- Object compression / La compression des objets
- Large binary files / Les fichiers binaires volumineux
- Conflict resolution / La gestion des conflits

## 🤝 Contributing / Contribution

This project is designed for educational purposes. Feel free to:

Ce projet est conçu à des fins éducatives. N'hésitez pas à :

- Add new features / Ajouter de nouvelles fonctionnalités
- Improve error handling / Améliorer la gestion d'erreur
- Optimize performance / Optimiser les performances
- Add tests / Ajouter des tests

## 📚 Further reading / Ressources pour aller plus loin

- [Official Git Documentation / Documentation officielle Git](https://git-scm.com/doc)
- [Git Internals](https://git-scm.com/book/en/v2/Git-Internals-Plumbing-and-Porcelain)
- [Building Git - Book on Git implementation / Livre sur l'implémentation de Git](https://shop.jcoglan.com/building-git/)

## 📄 License / Licence

This project is under MIT license. Free to use for educational and personal purposes.

Ce projet est sous licence MIT. Libre d'utilisation à des fins éducatives et personnelles.

---

**Created with ❤️ to understand Git internals / Créé avec ❤️ pour comprendre les entrailles de Git**