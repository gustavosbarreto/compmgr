/*
 * $Id$
 *
 * Copyright © 2003-2004 Fredrik Höglund <fredrik@kde.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Fredrik Höglund not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Fredrik Höglund makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * FREDRIK HÖGLUND DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL FREDRIK HÖGLUND BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <qrect.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xfixes.h>

#include <cmath>
#include <cstdlib>
#include <stdio.h>
#include <string.h>

#include "shadowpainter.h"
#include "utils.h"


ShadowPainter *ShadowPainter::s_self = NULL;
extern Display *dpy;


ShadowPainter *ShadowPainter::self()
{
	if ( !s_self )
		s_self = new ShadowPainter;
	return s_self;
}


ShadowPainter::ShadowPainter ()
	: source( None ), shadowOpacity( 1.0 ), lastOpacity( -1 ), tilesValid( false )
{
	for (int i = 0; i < NTiles; i++)
		tiles[i] = None;
}


ShadowPainter::~ShadowPainter ()
{
	for (int i = 0; i < NTiles; i++)
		releasePicture( tiles[i] );

	XRenderFreePicture( dpy, source );
}


void ShadowPainter::setRadius( double r )
{
	radius = r;

	for ( int i = 0; i < NTiles; i++ )
		releasePicture( tiles[i] );

	tilesValid = false;
}


void ShadowPainter::setOpacity( double opacity )
{
	shadowOpacity = opacity;
}


void ShadowPainter::setupForOpacity( double opacity )
{
	if ( !source ) {
		XRenderPictFormat *format = XRenderFindStandardFormat( dpy, PictStandardARGB32 );
		XRenderPictureAttributes pa;
		pa.repeat = true; 

		Pixmap pixmap = XCreatePixmap( dpy, DefaultRootWindow(dpy), 1, 1, 32 );
		source = XRenderCreatePicture( dpy, pixmap, format, CPRepeat, &pa );
		XFreePixmap( dpy, pixmap );
	} 

	if ( int(opacity * 0xff) != int(lastOpacity * 0xff) ) {
		XRenderColor col = { 0, 0, 0, int( 0xffff * opacity ) };
		XRenderFillRectangle( dpy, PictOpSrc, source, &col, 0, 0, 1, 1 );
	}

	lastOpacity = opacity;
}


inline void ShadowPainter::blendTile( Tile tile, Picture dest, int x, int y )
{
	XRenderComposite( dpy, PictOpOver, source, tiles[tile], dest, 0, 0, 0, 0, x, y, size, size );
}

inline void ShadowPainter::blendTile( Tile tile, Picture dest, int x, int y, int width, int height )
{
	XRenderComposite( dpy, PictOpOver, source, tiles[tile], dest, 0, 0, 0, 0, x, y, width, height );
}

void ShadowPainter::draw( Picture dest, const QRect &r, double windowOpacity )
{
	if ( !tilesValid )
		createTiles();

	setupForOpacity( shadowOpacity * windowOpacity );

	int lw = (r.width() > size * 2) ? size : r.width() / 2;
	int th = (r.height() > size * 2) ? size : r.height() / 2;
	int rw = qMin( r.width() - lw, size );
	int bh = qMin( r.height() - th, size );
	int sx = size - rw; // Only for right column
	int sy = size - bh; // Only for bottom row

	// Top row
	// ------------------------------------------------------------------------
	XRenderComposite( dpy, PictOpOver, source, tiles[TopLeft], dest,
					  0, 0, 0, 0, r.x(), r.y(), lw, th );

	if ( r.width() > size * 2 )
		XRenderComposite( dpy, PictOpOver, source, tiles[Top], dest,
						  0, 0, 0, 0, r.x() + size, r.y(), r.width() - size * 2, th );

	XRenderComposite( dpy, PictOpOver, source, tiles[TopRight], dest,
					  sx, 0, 0, 0, r.right() - rw + 1, r.y(), rw, th );

	// Center row
	// ------------------------------------------------------------------------
	if ( r.height() > size * 2 ) {
		XRenderComposite( dpy, PictOpOver, source, tiles[Left], dest,
						  0, 0, 0, 0, r.x(), r.y() + size, lw, r.height() - size * 2 );

		if ( r.width() > size * 2 )
			XRenderComposite( dpy, PictOpOver, source, tiles[Center], dest,
							  0, 0, 0, 0, r.x() + size, r.y() + size,
							  r.width() - size * 2, r.height() - size * 2 );

		XRenderComposite( dpy, PictOpOver, source, tiles[Right], dest,
						  sx,0, 0,0, r.right() - rw + 1, r.y() + size,
						  rw, r.height() - size * 2 );
	}

	// Bottom row
	// ------------------------------------------------------------------------
	XRenderComposite( dpy, PictOpOver, source, tiles[BottomLeft], dest,
					  0, sy, 0, 0, r.x(), r.bottom() - bh + 1, lw, bh );

	if ( r.width() > size * 2 )
		XRenderComposite( dpy, PictOpOver, source, tiles[Bottom], dest,
						  0, sy, 0, 0, r.x() + size, r.bottom() - bh + 1,
						  r.width() - size * 2, bh );

	XRenderComposite( dpy, PictOpOver, source, tiles[BottomRight], dest,
					  sx, sy, 0, 0, r.right() - rw + 1, r.bottom() - bh + 1, rw, bh );
}


void ShadowPainter::draw( Picture dest, const QRect &r, Picture shadow, double windowOpacity )
{
	setupForOpacity( shadowOpacity * windowOpacity );
	XRenderComposite( dpy, PictOpOver, source, shadow, dest, 0, 0, 0, 0,
					  r.x(), r.y(), r.width(), r.height() );
}


Picture ShadowPainter::createShadow( const QSize &s )
{
	int width = s.width(), height = s.height();
	Picture shadow = createPicture( width, height );

	if ( !tilesValid )
		createTiles();

	int lw = (width > size * 2) ? size : width / 2;
	int th = (height > size * 2) ? size : height / 2;
	int rw = qMin( width - lw, size );
	int bh = qMin( height - th, size );
	int sx = size - rw; // Only for right column
	int sy = size - bh; // Only for bottom row

	// Top row
	// ------------------------------------------------------------------------
	XRenderComposite( dpy, PictOpSrc, tiles[TopLeft], None, shadow,
					  0, 0, 0, 0, 0, 0, lw, th );

	if ( width > size * 2 )
		XRenderComposite( dpy, PictOpSrc, tiles[Top], None, shadow,
						  0, 0, 0, 0, size, 0, width - size * 2, th );

	XRenderComposite( dpy, PictOpSrc, tiles[TopRight], None, shadow,
					  sx, 0, 0, 0, width - rw, 0, rw, th );

	// Center row
	// ------------------------------------------------------------------------
	if ( height > size * 2 ) {
		XRenderComposite( dpy, PictOpSrc, tiles[Left], None, shadow,
						  0, 0, 0, 0, 0, size, lw, height - size * 2 );

		if ( width > size * 2 )
			XRenderComposite( dpy, PictOpSrc, tiles[Center], None, shadow,
							  0, 0, 0, 0, size, size, width - size * 2, height - size * 2 );
			
		XRenderComposite( dpy, PictOpSrc, tiles[Right], None, shadow,
						  sx, 0, 0, 0, width - rw, size, rw, height - size * 2 );
	}

	// Bottom row
	// ------------------------------------------------------------------------
	XRenderComposite( dpy, PictOpSrc, tiles[BottomLeft], None, shadow,
					  0, sy, 0, 0, 0, height - bh, lw, bh );

	if ( width > size * 2 )
		XRenderComposite( dpy, PictOpSrc, tiles[Bottom], None, shadow,
						  0, sy, 0, 0, size, height - bh, width - size * 2, bh );

	XRenderComposite( dpy, PictOpSrc, tiles[BottomRight], None, shadow,
					  sx, sy, 0, 0, width - rw, height - bh, rw, bh );

	return shadow;
}


double ShadowPainter::gaussian( double x, double y )
{
	return ((1 / sqrt (2 * M_PI * radius))
			* std::exp (-(x*x + y*y) / (2 * radius * radius)));
}


unsigned char ShadowPainter::sum (double *kernel, int x, int y)
{
	int center = size / 2;

	int x_start = qMax( center - x, 0 );
	int x_end   = qMin( size + center - x, size );

	int y_start = qMax( center - y, 0 );
	int y_end   = qMin( size + center - y, size );

	double val = 0;
	for (int y = y_start; y < y_end; y++) {
		int ki = y * size + x_start;
		for (int x = x_start; x < x_end; x++)
			val += kernel[ki++];
	}

	if (val > 1)
		val = 1;

	return ((unsigned char) (val * 255.0));
}


void ShadowPainter::createTiles ()
{
	// Create the gaussian kernel
	// -------------------------------------------------------------
	size = int (std::ceil (radius * 3) + 1) & ~1;

	int center  = size / 2;
	int len     = size * size;

	double *kernel = new double [len];

	double total = 0;
	int ki = 0;
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++)
		{
			double val = gaussian (x - center, y - center);
			kernel[ki++] = val;
			total += val;
		}
	}

	for (int i = 0; i < len; i++)
		kernel[i] /= total;


	// Create the shadow image
	// ------------------------------------------------------------
	int width = int( size * 3);
	int height = width;

	// Allocate image data
	unsigned char *data = (unsigned char *)malloc (width * height);

	// Center fill
	unsigned char pixel = sum (kernel, center, center);
	memset( data, pixel, width * height );

	// Corners
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++)
		{
			unsigned char pixel = sum (kernel, x - center, y - center);
			data[y * width + x] = pixel;
			data[(height - y - 1) * width + x] = pixel;
			data[(height - y - 1) * width + (width - x - 1)] = pixel;
			data[y * width + (width - x - 1)] = pixel;
		}
	}

	// Top / bottom
	for (int y = 0; y < size; y++) {
		unsigned char pixel = sum (kernel, center, y - center);
		int i = y * width + size;
		int j = (height - y - 1) * width + size;
		for (int x = size; x < width - size; x++) {
			data[i++] = pixel;
			data[j++] = pixel;
		}
	}

	// Left / Right
	for (int x = 0; x < size; x++) {
		unsigned char pixel = sum (kernel, x - center, center);
		for (int y = size; y < height - size; y++) {
			data[y * width + x] = pixel;
			data[y * width + (width - x - 1)] = pixel;
		}
	}

	delete [] kernel;

	
	// Convert the image to a pixmap
	// ------------------------------------------------------------
	XImage *image = XCreateImage( dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
								  8, ZPixmap, 0, (char*)data, width, height, 8, width );
	Pixmap pixmap = XCreatePixmap( dpy, DefaultRootWindow(dpy), width, height, 8 );
	GC gc = XCreateGC( dpy, pixmap, 0, 0 );
	XPutImage( dpy, pixmap, gc, image, 0, 0, 0, 0, width, height );
	XFreeGC( dpy, gc );
	XDestroyImage( image );	

	XRenderPictFormat *format = XRenderFindStandardFormat( dpy, PictStandardA8 );
	Picture picture = XRenderCreatePicture( dpy, pixmap, format, 0, 0 );
	XFreePixmap( dpy, pixmap );


	// Cut the tiles from the pixmap
	// ------------------------------------------------------------
	tiles[TopLeft]     = createPicture( picture, 0, 0, size, size );
	tiles[Top]         = createPicture( picture, size, 0, size, size, Repeating );
	tiles[TopRight]    = createPicture( picture, width - size, 0, size, size );

	tiles[Left]        = createPicture( picture, 0, size, size, size, Repeating );
	tiles[Center]      = createPicture( picture, size, size, size, size, Repeating );
	tiles[Right]       = createPicture( picture, width - size, size, size, size, Repeating );

	tiles[BottomLeft]  = createPicture( picture, 0, height - size, size, size);
	tiles[Bottom]      = createPicture( picture, size, height - size, size, size, Repeating );
	tiles[BottomRight] = createPicture( picture, width - size, height - size, size, size );

	XRenderFreePicture( dpy, picture );
	tilesValid = true;
}


Picture ShadowPainter::createPicture( Picture source, int x, int y, int width, int height, int flags )
{
	Picture picture = createPicture( width, height, flags );
	XRenderComposite( dpy, PictOpSrc, source, None, picture, x, y, 0, 0, 0, 0, width, height );
	return picture;
}


Picture ShadowPainter::createPicture( int width, int height, int flags )
{
	XRenderPictFormat *format = XRenderFindStandardFormat( dpy, PictStandardA8 );
	XRenderPictureAttributes pa;
	pa.repeat = bool( flags & Repeating );

	Pixmap pixmap = XCreatePixmap( dpy, DefaultRootWindow( dpy ), width, height, 8 );
	Picture picture = XRenderCreatePicture( dpy, pixmap, format, CPRepeat, &pa );
	XFreePixmap( dpy, pixmap );

	return picture;
}

