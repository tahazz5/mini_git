#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <openssl/sha.h>

#define MAX_PATH 1024
#define MAX_HASH 41
#define MAX_MESSAGE 256
#define MAX_FILES 100

typedef struct {
    char filename[MAX_PATH];
    char hash[MAX_HASH];
} IndexEntry;

typedef struct {
    char hash[MAX_HASH];
    char message[MAX_MESSAGE];
    char timestamp[32];
    char parent[MAX_HASH];
    IndexEntry files[MAX_FILES];
    int file_count;
} Commit;

// Helper function to create directories
int create_dir(const char *path) {
    return mkdir(path, 0755);
}

// Check if directory exists
int dir_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

// Check if file exists
int file_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
}

// Calculate SHA-1 hash of content
void calculate_hash(const char *content, char *hash_output) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((unsigned char*)content, strlen(content), hash);
    
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf(hash_output + (i * 2), "%02x", hash[i]);
    }
    hash_output[40] = '\0';
}

// Read entire file content
char* read_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) return NULL;
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *content = malloc(length + 1);
    if (content) {
        fread(content, 1, length, file);
        content[length] = '\0';
    }
    
    fclose(file);
    return content;
}

// Write content to file
int write_file(const char *filepath, const char *content) {
    FILE *file = fopen(filepath, "w");
    if (!file) return 0;
    
    fprintf(file, "%s", content);
    fclose(file);
    return 1;
}

// Write object to object store
int write_object(const char *hash, const char *content) {
    char object_path[MAX_PATH];
    snprintf(object_path, sizeof(object_path), ".minigit/objects/%s", hash);
    return write_file(object_path, content);
}

// Read object from object store
char* read_object(const char *hash) {
    char object_path[MAX_PATH];
    snprintf(object_path, sizeof(object_path), ".minigit/objects/%s", hash);
    return read_file(object_path);
}

// Get current HEAD commit hash
char* get_head_commit() {
    if (!file_exists(".minigit/refs/heads/main")) {
        return NULL;
    }
    
    char *content = read_file(".minigit/refs/heads/main");
    if (content) {
        // Remove newline if present
        char *newline = strchr(content, '\n');
        if (newline) *newline = '\0';
    }
    return content;
}

// Initialize repository
void cmd_init() {
    if (dir_exists(".minigit")) {
        printf("Repository already exists!\n");
        return;
    }
    
    // Create directory structure
    create_dir(".minigit");
    create_dir(".minigit/objects");
    create_dir(".minigit/refs");
    create_dir(".minigit/refs/heads");
    
    // Initialize HEAD
    write_file(".minigit/HEAD", "ref: refs/heads/main\n");
    
    // Initialize empty index
    write_file(".minigit/index", "");
    
    printf("Initialized empty repository in .minigit\n");
}

// Read index file
int read_index(IndexEntry *entries) {
    char *content = read_file(".minigit/index");
    if (!content) return 0;
    
    int count = 0;
    char *line = strtok(content, "\n");
    
    while (line && count < MAX_FILES) {
        char *space = strchr(line, ' ');
        if (space) {
            *space = '\0';
            strcpy(entries[count].filename, line);
            strcpy(entries[count].hash, space + 1);
            count++;
        }
        line = strtok(NULL, "\n");
    }
    
    free(content);
    return count;
}

// Write index file
void write_index(IndexEntry *entries, int count) {
    FILE *file = fopen(".minigit/index", "w");
    if (!file) return;
    
    for (int i = 0; i < count; i++) {
        fprintf(file, "%s %s\n", entries[i].filename, entries[i].hash);
    }
    
    fclose(file);
}

// Add file to staging area
void cmd_add(const char *filepath) {
    if (!dir_exists(".minigit")) {
        printf("Not a repository. Run 'init' first.\n");
        return;
    }
    
    if (!file_exists(filepath)) {
        printf("File '%s' not found\n", filepath);
        return;
    }
    
    // Read file content and calculate hash
    char *content = read_file(filepath);
    if (!content) {
        printf("Could not read file '%s'\n", filepath);
        return;
    }
    
    char hash[MAX_HASH];
    calculate_hash(content, hash);
    
    // Write object
    write_object(hash, content);
    
    // Update index
    IndexEntry entries[MAX_FILES];
    int count = read_index(entries);
    
    // Find existing entry or add new one
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].filename, filepath) == 0) {
            strcpy(entries[i].hash, hash);
            found = 1;
            break;
        }
    }
    
    if (!found && count < MAX_FILES) {
        strcpy(entries[count].filename, filepath);
        strcpy(entries[count].hash, hash);
        count++;
    }
    
    write_index(entries, count);
    free(content);
    
    printf("Added '%s' to staging area\n", filepath);
}

