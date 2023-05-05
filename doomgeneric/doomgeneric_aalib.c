#include <ctype.h>  /* for tolower(3p) */
#include <error.h>  /* for error(3gnu) */
#include <math.h>   /* for round(3p), ceil(3p), floor(3p), sqrt(3p) */
#include <stdio.h>  /* for puts(3p), realloc(3p) */
#include <stdlib.h> /* for setenv(3p) and unsetenv(3p) */
#include <string.h> /* for memset(3p) */

#include <aalib.h>

#include "doomgeneric.h"
#include "doomkeys.h"
#include "m_argv.h"

static aa_context *context = NULL;

int main(int argc, char **argv)
{
    // aa_parseoptions looks at both argv and getenv("AAOPTS").
    if (!aa_parseoptions(NULL, NULL, &argc, argv))
    {
        puts(aa_help);
        return 2;
    }

    doomgeneric_Create(argc, argv);
    if (M_ParmExists("-help")) {
        puts(aa_help);
        return 0;
    }

    for (;;)
        doomgeneric_Tick();

    if (context != NULL)
        aa_close(context);

    return 0;
}

void DG_Init()
{
}

void DG_SetWindowTitle(const char *title)
{
}

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define bound(lo, a, hi) min(max(a, lo), hi)

// We have several framebuffers in a pipeline:
//  1. (DOOM)   DG_ScreenBuffer      is 320x200 non-square pixels at a 4:3 aspect ratio (=pixels are 5:6)
//  2. (aadoom) context->imagebuffer is 160x48  non-square pixels at a 8:5 aspect ratio (=pixels are 12:25)
//  3. (aalib)  context->textbuffer  is  80x24  chars             at a 8:5 aspect ratio (=chars are 12:25)
//  4. (VT220)  <display>            is 800x240 non-square pixels at a 8:5 aspect ratio (=pixels are 12:25)
//
// Cropping the VT220's 8:5 to 4:3 gives us a letterboxed region of:
//  2. (aadoom) context->imagebuffer is 133⅓x48  non-square pixels at a 8:5 aspect ratio (=pixels are 12:25)
//  3. (aalib)  context->textbuffer  is  66⅔x24  chars             at a 8:5 aspect ratio (=chars are 12:25)
//  4. (VT220)  <display>            is 666⅔x240 non-square pixels at a 8:5 aspect ratio (=pixels are 12:25)
//
// So, we need to downscale that 320x200px image to 133x48px, or a scaling factor of ~2.4x4.2.
struct {
    // The dimensions of the full grayscale pixel framebuffer.
    int full_aa_resx;
    int full_aa_resy;

    // The inner portion of that framebuffer that we'll be drawing to
    // (letterboxed).
    int aa_xoff;
    int aa_yoff;
    int aa_resx;
    int aa_resy;

    // How many pixels from the DG_ScreenBuffer framebuffer will
    // become one pixel in the grayscale framebuffer.
    double doomx_per_aax;
    double doomy_per_aay;

    unsigned char *tmpbuf;
} screen;

