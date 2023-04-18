#ifndef PROTOCOL_H
#define PROTOCOL_H

#define FIELD 256
#define BOARD 9

typedef struct
{
    int size;
    char type = "PLAY";
    char name[FIELD];
} PLAY;

typedef struct
{
} WAIT;

typedef struct
{
    int size;
    char role;
    char name[FIELD];
} BEGN;

typedef struct
{
    int size;
    char role;
    int x;
    int y;
} MOVE;

typedef struct
{
    int size;
    char role;
    int x;
    int y;
    char board[BOARD];
} MOVD;

typedef struct
{
    int size;
    char reason[FIELD];
} INVL;

typedef struct
{
} RSGN;

typedef struct
{
    int size;
    char message;
} DRAW;

typedef struct
{
    int size;
    char outcome;
    char reason[FIELD];
} OVER;

#endif