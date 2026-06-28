#ifndef POCKETGAME_TYPES_H
#define POCKETGAME_TYPES_H

typedef enum {
  PG_TOKEN_EMPTY = 0,
  PG_TOKEN_BLACK = 1,
  PG_TOKEN_WHITE = 2,
  PG_TOKEN_RED = 3,
  PG_TOKEN_BLUE = 4,
  PG_TOKEN_GREEN = 5,
} PgToken;

typedef enum {
  PG_BUTTON_NONE = -1,
  PG_BUTTON_UP = 0,
  PG_BUTTON_DOWN,
  PG_BUTTON_LEFT,
  PG_BUTTON_RIGHT,
  PG_BUTTON_CONFIRM,
  PG_BUTTON_A,
  PG_BUTTON_B,
  PG_BUTTON_OPTION,
} PgButton;

typedef struct PgCursor PgCursor;

typedef struct {
  int width;
  int height;
  int* cell;
  PgCursor* cursor;
} PgGrid;

struct PgCursor {
  int x;
  int y;
  PgGrid* grid;
};

typedef struct {
  int button;
} PgInput;

typedef void (*PgRoutine)(void);
typedef PgInput (*PgInputReader)(void);
typedef void (*PgDrawHandler)(void);

#endif /* POCKETGAME_TYPES_H */