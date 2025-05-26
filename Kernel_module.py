# Execute a program that will create multiple child processes in which memory is allocated dynamically. While this task is executing, generate an output with the process id and the memory map of each child process in a tree structure. 
# Example: my_kernel_module  <process id of the parent program executing>


import os
import mmap
import psutil
import time

def allocate_memory():
    """Allocate some dynamic memory using mmap."""
    size = 1024 * 1024  # 1MB
    mem = mmap.mmap(-1, size)
    mem.write(b'A' * 1024)  # Write some data
    return mem

def get_memory_map(pid):
    """Retrieve and display the memory segments of a process."""
    try:
        process = psutil.Process(pid)
        mappings = process.memory_maps()
        segments = [f"  - {m.path} [{m.rss} bytes]" for m in mappings]
        return segments
    except psutil.NoSuchProcess:
        return ["  - Process exited before memory map retrieval"]

def create_child_processes(num_children=3):
    """Create multiple child processes and display their memory segments in a tree structure."""
    parent_pid = os.getpid()
    print(f"my_kernel_module  <{parent_pid}>")

    children = []

    for _ in range(num_children):
        pid = os.fork()
        if pid == 0:  # Child process
            mem = allocate_memory()
            time.sleep(2)  # Keep the process alive to display memory
            print(f"  ├── Child Process <{os.getpid()}>")
            for segment in get_memory_map(os.getpid()):
                print(f"  │   {segment}")
            mem.close()
            os._exit(0)  # Exit child process
        else:
            children.append(pid)

    for child in children:
        os.waitpid(child, 0)  # Wait for children to terminate

    print(f"  └── Parent Process <{parent_pid}>")
    for segment in get_memory_map(parent_pid):
        print(f"      {segment}")

if __name__ == "__main__":
    create_child_processes(3)
