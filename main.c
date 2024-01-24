#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "common.h"
#include "process.h"

int CACHE_TIME = 30; // seconds

typedef struct {
    int days;
    int hours;
    int minutes;
    int seconds;
    int total;
} RelativeTime;

RelativeTime get_reltime(const char *filename) {
    struct stat fileStat;
    RelativeTime reltime = {0, 0, 0, 0, -1};
    if (stat(filename, &fileStat) != 0) {
        perror("Error getting file information");
        return reltime;
    }
    // Get modification time
    time_t modifiedTime = fileStat.st_mtime;

    // Get current time
    time_t currentTime = time(NULL);

    // Calculate time difference
    time_t timeDiff = difftime(currentTime, modifiedTime);

    // Convert time difference to human-readable format
    reltime.days = timeDiff / (60 * 60 * 24);
    reltime.hours = (timeDiff % (60 * 60 * 24)) / (60 * 60);
    reltime.minutes = (timeDiff % (60 * 60)) / 60;
    reltime.seconds = timeDiff % 60;
    reltime.total = timeDiff;

    return reltime;
}

void read_file(FileRead file_obj) {
    const char* filename = file_obj.name;
    string dst = file_obj.data;

    FILE *file = fopen(filename, "r");
    int filefd = fileno(file);
    ssize_t bytes_read = read(filefd,
            dst.data,
            dst.size);
    if (bytes_read == -1) {
        perror("Error opening file");
        return;
    }
    dst.data[dst.size] = '\0';
    fclose(file);
}

void write_file(FileWrite file_obj) {
    const char* filename = file_obj.name;
    const char* src = file_obj.data;

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error");
        return;
    }
    fprintf(file, "%s", src);
    fclose(file);
}

// Store content if cache is invalid
void store_oninvalid(FileWrite file,
        RelativeTime reltime){

    if (reltime.total < CACHE_TIME) {
        return;
    }
    write_file(file);
}

void check_battery(string s){
    char buffer0[1024];
    argvT argv = {"Battery::Status", (char*) NULL};
    argvT argv1 = {"sed", "-n", "s/.*\"percentage\": \\([0-9]*\\).*/\\1/p", (char*) NULL};
    string buffer = {buffer0, sizeof(buffer0)};
    spawn_out("termux-battery-status",
            argv, buffer);
    spawn("sed", argv1, buffer, s);
}

void read_or_regen(FileRead file) {
    RelativeTime reltime = get_reltime(file.name);

    if ((reltime.total < CACHE_TIME) && (reltime.total >= 0)) {
        read_file(file);
        return;
    }

    check_battery(file.data);
    FileWrite filew = {file.name, file.data.data};
    write_file(filew);
}

int main(){
    char buff[1024];
    char filename[1024];
    char *fn = "/.cache/battery";
    char *home = getenv("HOME");
    
    if (sizeof(filename) < strlen(home)+1) {
        fprintf(stderr,
                "Path '%s' is too long!\n", home);
        return EXIT_FAILURE;
    }
    strncpy(filename, home, sizeof(filename));
    if (sizeof(filename) < (
                strlen(filename) + strlen(fn) + 1)) {
        fprintf(stderr,
                "Final size of filename is too long!\n");
        return EXIT_FAILURE;
    }
    strncat(filename, fn, (sizeof(filename) - strlen(filename)));
    string s = {buff, 1024};
    FileRead fr = {filename, s};
    read_or_regen(fr);
    printf("%s", buff);
    return 0;
}
