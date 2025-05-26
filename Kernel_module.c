// Execute a program that will create multiple child processes in which memory is allocated dynamically. While this task is executing, generate an output with the process id and the memory map of each child process in a tree structure. 
// Example: my_kernel_module  <process id of the parent program executing>


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_CHILDREN 3  // Number of child processes

/**
 * Function: allocate_memory
 * -------------------------
 * Allocates 1MB of memory dynamically and fills it with 'A' to ensure it's used.
 * This helps in visualizing memory usage in the process memory map.
 */
void allocate_memory() {
    char *mem = (char *)malloc(1024 * 1024);  // Allocate 1MB dynamically
    if (mem == NULL) {
        perror("Memory allocation failed");
        exit(1);
    }
    for (int i = 0; i < 1024; i++) {
        mem[i] = 'A';  // Ensure memory is actively used
    }
}

/**
 * Function: save_memory_map_to_file
 * ---------------------------------
 * Saves the process's memory map into a file named "mem_map_<pid>.txt".
 * This ensures memory map output is stored correctly, avoiding interleaved output in stdout.
 *
 * @param pid - Process ID whose memory map will be saved.
 */
void save_memory_map_to_file(pid_t pid) {
    char path[256], output_file[256];
    snprintf(path, sizeof(path), "/proc/%d/maps", pid); // Path to process memory map
    snprintf(output_file, sizeof(output_file), "mem_map_%d.txt", pid); // Output file name

    FILE *file = fopen(path, "r");  // Open the memory map file
    FILE *out = fopen(output_file, "w");  // Create output file for writing

    if (!file || !out) {
        perror("Failed to open memory map or output file");
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) { // Read memory map line by line
        fputs(line, out); // Write to output file
    }

    fclose(file);
    fclose(out);
}

/**
 * Function: print_memory_map_from_file
 * ------------------------------------
 * Reads and prints the memory map from a previously saved file.
 * This ensures that memory maps are displayed sequentially without interference.
 *
 * @param pid - Process ID whose memory map file will be read and printed.
 */
void print_memory_map_from_file(pid_t pid) {
    char output_file[256];
    snprintf(output_file, sizeof(output_file), "mem_map_%d.txt", pid); // Construct filename

    FILE *file = fopen(output_file, "r"); // Open saved memory map file
    if (!file) {
        perror("Failed to open saved memory map file");
        return;
    }

    printf("  │   Memory Map of Process <%d>:\n", pid);
    char line[512];
    while (fgets(line, sizeof(line), file)) { // Read and print line by line
        printf("  │   %s", line);
    }
    fclose(file);
}

int main() {
    pid_t parent_pid = getpid();
    printf("my_kernel_module  <%d>\n", parent_pid);

    pid_t children[NUM_CHILDREN];

    // Create child processes
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();

        if (pid == 0) {  // Child process
            allocate_memory(); // Allocate memory
            save_memory_map_to_file(getpid());  // Save memory map to a file
            exit(0);
        } else if (pid > 0) {
            children[i] = pid;  // Store child PID
        } else {
            perror("Fork failed");
            exit(1);
        }
    }

    // Parent waits for all children to finish
    for (int i = 0; i < NUM_CHILDREN; i++) {
        waitpid(children[i], NULL, 0);
    }

    // Print child memory maps after all children exit
    for (int i = 0; i < NUM_CHILDREN; i++) {
        printf("  ├── Child Process <%d>\n", children[i]);
        print_memory_map_from_file(children[i]); // Print child's memory map
    }

    // Print parent's memory map
    printf("  └── Parent Process <%d>\n", parent_pid);
    save_memory_map_to_file(parent_pid);  // Save parent's memory map
    print_memory_map_from_file(parent_pid); // Print parent's memory map

    return 0;
}