// Create commit
void cmd_commit(const char *message) {
    if (!dir_exists(".minigit")) {
        printf("Not a repository. Run 'init' first.\n");
        return;
    }
    
    IndexEntry entries[MAX_FILES];
    int count = read_index(entries);
    
    if (count == 0) {
        printf("Nothing to commit\n");
        return;
    }
    
    // Create commit object
    Commit commit;
    strcpy(commit.message, message);
    
    // Get current timestamp
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(commit.timestamp, sizeof(commit.timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Get parent commit
    char *parent = get_head_commit();
    if (parent) {
        strcpy(commit.parent, parent);
        free(parent);
    } else {
        strcpy(commit.parent, "");
    }
    
    // Copy files
    commit.file_count = count;
    for (int i = 0; i < count; i++) {
        commit.files[i] = entries[i];
    }
    
    // Serialize commit to string
    char commit_content[4096];
    int offset = 0;
    offset += sprintf(commit_content + offset, "message: %s\n", commit.message);
    offset += sprintf(commit_content + offset, "timestamp: %s\n", commit.timestamp);
    offset += sprintf(commit_content + offset, "parent: %s\n", commit.parent);
    offset += sprintf(commit_content + offset, "files:\n");
    
    for (int i = 0; i < commit.file_count; i++) {
        offset += sprintf(commit_content + offset, "  %s %s\n", 
                         commit.files[i].filename, commit.files[i].hash);
    }
    
    // Calculate commit hash and write object
    char commit_hash[MAX_HASH];
    calculate_hash(commit_content, commit_hash);
    write_object(commit_hash, commit_content);
    
    // Update branch reference
    write_file(".minigit/refs/heads/main", commit_hash);
    
    // Clear staging area
    write_file(".minigit/index", "");
    
    printf("Committed changes: %.7s - %s\n", commit_hash, message);
}

// Show repository status
void cmd_status() {
    if (!dir_exists(".minigit")) {
        printf("Not a repository. Run 'init' first.\n");
        return;
    }
    
    printf("Repository Status:\n");
    printf("==============================\n");
    printf("On branch: main\n");
    
    // Show staged files
    IndexEntry entries[MAX_FILES];
    int count = read_index(entries);
    
    if (count > 0) {
        printf("\nStaged files:\n");
        for (int i = 0; i < count; i++) {
            printf("  + %s\n", entries[i].filename);
        }
    }
    
    printf("\n");
}

// Parse commit from content
Commit parse_commit(const char *content) {
    Commit commit;
    memset(&commit, 0, sizeof(commit));
    
    char *line = strtok((char*)content, "\n");
    int in_files = 0;
    
    while (line) {
        if (strncmp(line, "message: ", 9) == 0) {
            strcpy(commit.message, line + 9);
        } else if (strncmp(line, "timestamp: ", 11) == 0) {
            strcpy(commit.timestamp, line + 11);
        } else if (strncmp(line, "parent: ", 8) == 0) {
            strcpy(commit.parent, line + 8);
        } else if (strcmp(line, "files:") == 0) {
            in_files = 1;
        } else if (in_files && strncmp(line, "  ", 2) == 0) {
            char *space = strchr(line + 2, ' ');
            if (space && commit.file_count < MAX_FILES) {
                *space = '\0';
                strcpy(commit.files[commit.file_count].filename, line + 2);
                strcpy(commit.files[commit.file_count].hash, space + 1);
                commit.file_count++;
            }
        }
        line = strtok(NULL, "\n");
    }
    
    return commit;
}

// Show commit history
void cmd_log() {
    if (!dir_exists(".minigit")) {
        printf("Not a repository. Run 'init' first.\n");
        return;
    }
    
    char *commit_hash = get_head_commit();
    if (!commit_hash) {
        printf("No commits yet\n");
        return;
    }
    
    printf("Commit History:\n");
    printf("========================================\n");
    
    int count = 0;
    while (commit_hash && strlen(commit_hash) > 0 && count < 10) {
        char *commit_content = read_object(commit_hash);
        if (!commit_content) break;
        
        Commit commit = parse_commit(commit_content);
        
        printf("Commit: %.7s\n", commit_hash);
        printf("Date: %s\n", commit.timestamp);
        printf("Message: %s\n", commit.message);
        
        if (commit.file_count > 0) {
            printf("Files:\n");
            for (int i = 0; i < commit.file_count; i++) {
                printf("  - %s\n", commit.files[i].filename);
            }
        }
        
        printf("----------------------------------------\n");
        
        free(commit_content);
        free(commit_hash);
        
        // Move to parent commit
        if (strlen(commit.parent) > 0) {
            commit_hash = malloc(strlen(commit.parent) + 1);
            strcpy(commit_hash, commit.parent);
        } else {
            commit_hash = NULL;
        }
        
        count++;
    }
    
    if (commit_hash) free(commit_hash);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <command> [args...]\n", argv[0]);
        printf("Commands: init, add, commit, status, log\n");
        return 1;
    }
    
    const char *command = argv[1];
    
    if (strcmp(command, "init") == 0) {
        cmd_init();
    } else if (strcmp(command, "add") == 0) {
        if (argc < 3) {
            printf("Usage: add <filepath>\n");
            return 1;
        }
        cmd_add(argv[2]);
    } else if (strcmp(command, "commit") == 0) {
        if (argc < 3) {
            printf("Usage: commit <message>\n");
            return 1;
        }
        cmd_commit(argv[2]);
    } else if (strcmp(command, "status") == 0) {
        cmd_status();
    } else if (strcmp(command, "log") == 0) {
        cmd_log();
    } else {
        printf("Unknown command: %s\n", command);
        return 1;
    }
    
    return 0;
}