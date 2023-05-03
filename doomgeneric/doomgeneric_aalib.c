#include "doomkeys.h"
#include "doomgeneric.h"
#include <aalib.h>

#include <ctype.h>  /* for tolower(3p) */
#include <error.h>  /* for error(3gnu) */
#include <stdio.h>  /* for puts(3p) */
#include <stdlib.h> /* for setenv(3p) and unsetenv(3p) */

static aa_context *context = NULL;

int main(int argc, char **argv) {
	/* AAOPTS */
	if (!aa_parseoptions(NULL, NULL, &argc, argv)) {
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

void DG_Init() {}

void DG_SetWindowTitle(const char *title) {}

#define div_roundup(n, d) (((n) + (d) - 1) / (d))
#define min(a, b) (((a) < (b)) ? (a) : (b))

// Wrapper to discard the return value.
void my_resize(aa_context *context) {
	aa_resize(context);
}

void DG_DrawFrame() {
	if (context == NULL) {
		context = aa_autoinit(&aa_defparams);
		if (context == NULL)
			error(1, 0, "failed to initialize aalib graphics");
		if (!aa_autoinitkbd(context, 0)) {
			aa_close(context);
			error(1, 0, "failed to initialize aalib keyboard");
		}
		aa_hidecursor(context);
		aa_resizehandler(context, my_resize);
	}

	unsigned char *out_buffer = aa_image(context);

	int full_out_resx = aa_imgwidth(context);
	int full_out_resy = aa_imgheight(context);

	int out_resx = full_out_resx;
	int out_resy = full_out_resy - 1;

	int doomx_per_outx = div_roundup(DOOMGENERIC_RESX, out_resx);
	int doomy_per_outy = div_roundup(DOOMGENERIC_RESY, out_resy);
	// Maybe shrink out_resx and/or out_resy due to rounding.
	out_resx = DOOMGENERIC_RESX / doomx_per_outx;
	out_resy = DOOMGENERIC_RESY / doomy_per_outy;

	// Shrink one of out_resx or out_resy to get a 4:3 aspect ratio.
	//out_resx = min(out_resx, ((4*out_resy)/3));
	//out_resy = min(out_resy, ((3*out_resx)/4));

	int out_xoff = (full_out_resx - out_resx) / 2;

	uint32_t v, r, g, b;
	for (int oy = 0; oy < out_resy; oy++) {
		for (int ox = 0; ox < out_resx; ox++) {
			v = 0;
			for (int dy = oy*doomy_per_outy; dy < (oy+1)*doomy_per_outy; dy++) {
				for (int dx = ox*doomx_per_outx; dx < (ox+1)*doomx_per_outx; dx++) {
					r = (DG_ScreenBuffer[dy*DOOMGENERIC_RESX+dx] >> 24) & 0xFF;
					g = (DG_ScreenBuffer[dy*DOOMGENERIC_RESX+dx] >> 16) & 0xFF;
					b = (DG_ScreenBuffer[dy*DOOMGENERIC_RESX+dx] >>  8) & 0xFF;
					v += (r*30 + g*59 + b*11) / 100;
				}
			}
			out_buffer[oy*full_out_resx+out_xoff+ox] = v/(doomx_per_outx*doomy_per_outy);
		}
	}

	aa_render(context, &aa_defrenderparams,
	          /* TTY X1 */ out_xoff,
	          /* TTY Y1 */ 0,
	          /* TTY X2 */ out_xoff + out_resx,
	          /* TTY Y2 */ out_resy);

	aa_flush(context);
}

unsigned char lastDoomKey = 0;
int nextKeyEvent = 0;

int DG_GetKey(int *pressed, unsigned char *doomKey) {
	if (context == NULL)
		return 0;

	int event;
 retry:
	if (nextKeyEvent) {
		event = nextKeyEvent;
		nextKeyEvent = 0;
	} else
		event = aa_getkey(context, 0);
	if (!event) {
		if (lastDoomKey) {
			*pressed = 0;
			*doomKey = lastDoomKey;
			lastDoomKey = 0;
			return 1;
		}
		return 0;
	}
	if (event >= AA_RELEASE)
		goto retry;

	if (lastDoomKey) {
		*pressed = 0;
		*doomKey = lastDoomKey;
		lastDoomKey = 0;
		nextKeyEvent = event;
		return 1;
	}

	switch (event) {
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
