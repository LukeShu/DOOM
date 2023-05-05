#include "doomkeys.h"

#include "doomgeneric.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>

static Display *s_Display = NULL;
static Window s_Window = 0;
static int s_Screen = 0;
static GC s_Gc = 0;
static XImage *s_Image = NULL;

#define KEYQUEUE_SIZE 16

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

static unsigned char *g_Framebuffer;
static unsigned char *s_Framebuffer;
static char *x_Framebuffer;

static unsigned char convertToDoomKey(unsigned int key)
{
    switch (key)
    {
      case XK_Return:
        key = KEY_ENTER;
        break;
      case XK_Escape:
        key = KEY_ESCAPE;
        break;
      case XK_Left:
        key = KEY_LEFTARROW;
        break;
      case XK_Right:
        key = KEY_RIGHTARROW;
        break;
      case XK_Up:
        key = KEY_UPARROW;
        break;
      case XK_Down:
        key = KEY_DOWNARROW;
        break;
      case XK_Control_L:
      case XK_Control_R:
        key = KEY_FIRE;
        break;
      case XK_space:
        key = KEY_USE;
        break;
      case XK_Shift_L:
      case XK_Shift_R:
        key = KEY_RSHIFT;
        break;
      default:
        key = tolower(key);
        break;
    }

    return key;
}

