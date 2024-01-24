#pragma once

typedef struct {
    char *data;
    int size;
} string;

typedef struct {
    const char *name;
    string data;
} FileRead;

typedef struct {
    const char *name;
    const char *data;
} FileWrite;

typedef const char* const argvT[];

string STR_NULL = {NULL, 0};
