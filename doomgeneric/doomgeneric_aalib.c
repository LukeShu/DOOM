#include "doomkeys.h"
#include "doomgeneric.h"
#include <aalib.h>

#include <ctype.h>  /* for tolower(3p) */
#include <error.h>  /* for error(3gnu) */
#include <stdlib.h> /* for setenv(3p) and unsetenv(3p) */

static aa_context *context = NULL;

#define min(a, b) ((a) < (b) ? (a) : (b))

int main(int argc, char **argv) {
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

void DG_DrawFrame() {
	if (context == NULL) {
		context = aa_autoinit(&aa_defparams);
		if (context == NULL)
			error(1, 0, "failed to initialize aalib graphics");
		if (!aa_autoinitkbd(context, 0)) {
			aa_close(context);
			error(1, 0, "failed to initialize aalib keyboard");
		}
	}

	unsigned char *out_buffer = aa_image(context);
	int out_resx = aa_imgwidth(context);
	int out_resy = aa_imgheight(context);

	//aa_close(context);
	//error(1, 0, "resx=%d resy=%d", out_resx, out_resy);

	uint32_t r, g, b;
	for (unsigned int y = 0; y < min(DOOMGENERIC_RESY, out_resy); y++) {
		for (unsigned int x = 0; x < min(DOOMGENERIC_RESX, out_resx); x++) {
			r = (DG_ScreenBuffer[y*DOOMGENERIC_RESX+x] >> 24) & 0xFF;
			g = (DG_ScreenBuffer[y*DOOMGENERIC_RESX+x] >> 16) & 0xFF;
			b = (DG_ScreenBuffer[y*DOOMGENERIC_RESX+x] >>  8) & 0xFF;
			out_buffer[y*out_resx+x] = (r + g + b) / 3;
		}
	}

	aa_render(context, &aa_defrenderparams,
	          /* TTY X1 */ 0,
	          /* TTY Y1 */ 0,
	          /* TTY X2 */ aa_scrwidth(context),
	          /* TTY Y2 */ aa_scrheight(context));

	aa_flush(context);
}

int DG_GetKey(int *pressed, unsigned char *doomKey) {
	if (context == NULL)
		return 0;

	int event = aa_getevent(context, 0);
	if (!event)
		return 0;

	*pressed = event >= AA_RELEASE;
	event &= ~AA_RELEASE;

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
	return 1;
}