void my_resize(aa_context *context)
{
    aa_resize(context);

    screen.full_aa_resx = aa_imgwidth(context);
    screen.full_aa_resy = aa_imgheight(context);

    // Crop that 8:5 to 4:3.
    screen.aa_resx = min(screen.full_aa_resx, (int)round(screen.full_aa_resy * (25.0/12.0) * (4.0/3.0)));
    screen.aa_resy = min(screen.full_aa_resy, (int)round(screen.full_aa_resx * (12.0/25.0) * (3.0/4.0)));
    screen.aa_xoff = (screen.full_aa_resx - screen.aa_resx) / 2;
    screen.aa_yoff = (screen.full_aa_resy - screen.aa_resy) / 2;

    screen.doomx_per_aax = ((float)DOOMGENERIC_RESX) / ((float)screen.aa_resx);
    screen.doomy_per_aay = ((float)DOOMGENERIC_RESY) / ((float)screen.aa_resy);

    screen.tmpbuf = realloc(screen.tmpbuf, screen.aa_resx * screen.aa_resy);
    memset(screen.tmpbuf, 0, screen.aa_resx * screen.aa_resy);
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
    if (context == NULL)
    {
        context = aa_autoinit(&aa_defparams);
        if (context == NULL)
            error(1, 0, "failed to initialize aalib graphics");
        if (!aa_autoinitkbd(context, 0))
        {
            aa_close(context);
            error(1, 0, "failed to initialize aalib keyboard");
        }
        aa_hidecursor(context);
        aa_resizehandler(context, my_resize);
        my_resize(context);
    }

#define get_rgb_(x, y) (DG_ScreenBuffer[bound(0, y, DOOMGENERIC_RESY-1)*DOOMGENERIC_RESX+bound(0, x, DOOMGENERIC_RESX-1)])
#define get_r(x, y) ((get_rgb_(x, y) >> 24) & 0xFF)
#define get_g(x, y) ((get_rgb_(x, y) >> 16) & 0xFF)
#define get_b(x, y) ((get_rgb_(x, y) >>  8) & 0xFF)
#define get_luma(x, y) ((unsigned char)((get_r(x,y)*30 + get_g(x,y)*59 + get_b(x,y)*11) / 100))
    // Convert to greyscale and downscale;
    // Input is DG_ScreenBufferConvert, output is screen.tmpbuf.
    for (int oy = 0; oy < screen.aa_resy; oy++)
        for (int ox = 0; ox < screen.aa_resx; ox++)
        {
            int lo_x = floor((ox  ) * screen.doomx_per_aax);
            int hi_x = ceil( (ox+1) * screen.doomx_per_aax);
            int lo_y = floor((oy  ) * screen.doomy_per_aay);
            int hi_y = ceil( (oy+1) * screen.doomy_per_aay);
            int sum = 0;
            for (int iy = lo_y; iy < hi_y; iy++)
                for (int ix = lo_x; ix < hi_x; ix++)
                    sum += get_luma(ix, iy);
            screen.tmpbuf[oy*screen.aa_resx+ox] = sum / ( (hi_x-lo_x) * (hi_y-lo_y) );
        }

    // Apply the Sobel operator and letterboxing.
    // Input is screen.tmpbuff, output context->imagebuffer.
    for (int y = 0; y < screen.aa_resy; y++)
        for (int x = 0; x < screen.aa_resx; x++)
        {
            int gx = 0;
            int gy = 0;
            for (int dy = -1; dy <= 1; dy++)
                for (int dx = -1; dx <= 1; dx++)
                {
                    unsigned char px = screen.tmpbuf[bound(0, y+dy, screen.aa_resy-1)*screen.aa_resx + bound(0, x+dx, screen.aa_resx-1)];
                    gx += px * sobel_x[dy+1][dx+1];
                    gy += px * sobel_y[dy+1][dx+1];
                }
            int g = (int) sqrt(gx*gx + gy*gy);
            if (g > 255)
                g = 255;
            aa_image(context)[(y+screen.aa_yoff)*screen.full_aa_resx+(x+screen.aa_xoff)] = (unsigned char) g;
        }

    // Convert to text.
    // Input is context->imagebuffer, output is context->textbuffer.
    aa_render(context, &aa_defrenderparams,
              /* TTY X1 */ 0,
              /* TTY Y1 */ 0,
              /* TTY X2 */ aa_scrwidth(context),
              /* TTY Y2 */ aa_scrheight(context));

    // Send to the VT220.
    aa_flush(context);
}

unsigned char lastDoomKey = 0;
int nextKeyEvent = 0;

int DG_GetKey(int *pressed, unsigned char *doomKey)
{
    if (context == NULL)
        return 0;

    int event;
 retry:
    if (nextKeyEvent)
    {
        event = nextKeyEvent;
        nextKeyEvent = 0;
    } else
        event = aa_getkey(context, 0);
    if (!event)
    {
        if (lastDoomKey)
        {
            *pressed = 0;
            *doomKey = lastDoomKey;
            lastDoomKey = 0;
            return 1;
        }
        return 0;
    }
    if (event >= AA_RELEASE)
        goto retry;

    if (lastDoomKey)
    {
        *pressed = 0;
        *doomKey = lastDoomKey;
        lastDoomKey = 0;
        nextKeyEvent = event;
        return 1;
    }

    switch (event)
    {
      case AA_UP:
        *doomKey = KEY_UPARROW;
        break;
      case AA_DOWN:
        *doomKey = KEY_DOWNARROW;
        break;
      case AA_LEFT:
        *doomKey = KEY_LEFTARROW;
        break;
      case AA_RIGHT:
        *doomKey = KEY_RIGHTARROW;
        break;
      case AA_BACKSPACE:
        *doomKey = KEY_BACKSPACE;
        break;
      case AA_ESC:
        *doomKey = KEY_ESCAPE;
        break;
      default:
        *doomKey = (unsigned char)tolower(event);
        if (event >= 127)
            aa_printf(context, 0, aa_scrheight(context)-1, AA_NORMAL,
                      "unknown keycode: %d", event);
    }
    *pressed = 1;
    lastDoomKey = *doomKey;
    aa_printf(context, 0, aa_scrheight(context)-1, AA_NORMAL,
              "doomkey=%d                    ", *doomKey);
    return 1;
}
