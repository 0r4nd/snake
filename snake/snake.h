#ifndef SNAKE_H_
#define SNAKE_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>


#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************/

/**
 * keypad struct + proto
 */
typedef struct {
  /*unsigned char key[4];  up,down,left,right */
  unsigned char last;
} keypad_t;

keypad_t keypad(int dir);
void keypad_process(keypad_t *k);


/******************************************************************************/

/**
 * buffer struct + proto
 */
typedef struct {
  char *front,*back,*cur;
  unsigned char *cfront,*cback,*ccur; /* color */
  int width,height,pitch;
  HANDLE hnd;
} buffer_t;

int buffer_new(buffer_t *buf, HANDLE hnd, int width, int height, int pitch);
void buffer_delete(buffer_t *buf);
void buffer_clear(buffer_t *buf);
void buffer_fillPlainQuad(buffer_t *buf, char tex, unsigned char col,
                          int x, int y, int width, int height);


void buffer_swap(buffer_t *buf);
void buffer_update(buffer_t *buf);


/******************************************************************************/
/**
 * sprite struct + proto
 */
typedef struct {
  keypad_t *keypad_hook;
  int x,y,len;
  char tex;
  unsigned char color;
  char alive;
} sprite_t;

sprite_t sprite(int x, int y, char tex, unsigned char color, int len);
sprite_t sprite_randInBuffer(char tex, buffer_t *buf);
void sprite_process_fill(sprite_t *spr, buffer_t *out);


/******************************************************************************/
/**
 * text proto
 */
void text_fill(const char *str, buffer_t *out,
               int x, int y, unsigned char col);



#ifdef __cplusplus
}
#endif


#endif /* SNAKE_H_ */


