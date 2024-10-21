#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

// Comparison function for qsort
int compare(const void *a, const void *b) {
    // Cast pointers to dirent pointers
    const struct dirent *entryA = *(const struct dirent **)a;
    const struct dirent *entryB = *(const struct dirent **)b;
    return strcmp(entryA->d_name, entryB->d_name);
}

int containsFlag(const char *arg, char letter) {
    // Check if the argument starts with '-' and contains 'l'
    if (arg[0] == '-' && strchr(arg, letter) != NULL) {
        return 1; // "-l" flag found
    }
    return 0; // No "-l" flag found
}

long getFileSize(const char *filename) {
    // Open the file in binary mode
    FILE *file = fopen(filename, "rb"); 
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    // Move the file pointer to the end of the file
    fseek(file, 0, SEEK_END); 
    // Get the current file pointer position, which is the file size
    long size = ftell(file);  
    // Close the file
    fclose(file);             

    return size;
}

void printHumanReadableSize(long size, int humanReadable) {

    if(humanReadable == 0) { printf("%-6ld bytes\t", size); return; }

    const char *sizes[] = {" B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double humanSize = size;

    // Loop through to find the appropriate unit (KB, MB, GB, etc.)
    while (humanSize >= 1024 && i < 4) {
        humanSize /= 1024.0;
        i++;
    }

    // Print the size with 2 decimal precision
    printf("%-6.2f %s\t", humanSize, sizes[i]);
}

long checkFileTypeAndSize(const char *path) {
    struct stat path_stat;
    
    // Get file or directory information
    if (stat(path, &path_stat) != 0) {
        perror("Error getting file information");
        return 0;
    }

    // Check if it's a regular file
    if (S_ISREG(path_stat.st_mode)) {
       // printf("%s is a file\n", path);
        // printf("File size: %ld bytes\n", path_stat.st_size);
        return path_stat.st_size;
    } 
    // Check if it's a directory
    else if (S_ISDIR(path_stat.st_mode)) {
        // printf("%s is a directory\n", path);
        // Use 4096 as this is the typical size of a linux dir
        return 4096;
    } 
    else {
        // printf("%s is neither a regular file nor a directory\n", path);
        return -1;
    }
}

void getLastModificationTime(const char *filename) {
    struct stat file_stat;

    // Get file attributes
    if (stat(filename, &file_stat) == -1) {
        perror("Error getting file information");
        return;
    }
    // Get the modification time as a string
    char *mod_time = ctime(&file_stat.st_mtime);

    // Remove the newline character at the end
    mod_time[strlen(mod_time) - 1] = '\0';
    // Convert the modification time to a readable format
    // printf("Last modification time of %s: %s", filename, ctime(&file_stat.st_mtime));
    printf("%-30s", mod_time); 
}

int containsLFlag(const char *arg) {
    // Check if the argument starts with '-' and contains 'l'
    if (arg[0] == '-' && strchr(arg, 'l') != NULL) {
        return 1; // "-l" flag found
    }
    return 0; // No "-l" flag found
}

int main(int argc, char *argv[]) {
 
  int debug = 0;

  if (argc < 2) {
        printf("Usage: %s <directory_path> <options>\n", argv[0]);
        return 1;
    }

    const char *dirPath = argv[1];
    
    if ( debug) printf("[DEBUG] argv[1]: %s\n", dirPath);

    DIR *dir = opendir(dirPath);

    if (dir == NULL) {
        perror("Unable to open directory");
        return 1;
    }

    struct dirent *entry;
    struct dirent **entries = NULL;
    int count = 0;
    int capacity = 10;

    // Allocate initial memory for entries
    entries = malloc(capacity * sizeof(struct dirent *));
    if (entries == NULL) {
        perror("Unable to allocate memory");
        closedir(dir);
        return 1;
    }

    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Reallocate memory if needed
        if (count >= capacity) {
            capacity *= 2;
            entries = realloc(entries, capacity * sizeof(struct dirent *));
            if (entries == NULL) {
                perror("Unable to reallocate memory");
                closedir(dir);
                return 1;
            }
        }
        // Allocate memory for the new entry and copy it
        entries[count] = malloc(sizeof(struct dirent) + strlen(entry->d_name) + 1);
        if (entries[count] == NULL) {
            perror("Unable to allocate memory for entry");
            closedir(dir);
            return 1;
        }
        memcpy(entries[count], entry, sizeof(struct dirent) + strlen(entry->d_name) + 1);
        count++;
    }

    closedir(dir);

    // Parse program arguments
    int showLongListing = 0;
    int showHumanReadable = 0;

    for (int i = 2; i < argc; i++) {  // Iterate over arguments
        if (containsFlag(argv[i], 'l')) { showLongListing = 1; } 
        if (containsFlag(argv[i], 'h')) { showHumanReadable = 1; } 
    }


    // Sort the entries alphabetically
    qsort(entries, count, sizeof(struct dirent *), compare);

    // Print the sorted entries
    printf("Contents of directory: %s\n", dirPath);
    for (int i = 0; i < count; i++) {

        const char *filename = entries[i]->d_name;

        if(showLongListing){

            // Print Size
            long size = checkFileTypeAndSize(filename);

            if (size != -1) { printHumanReadableSize(size, showHumanReadable); }

            // Print Last modification time
            getLastModificationTime(filename);

            // Print filename
            printf("%-60s\n", entries[i]->d_name);
        }
        else{
            printf("%-40s\n", entries[i]->d_name);
        }
        
        free(entries[i]); // Free memory allocated for each entry
    }

    free(entries); // Free the array of pointers

    return 0;
}
