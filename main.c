

#include <time.h>

#include "snake/snake.h"


/******************************************************************************/
/**< console functions */
void console_set_size(HANDLE hnd, short xsize, short ysize)
{
  COORD console_sz;
  console_sz.X = xsize;
  console_sz.Y = ysize;
  SetConsoleScreenBufferSize(hnd, console_sz);
}
void console_set_canvasSize(HANDLE hnd, short xsize, short ysize)
{
  SMALL_RECT canvas_sz = {0};
  canvas_sz.Right = xsize;
  canvas_sz.Bottom = ysize;
  SetConsoleWindowInfo(hnd, 1, &canvas_sz);
}
void console_cls(HANDLE hnd)
{
  DWORD length,written;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  COORD topLeft = {0};
  SetConsoleCursorPosition(hnd, topLeft);

  /* std::cout uses a buffer to batch writes to the underlying console. */
  /* We need to flush that to the console because we're circumventing */
  /* std::cout entirely; after we clear the console, we don't want */
  /* stale buffered text to randomly be written out. */
  /*std::cout.flush(); */
  /*FlushConsoleInputBuffer(hnd); */

  /* Figure out the current width and height of the console window */
  if (!GetConsoleScreenBufferInfo(hnd, &csbi))
    abort();

  /* Flood-fill the console with spaces to clear it */
  length = csbi.dwSize.X * csbi.dwSize.Y;
  FillConsoleOutputCharacter(hnd, TEXT(' '), length, topLeft, &written);

  /* Reset the attributes of every character to the default. */
  /* This clears all background colour formatting, if any. */
  FillConsoleOutputAttribute(hnd, csbi.wAttributes, length, topLeft, &written);

  /* Move the cursor back to the top left for the next sequence of writes*/
  SetConsoleCursorPosition(hnd, topLeft);
  /* Black and white */
  SetConsoleTextAttribute(hnd, 0xf);
}

void console_replace(HANDLE hnd)
{
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  COORD topLeft = { 0, 0 };
  SetConsoleCursorPosition(hnd, topLeft);
  if (!GetConsoleScreenBufferInfo(hnd, &csbi)) abort();
  SetConsoleCursorPosition(hnd, topLeft);
}



/* snake */
#define SNAKE_MAX 512
static sprite_t aSNAKE[SNAKE_MAX];

/* mouses */
#define MOUSE_MAX 50
static sprite_t aMOUSE[MOUSE_MAX];


/**/
int main(void)
{
  int i,val;
  char score[32] = {0};
  buffer_t buf;
  keypad_t keys;
  sprite_t *head = aSNAKE+0;

  /* setup handle for console */
  HANDLE hnd = GetStdHandle(STD_OUTPUT_HANDLE);
  srand((unsigned int)time(NULL));
  SetConsoleTitle("Snake");
  console_set_size(hnd, 100,100);
  console_set_canvasSize(hnd, 79,40);

  /* buffer setup */
  if (0 != buffer_new(&buf,hnd, 80,40, 1))
    return EXIT_FAILURE;


  /* Main loop */
  while (1) {

    /* Init */
    console_cls(hnd);
    buffer_clear(&buf);
    memset(aSNAKE, 0, sizeof(aSNAKE));
    memset(aMOUSE, 0, sizeof(aMOUSE));

    *head = sprite(10, 10, '®', 0x6c, 5);

    head->keypad_hook = &keys;
    for (i=0 ; i<MOUSE_MAX; i++)
      aMOUSE[i] = sprite_randInBuffer('©', &buf);

    /* keypad setup */
    keys = keypad(3);


    /* Game loop */
    while (head->alive) {
      keypad_process(&keys);


      for (i = 2 ; i < head->len ; i++)
        if (aSNAKE[i].alive  && head->x == aSNAKE[i].x && head->y == aSNAKE[i].y) {
          head->alive = 0;
          break;
        }

      for (i = head->len ; i >= 1 ; i--) {
        sprite_process_fill(aSNAKE + i - 1, &buf);
        if (0 == aSNAKE[i].alive)
          aSNAKE[i] = sprite(aSNAKE[i - 1].x, aSNAKE[i - 1].y, '#', 0x2a, 0);
        aSNAKE[i].x = aSNAKE[i -1].x;
        aSNAKE[i].y = aSNAKE[i -1].y;
      }

      /* eat a mouse ? */
      for (i = 0 ; i < MOUSE_MAX; i++)
          if (aMOUSE[i].alive && head->x == aMOUSE[i].x && head->y == aMOUSE[i].y) {
            aMOUSE[i].alive = 0;
            head->len += aMOUSE[i].len;
            if (head->len >= SNAKE_MAX) head->alive = 0;
          }

      /* mouse process */
      for (i = 0 ; i < MOUSE_MAX; i++) {
        if (0 == aMOUSE[i].alive)
          aMOUSE[i] = sprite_randInBuffer('©', &buf);
        sprite_process_fill(aMOUSE + i, &buf);
      }


      buffer_fillPlainQuad(&buf, ' ', 0x88, 1,37, 78,3);
      sprintf(score,"Score: %d", head->len);
      text_fill(score, &buf, 2,38, 0x8f);

      Sleep(50);
      console_replace(hnd);
      buffer_update(&buf);
      buffer_clear(&buf);
      buffer_swap(&buf);
    }

    while (1) {
      console_cls(hnd);
      if (SNAKE_MAX == head->len) printf("\n CONGRATULATION YOU FINISH THE GAME!\n\n");
      else printf("\n    GAME OVER!\n\n");
      printf("    Result: %d\n\n\n Play again [y/n] ?\n\n", head->len);
      printf(" \n\n\n\n    by 0xR4nD\n");
      val = getchar();
      if ('y' == val || 'Y' == val) break;
      else if (val == 'n' || 'N' == val) goto end;
    }

  }

  end:
  console_cls(hnd);
  buffer_delete(&buf);
  return EXIT_SUCCESS;
}


