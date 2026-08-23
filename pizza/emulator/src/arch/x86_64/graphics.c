#include "../../../include/graphics.h"

#include <SDL2/SDL.h>
#include <stdio.h>

#include "../include/pocketgame.h"

static SDL_Window* pg_window = 0;
static SDL_Renderer* pg_renderer = 0;

static int pg_window_width = 640;
static int pg_window_height = 640;

static PgInput pg_graphic_read_input(void);

static int pg_min_int(int a, int b) { return a < b ? a : b; }

int pg_graphic_open(int width, int height) {
  if (width <= 0 || height <= 0) {
    return -1;
  }

  if (pg_window != 0 && pg_renderer != 0) {
    return 0;
  }

  pg_window_width = width;
  pg_window_height = height;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    return -1;
  }

  pg_window = SDL_CreateWindow("PocketGame emulator", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, pg_window_width,
                               pg_window_height, SDL_WINDOW_SHOWN);

  if (pg_window == 0) {
    fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
    SDL_Quit();
    return -1;
  }

  pg_renderer = SDL_CreateRenderer(
      pg_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (pg_renderer == 0) {
    pg_renderer = SDL_CreateRenderer(pg_window, -1, SDL_RENDERER_SOFTWARE);
  }

  if (pg_renderer == 0) {
    fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
  }

  pg_set_input_reader(pg_graphic_read_input);

  return 0;
}

void pg_graphic_close(void) {
  if (pg_renderer != 0) {
    SDL_DestroyRenderer(pg_renderer);
    pg_renderer = 0;
  }

  if (pg_window != 0) {
    SDL_DestroyWindow(pg_window);
    pg_window = 0;
  }

  SDL_Quit();
}

static PgInput pg_graphic_read_input(void) {
  PgInput input = {PG_BUTTON_NONE};
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      pg_stop();
      return input;
    }

    if (event.type != SDL_KEYDOWN) {
      continue;
    }

    switch (event.key.keysym.sym) {
      case SDLK_UP:
      case SDLK_w:
        input.button = PG_BUTTON_UP;
        return input;

      case SDLK_DOWN:
      case SDLK_s:
        input.button = PG_BUTTON_DOWN;
        return input;

      case SDLK_LEFT:
      case SDLK_a:
        input.button = PG_BUTTON_LEFT;
        return input;

      case SDLK_RIGHT:
      case SDLK_d:
        input.button = PG_BUTTON_RIGHT;
        return input;

      case SDLK_RETURN:
      case SDLK_SPACE:
        input.button = PG_BUTTON_CONFIRM;
        return input;

      case SDLK_ESCAPE:
      case SDLK_q:
        pg_stop();
        return input;

      default:
        break;
    }
  }

  SDL_Delay(16);
  return input;
}

static void pg_graphic_set_token_color(int value) {
  switch (value) {
    case PG_TOKEN_EMPTY:
      SDL_SetRenderDrawColor(pg_renderer, 32, 130, 72, 255);
      break;

    case PG_TOKEN_BLACK:
      SDL_SetRenderDrawColor(pg_renderer, 16, 16, 16, 255);
      break;

    case PG_TOKEN_WHITE:
      SDL_SetRenderDrawColor(pg_renderer, 235, 235, 235, 255);
      break;

    case PG_TOKEN_RED:
      SDL_SetRenderDrawColor(pg_renderer, 220, 64, 64, 255);
      break;

    case PG_TOKEN_BLUE:
      SDL_SetRenderDrawColor(pg_renderer, 64, 96, 220, 255);
      break;

    case PG_TOKEN_GREEN:
      SDL_SetRenderDrawColor(pg_renderer, 64, 180, 96, 255);
      break;

    default:
      SDL_SetRenderDrawColor(pg_renderer, 120, 120, 120, 255);
      break;
  }
}

void pg_graphic_draw_grid(PgGrid* grid) {
  if (pg_renderer == 0 || grid == 0) {
    return;
  }

  if (grid->width <= 0 || grid->height <= 0) {
    return;
  }

  int size = pg_min_int(pg_window_width, pg_window_height);
  int max_side = grid->width > grid->height ? grid->width : grid->height;

  if (max_side <= 0) {
    return;
  }

  int cell_size = size / (max_side + 2);

  if (cell_size <= 0) {
    return;
  }

  int board_w = grid->width * cell_size;
  int board_h = grid->height * cell_size;
  int offset_x = (pg_window_width - board_w) / 2;
  int offset_y = (pg_window_height - board_h) / 2;

  SDL_SetRenderDrawColor(pg_renderer, 20, 22, 26, 255);
  SDL_RenderClear(pg_renderer);

  for (int y = 0; y < grid->height; y++) {
    for (int x = 0; x < grid->width; x++) {
      int draw_x = offset_x + x * cell_size;
      int draw_y = offset_y + (grid->height - 1 - y) * cell_size;
      SDL_Rect rect = {draw_x, draw_y, cell_size, cell_size};
      int value = pg_grid_get(grid, x, y);

      pg_graphic_set_token_color(value);
      SDL_RenderFillRect(pg_renderer, &rect);

      SDL_SetRenderDrawColor(pg_renderer, 8, 64, 36, 255);
      SDL_RenderDrawRect(pg_renderer, &rect);
    }
  }

  if (grid->cursor != 0) {
    PgCursor* cursor = grid->cursor;

    if (cursor->x >= 0 && cursor->y >= 0 && cursor->x < grid->width &&
        cursor->y < grid->height) {
      int cursor_x = offset_x + cursor->x * cell_size;
      int cursor_y = offset_y + (grid->height - 1 - cursor->y) * cell_size;

      int inset = cell_size / 8;
      SDL_Rect cursor_rect = {cursor_x + inset, cursor_y + inset,
                              cell_size - inset * 2, cell_size - inset * 2};

      SDL_SetRenderDrawColor(pg_renderer, 255, 220, 64, 255);
      SDL_RenderDrawRect(pg_renderer, &cursor_rect);
    }
  }

  SDL_RenderPresent(pg_renderer);
}

void pg_graphic_show_message(const char* text) {
  if (text == 0) {
    return;
  }

  if (pg_window != 0) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "PocketGame", text,
                             pg_window);
    return;
  }

  printf("%s\n", text);
}
