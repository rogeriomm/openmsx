// $Id$

/*  Written By: Garrett Banuk <mongoose@mongeese.org>
 *  This is free, just be sure to give me credit when using it
 *  in any of your programs.
 */

#include "SDLFont.hh"
#include "openmsx.hh"
#include "MSXException.hh"


SDLFont::SDLFont(const std::string &bitmapName)
{
	// load the font bitmap
	SDL_Surface *tempSurface;
	if (!(tempSurface = SDL_LoadBMP(bitmapName.c_str())))
		throw MSXException("Can't load console font");

	fontSurface = SDL_DisplayFormat(tempSurface);
	SDL_FreeSurface(tempSurface);

	charWidth  = fontSurface->w / 256;
	charHeight = fontSurface->h;

	// Set font as transparent. The assumption we'll go on is that the
	// first pixel of the font image will be the color we should treat
	// as transparent.
	if (SDL_GetVideoSurface()->flags & SDL_OPENGLBLIT)
		setAlphaGL(SDL_ALPHA_TRANSPARENT);
	else
		SDL_SetColorKey(fontSurface, SDL_SRCCOLORKEY | SDL_RLEACCEL, 
		                SDL_MapRGB(fontSurface->format, 255, 0, 255));
}

SDLFont::~SDLFont()
{
	SDL_FreeSurface(fontSurface);
}

// sets the transparency value for the font in question.  assumes that
// we're in an OpenGL context
void SDLFont::setAlphaGL(int alpha)
{
	if (fontSurface->format->BytesPerPixel == 2) {
		// 16-bit SDL surfaces do not support alpha-blending under OpenGL
		return;
	}

	// iterate over all pixels in the font surface.  For each pixel that
	// is (255,0,255), set its alpha channel appropriately.
	int imax = fontSurface->h * fontSurface->w * 4;
	unsigned char *pix = (unsigned char *)(fontSurface->pixels);
	unsigned char r_targ = 255;	// pix[0]
	unsigned char g_targ = 0;	// pix[1]
	unsigned char b_targ = 255;	// pix[2]
	for (int i=3; i<imax; i+=4)
		if (pix[i-3] == r_targ && pix[i-2] == g_targ && pix[i-1] == b_targ)
			pix[i] = alpha;
	// also make sure that alpha blending is disabled for the font surface
	SDL_SetAlpha(fontSurface, 0, SDL_ALPHA_OPAQUE);
}

void SDLFont::drawText(const std::string &string, SDL_Surface *surface, int x, int y)
{
	// see how many characters can fit on the screen
	if (x > surface->w || y > surface->h)
		return;
	int characters = string.length();
	if (characters > (surface->w - x) / charWidth)
		characters = (surface->w - x) / charWidth;

	SDL_Rect DestRect;
	DestRect.x = x;
	DestRect.y = y;
	DestRect.w = charWidth;
	DestRect.h = charHeight;

	SDL_Rect SourceRect;
	SourceRect.y = 0;
	SourceRect.w = charWidth;
	SourceRect.h = charHeight;

	// Now draw it
	for (int loop=0; loop<characters; loop++) {
		SourceRect.x = string[loop] * charWidth;
		SDL_BlitSurface(fontSurface, &SourceRect, surface, &DestRect);
		DestRect.x += charWidth;
	}
	// if we're in OpenGL-mode, we need to manually update after blitting
	if (surface->flags & SDL_OPENGLBLIT) {
		DestRect.x = x;
		DestRect.w = characters * charWidth;
		SDL_UpdateRects(surface, 1, &DestRect);
	}
}

int SDLFont::height()
{
	return charHeight;
}

int SDLFont::width()
{
	return charWidth;
}

