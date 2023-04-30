#include "doomkeys.h"
#include "doomgeneric.h"
#include <aalib.h>

#include <ctype.h>  /* for tolower(3p) */
#include <error.h>  /* for error(3gnu) */
#include <stdio.h>  /* for puts(3p) */
#include <stdlib.h> /* for setenv(3p) and unsetenv(3p) */

static aa_context *context = NULL;

int main(int argc, char **argv)
{
    /* AAOPTS */
    if (!aa_parseoptions(NULL, NULL, &argc, argv))
    {
        puts(aa_help);
        return 2;
    }

    doomgeneric_Create(argc, argv);
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

#define div_roundup(n, d) (((n) + (d) - 1) / (d))
#define min(a, b) (((a) < (b)) ? (a) : (b))

struct {
    // The dimensions of the full grayscale pixel framebuffer.
    int full_out_resx;
    int full_out_resy;

    // The inner portion of that framebuffer that we'll be drawing to
    // (letterboxed).
    int out_xoff;
    int out_resx;
    int out_resy;

    // How many pixels from the DG_ScreenBuffer framebuffer will
    // become one pixel in the grayscale framebuffer.
    int doomx_per_outx;
    int doomy_per_outy;
} screensize;

void my_resize(aa_context *context)
{
    aa_resize(context);

    screensize.full_out_resx = aa_imgwidth(context);
    screensize.full_out_resy = aa_imgheight(context);
    fprintf(stderr, "aa pixbuf: %d x %d\n", screensize.full_out_resx, screensize.full_out_resy);

    // Shrink one of dimension to get a 4:3 aspect ratio.
    screensize.out_resx = min(screensize.full_out_resx, ((4*screensize.full_out_resy)/3));
    screensize.out_resy = min(screensize.full_out_resy, ((3*screensize.full_out_resx)/4));

    screensize.doomx_per_outx = div_roundup(DOOMGENERIC_RESX, screensize.out_resx);
    screensize.doomy_per_outy = div_roundup(DOOMGENERIC_RESY, screensize.out_resy);
    // Maybe shrink out_resx and/or out_resy due to rounding.
    //screensize.out_resx = DOOMGENERIC_RESX / screensize.doomx_per_outx;
    //screensize.out_resy = DOOMGENERIC_RESY / screensize.doomy_per_outy;

    screensize.out_xoff = (screensize.full_out_resx - screensize.out_resx) / 2;
}

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

    unsigned char *out_buffer = aa_image(context);

    // The VT220 is 800x240 non-square pixels (80x24 chars) at a 8:5 aspect ratio.
    //
    // DG_ScreenBuffer is 320x200 non-square pixels at a 4:3 aspect ratio.
    //
    // Cropping the VT220 to 4:3 gives us 600x240 pixels (60x24 chars).

    // If I instead say that a VT220 is (80*7)x240 non-square pixels, that crops to 420x240 pixels.
    //
    // X: 1.3 DOOM pixels per VT pixel.

    //   0123456789
    // 0 AAAAABBBBB
    // 1 AAAAABBBBB
    // 2 AAAAABBBBB
    // 3 CCCCCDDDDD
    // 4 CCCCCDDDDD
    // 5 CCCCCDDDDD
    // 6 EEEEEFFFFF
    // 7 EEEEEFFFFF
    // 8 EEEEEFFFFF
    // 9

    // x: 6  DP => 1 char : 53.3 char => 5.333"
    // y: 10 DP => 1 char : 20   char => 4.166"
    //
    // 4:3   = 1.333
    // 5⅓:4⅙ = 1.28

    uint32_t v, r, g, b;
    for (int oy = 0; oy < screensize.out_resy; oy++)
    {
        for (int ox = 0; ox < screensize.out_resx; ox++)
        {
            v = 0;
            for (int dy = oy*screensize.doomy_per_outy; dy < (oy+1)*screensize.doomy_per_outy; dy++)
            {
                for (int dx = ox*screensize.doomx_per_outx; dx < (ox+1)*screensize.doomx_per_outx; dx++)
                {
                    r = (DG_ScreenBuffer[dy*DOOMGENERIC_RESX+dx] >> 24) & 0xFF;
                    g = (DG_ScreenBuffer[dy*DOOMGENERIC_RESX+dx] >> 16) & 0xFF;
                    b = (DG_ScreenBuffer[dy*DOOMGENERIC_RESX+dx] >>  8) & 0xFF;
                    v += (r*30 + g*59 + b*11) / 100;
                }
            }
            out_buffer[oy*screensize.full_out_resx+screensize.out_xoff+ox] =
                v/(screensize.doomx_per_outx*screensize.doomy_per_outy);
        }
    }

    aa_render(context, &aa_defrenderparams,
              /* TTY X1 */ 0,
              /* TTY Y1 */ 0,
              /* TTY X2 */ aa_scrwidth(context),
              /* TTY Y2 */ aa_scrheight(context)-1);

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
