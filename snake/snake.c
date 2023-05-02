

#include "snake.h"







/******************************************************************************/

/**
 * set a keypad structure
 * @param dir direction of the snake (top:0 bottom:1 left:2 rigth:3)
 * @param a keypad_t structure
 */
keypad_t keypad(int dir)
{
  keypad_t k;
  memset(&k,0,sizeof(keypad_t));
  k.last = dir;
  return k;
}


/******************************************************************************/

/**
 * keypad functions
 * @param k keypad to check
 */
void keypad_process(keypad_t *k)
{
  if (GetAsyncKeyState(38) & 0x8000) { if (k->last!=1) k->last = 0;}
  if (GetAsyncKeyState(40) & 0x8000) { if (k->last!=0) k->last = 1;}
  if (GetAsyncKeyState(37) & 0x8000) { if (k->last!=3) k->last = 2;}
  if (GetAsyncKeyState(39) & 0x8000) { if (k->last!=2) k->last = 3;}
}


/******************************************************************************/

/**
 * create a new buffer
 * @param buf this
 * @param hnd handle on the console
 * @param width buffer width (!= console width)
 * @param height buffer height (!= console height)
 * @param pitch we can add extra chars on right (ex: '\n')
 * @return if(!=0) == error
 */
int buffer_new(buffer_t *buf, HANDLE hnd, int width, int height, int pitch)
{
  int j;
  /* assert */
  if (NULL == buf) return -1;
  if (width <8 || height<8 || (width-pitch)<8) return -2;

  memset(buf,0,sizeof(buffer_t));
  buf->front = malloc((width+pitch)*height * 4);
  if (NULL == buf->front) return -3;
  memset(buf->front, 0, (width+pitch)*height * 4);
  buf->back = buf->front + (width+pitch)*height;
  buf->cfront = (unsigned char*)buf->back + (width+pitch)*height;
  buf->cback = buf->cfront + (width+pitch)*height;
  buf->cur = buf->front;
  buf->ccur = buf->cfront;
  buf->width = width;
  buf->height = height;
  buf->pitch = pitch;
  buf->hnd = hnd;

  /* fix the front buffer pitch with '\n' char */
  memset(buf->front, ' ', (width+pitch)*height * 2);
  for (j=0; j<(height*2)-1; j++) buf->front[j*(width + pitch) + width] = '\n';
  buf->front[(height-1)*(width + pitch) + width] = '\0';
  buf->back[(height-1)*(width + pitch) + width] = '\0';
  return 0;
}


/******************************************************************************/

/**
 * delete a buffer
 * @param buf this
 */
void buffer_delete(buffer_t *buf)
{
  if (NULL == buf) return;
  free(buf->front);
  buf->front = NULL;
  buf->back = NULL;
  buf->cur = NULL;
  buf->cfront = NULL;
  buf->cback = NULL;
  buf->ccur = NULL;
}


/******************************************************************************/

/**
 * clear the current sub-buffer
 * @param buf this
 */
void buffer_clear(buffer_t *buf)
{
  int i,j,offset;

  /* SetConsoleTextAttribute(buf->hnd, 0); */
  for (j=0; j<buf->height-3; j++)
    for (i=0; i<buf->width; i++) {
      offset = j*(buf->width + buf->pitch) + i;
      if (i==0 || i==buf->width-1 || j==0 || j==buf->height-4) {
        buf->cur[offset] = 'X';
        buf->ccur[offset] = 0x6e;
      } else {
        buf->cur[offset] = ' ';
        buf->ccur[offset] = 0xf0;
      }
    }

}


/******************************************************************************/

/**
 * clear the current sub-buffer
 * @param buf this
 */
void buffer_fillPlainQuad(buffer_t *buf, char tex, unsigned char col,
                          int x, int y, int width, int height)
{
  int i,j,offset;

  for (j=y; j<height+y; j++)
    for (i=x; i<width+x; i++) {
      offset = j*(buf->width + buf->pitch) + i;
      buf->cur[offset] = tex;
      buf->ccur[offset] = col;
    }

}



/******************************************************************************/

