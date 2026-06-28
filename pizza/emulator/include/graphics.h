#ifndef POCKETGAME_GRAPHIC_H
#define POCKETGAME_GRAPHIC_H

#include "types.h"

#define WIDTH 600
#define HEIGHT 600

int pg_graphic_open(int width, int height);
void pg_graphic_close(void);

void pg_graphic_draw_grid(PgGrid* grid);
void pg_graphic_show_message(const char* text);

#endif /* POCKETGAME_GRAPHIC_H */
