#include <ctype.h>  /* for tolower(3p) */
#include <error.h>  /* for error(3gnu) */
#include <stdio.h>  /* for puts(3p) */
#include <stdlib.h> /* for setenv(3p) and unsetenv(3p) */

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

    // We have several framebuffers in a pipeline:
    //  1. (DOOM engine) DG_ScreenBuffer is 320x200 non-square pixels at a 4:3 aspect ratio (=pixels are 5:6)
    //  2. (aadoom)      out_buffer      is 160x48  non-square pixels at a 8:5 aspect ratio (=pixels are 12:25)
    //  3. (aalib)   context->textbuffer is  80x24  chars             at a 8:5 aspect ratio (=chars are 12:25)
    //  4. (VT220)       <display>       is 800x240 non-square pixels at a 8:5 aspect ratio (=pixels are 12:25)
    //
    // Cropping the VT220's 8:5 to 4:3 gives us a letterboxed region of:
    //  2. (aadoom)      out_buffer      is 133⅓x48  non-square pixels at a 8:5 aspect ratio (=pixels are 12:25)
    //  3. (aalib)   context->textbuffer is  66⅔x24  chars             at a 8:5 aspect ratio (=chars are 12:25)
    //  4. (VT220)       <display>       is 666⅔x240 non-square pixels at a 8:5 aspect ratio (=pixels are 12:25)
    //
    // So, we need to downscale that 320x200px image to 133⅓x48px, or a scaling factor of 2.4x1⅙.

    for (int oy = 0; oy < screensize.out_resy; oy++)
    {
        for (int ox = 0; ox < screensize.out_resx; ox++)
        {
            uint32_t v = 0;
            for (int dy = oy*screensize.doomy_per_outy; dy < (oy+1)*screensize.doomy_per_outy; dy++)
            {
                for (int dx = ox*screensize.doomx_per_outx; dx < (ox+1)*screensize.doomx_per_outx; dx++)
                {
                    uint32_t r = (DG_ScreenBuffer[dy*DOOMGENERIC_RESX+dx] >> 24) & 0xFF;
                    uint32_t g = (DG_ScreenBuffer[dy*DOOMGENERIC_RESX+dx] >> 16) & 0xFF;
                    uint32_t b = (DG_ScreenBuffer[dy*DOOMGENERIC_RESX+dx] >>  8) & 0xFF;
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