/**
 * swap front/back buffers
 * @param buf this
 */
void buffer_swap(buffer_t *buf)
{
  if (buf->cur == buf->front) buf->cur = buf->back;
  else buf->cur = buf->front;
  if (buf->ccur == buf->cfront) buf->ccur = buf->cback;
  else buf->ccur = buf->cfront;
}


/******************************************************************************/

/**
 * draw current buffer into console
 * @param buf this
 * @param score current player score
 */
void buffer_update(buffer_t *buf)
{
  int i,j,offset;
  for (j = 0; j < buf->height; j++)
    for (i = 0; i < buf->width + buf->pitch; i++) {
      offset = j*(buf->width + buf->pitch) + i;
      //if (i&1) SetConsoleTextAttribute(buf->hnd, 0xf8);
      //else SetConsoleTextAttribute(buf->hnd, 0xa1);
      SetConsoleTextAttribute(buf->hnd, buf->ccur[offset]);
      printf("%c", buf->cur[offset]);
    }

  //printf("%s\n Score: %d", buf->cur, score);
}


/******************************************************************************/

/**
 * set a sprite
 * @param x xposition
 * @param y yposition
 * @param tex char to draw into the console
 * @param color color of the char into the console
 * @param len length of the sprite (for snake)
 * @return the sprite_t structure
 */
sprite_t sprite(int x, int y, char tex, unsigned char color, int len)
{
  sprite_t spr = {0};
  spr.x = x;
  spr.y = y;
  spr.len = len;
  spr.tex = tex;
  spr.color = color;
  spr.alive = 1;
  return spr;
}


/******************************************************************************/

/**
 * set a random sprite into a buffer
 * @param tex char to draw into the console
 * @param buf the buffer
 * @return the sprite_t structure
 */
sprite_t sprite_randInBuffer(char tex, buffer_t *buf)
{
  int x,y,col,len;

  do {
    x = rand()%(buf->width-2) + 1;
    y = rand()%(buf->height-2) + 1;
  } while (buf->cur[y*(buf->width+buf->pitch) + x] != ' ');

  len = rand()&1;
  col = 0xf7;
  if (1 == len) col = 0xf8;
  return sprite(x,y,tex,col,len+1);
}


/******************************************************************************/

/**
 * draw a sprite into a buffer
 * @param spr this
 * @param buf buffer to draw
 * @return
 */
void sprite_process_fill(sprite_t *spr, buffer_t *out)
{
  static char add[4][2] = { {+0,-1}, {+0,+1}, {-1,+0}, {+1,+0} };
  int offset;
  if (0 == spr->alive) return;

  if (NULL != spr->keypad_hook) {
    spr->x += add[spr->keypad_hook->last][0];
    spr->y += add[spr->keypad_hook->last][1];
  }
  /* wrap */
  if (spr->x < 1) spr->x = out->width-2;
  if (spr->x >= out->width-1) spr->x = 1;
  if (spr->y < 1) spr->y = out->height-5;
  if (spr->y >= out->height-4) spr->y = 1;

  offset = spr->y*(out->width+out->pitch) + spr->x;
  /*if (NULL != spr->keypad_hook) {
    if (out->cur[offset] == spr->tex) return -1;
  }*/

  /*COORD coord = {spr->x,spr->y};
  SetConsoleCursorPosition(out->hnd, coord);
  SetConsoleTextAttribute(out->hnd, spr->color);
  SetConsoleTextAttribute(out->hnd, 0x00f0);
*/

  out->cur[offset] = spr->tex;
  out->ccur[offset] = spr->color;
}





/******************************************************************************/

/**
 * draw a text into a buffer
 * @param str nt-string
 * @param buf buffer to draw
 * @param x  xposition
 * @param y  yposition
 * @param col color
 * @note not clipped
 */
void text_fill(const char *str, buffer_t *out,
               int x, int y, unsigned char col)
{
  int offset = y*(out->width+out->pitch) + x;

  while ('\0' != *str) {
    out->cur[offset] = *str++;
    out->ccur[offset] = col;
    offset++;
  }

}