static void addKeyToQueue(int pressed, unsigned int keyCode)
{
    unsigned char key = convertToDoomKey(keyCode);

    unsigned short keyData = (pressed << 8) | key;

    s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
    s_KeyQueueWriteIndex++;
    s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

void DG_Init()
{
    memset(s_KeyQueue, 0, KEYQUEUE_SIZE * sizeof(unsigned short));

    // window creation

    s_Display = XOpenDisplay(NULL);

    s_Screen = DefaultScreen(s_Display);

    int blackColor = BlackPixel(s_Display, s_Screen);
    int whiteColor = WhitePixel(s_Display, s_Screen);

    XSetWindowAttributes attr;
    memset(&attr, 0, sizeof(XSetWindowAttributes));
    attr.event_mask = ExposureMask | KeyPressMask;
    attr.background_pixel = BlackPixel(s_Display, s_Screen);

    int depth = DefaultDepth(s_Display, s_Screen);

    s_Window = XCreateSimpleWindow(s_Display,                    // display
                                   DefaultRootWindow(s_Display), // parent
                                   0,                            // x
                                   0,                            // y
                                   DOOMGENERIC_RESX,             // width
                                   DOOMGENERIC_RESY,             // height
                                   0,                            // border_width
                                   blackColor,                   // border
                                   blackColor);                  // background

    XSelectInput(s_Display, s_Window, StructureNotifyMask | KeyPressMask | KeyReleaseMask);

    XMapWindow(s_Display, s_Window);

    s_Gc = XCreateGC(s_Display, s_Window, 0, NULL);

    XSetForeground(s_Display, s_Gc, whiteColor);

    XkbSetDetectableAutoRepeat(s_Display, 1, 0);

    // Wait for the MapNotify event

    while(1)
    {
        XEvent e;
        XNextEvent(s_Display, &e);
        if (e.type == MapNotify)
        {
            break;
        }
    }

    g_Framebuffer = calloc(1, DOOMGENERIC_RESX * DOOMGENERIC_RESY);
    s_Framebuffer = calloc(1, DOOMGENERIC_RESX * DOOMGENERIC_RESY);
    x_Framebuffer = calloc(4, DOOMGENERIC_RESX * DOOMGENERIC_RESY);

    s_Image = XCreateImage(s_Display,                          // display
                           DefaultVisual(s_Display, s_Screen), // visual
                           depth,                              // depth
                           ZPixmap,                            // format
                           0,                                  // offset
                           x_Framebuffer,                      // data
                           DOOMGENERIC_RESX,                   // width
                           DOOMGENERIC_RESY,                   // height
                           32,                                 // bitmap_pad
                           0);                                 // bytes_per_line
}

int sobel_x[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

int sobel_y[3][3] = {
    {-1, -2, -1},
    { 0,  0,  0},
    { 1,  2,  1}
};


void DG_DrawFrame()
{
    if (s_Display)
    {
        while (XPending(s_Display) > 0)
        {
            XEvent e;
            XNextEvent(s_Display, &e);
            if (e.type == KeyPress)
            {
                KeySym sym = XKeycodeToKeysym(s_Display, e.xkey.keycode, 0);
                //printf("KeyPress:%d sym:%d\n", e.xkey.keycode, sym);

                addKeyToQueue(1, sym);
            }
            else if (e.type == KeyRelease)
            {
                KeySym sym = XKeycodeToKeysym(s_Display, e.xkey.keycode, 0);
                //printf("KeyRelease:%d sym:%d\n", e.xkey.keycode, sym);
                addKeyToQueue(0, sym);
            }
        }

        for (int y = 0; y < DOOMGENERIC_RESY; y++)
            for (int x = 0; x < DOOMGENERIC_RESX; x++)
            {
                uint32_t r = (DG_ScreenBuffer[y*DOOMGENERIC_RESX+x] >> 24) & 0xFF;
                uint32_t g = (DG_ScreenBuffer[y*DOOMGENERIC_RESX+x] >> 16) & 0xFF;
                uint32_t b = (DG_ScreenBuffer[y*DOOMGENERIC_RESX+x] >>  8) & 0xFF;
                g_Framebuffer[y*DOOMGENERIC_RESX+x] = (r*30 + g*59 + b*11) / 100;
            }

        for (int y = 1; y < DOOMGENERIC_RESY - 1; y++)
            for (int x = 1; x < DOOMGENERIC_RESX - 1; x++)
            {
                int gx = 0;
                int gy = 0;
                for (int dy = -1; dy <= 1; dy++)
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        unsigned char px = DG_ScreenBuffer[(y+dy)*DOOMGENERIC_RESX+(x+dx)];
                        gx += px * sobel_x[dy+1][dx+1];
                        gy += px * sobel_y[dy+1][dx+1];
                    }
                int g = (int) sqrt(gx*gx + gy*gy);
                if (g > 255)
                    g = 255;
                s_Framebuffer[y*DOOMGENERIC_RESX+x] = (unsigned char) g;
            }

        for (int y = 0; y < DOOMGENERIC_RESY; y++)
            for (int x = 0; x < DOOMGENERIC_RESX; x++)
            {
                int idx = y*DOOMGENERIC_RESX+x;
                x_Framebuffer[idx*4 + 0] = s_Framebuffer[idx];
                x_Framebuffer[idx*4 + 1] = s_Framebuffer[idx];
                x_Framebuffer[idx*4 + 2] = s_Framebuffer[idx];
                x_Framebuffer[idx*4 + 3] = s_Framebuffer[idx];
            }

        XPutImage(s_Display,         // display
                  s_Window,          // drawable
                  s_Gc,              // Graphics Context
                  s_Image,           // image
                  0,                 // src_x
                  0,                 // src_y
                  0,                 // dest_x
                  0,                 // dest_y
                  DOOMGENERIC_RESX,  // width
                  DOOMGENERIC_RESY); // height

        //XFlush(s_Display);
    }

    //printf("frame\n");
}

void DG_SleepMs(uint32_t ms)
{
    usleep (ms * 1000);
}

uint32_t DG_GetTicksMs()
{
    struct timeval  tp;
    struct timezone tzp;

    gettimeofday(&tp, &tzp);

    return (tp.tv_sec * 1000) + (tp.tv_usec / 1000); /* return milliseconds */
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
    if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex)
    {
        //key queue is empty

        return 0;
    }
    else
    {
        unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
        s_KeyQueueReadIndex++;
        s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

        *pressed = keyData >> 8;
        *doomKey = keyData & 0xFF;

        return 1;
    }
}

void DG_SetWindowTitle(const char * title)
{
    if (s_Window)
    {
        XChangeProperty(s_Display, s_Window, XA_WM_NAME, XA_STRING, 8, PropModeReplace, (unsigned char *)title, strlen(title));
    }
}

int main(int argc, char **argv)
{
    doomgeneric_Create(argc, argv);

    for (int i = 0; ; i++)
    {
        doomgeneric_Tick();
    }


    return 0;
}
