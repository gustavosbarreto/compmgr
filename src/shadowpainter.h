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

#ifndef _SHADOWPAINTER_H
#define _SHADOWPAINTER_H

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

class ShadowPainter
{
	private:
		enum Tile { TopLeft=0, Top, TopRight, Right,
			BottomRight, Bottom, BottomLeft, Left, Center, NTiles };

		enum PictureFlags { NoFlags = 0, Repeating = 1 };

	public:
		static ShadowPainter *self();
		~ShadowPainter ();

		// Sets the shadow radius
		static void setShadowRadius( double radius )
		{
			self()->setRadius( radius );
		}

		// Sets the opacity used when drawing shadows
		static void setShadowOpacity( double opacity )
		{
			self()->setOpacity( opacity );
		}

		// Draws a shadow in destination with the specified geometry.
		// The opacity parameter is relative to the opacity set with setShadowOpacity().
		static void drawShadow( Picture destination, const QRect &r, double opacity = 1.0 )
		{
			self()->draw( destination, r, opacity );
		}

		// Draws shadow pictures created by createShadowPicture().
		// The opacity parameter is relative to the opacity set with setShadowOpacity().
		static void drawShadowPicture( Picture destination, const QRect &rect,
									   Picture shadow, double opacity = 1.0 )
		{
			self()->draw( destination, rect, shadow, opacity );
		}

		// Creates a shadow picture of the specified size.
		static Picture createShadowPicture( const QSize &size )
		{
			return self()->createShadow( size );
		}

		// Calculates the shadow geometry for a given window geometry
		static QRect shadowGeometry( const QRect &r )
		{
			int size = self()->size;
			return QRect( r.x() - size / 2, r.y() - size / 2, r.width() + size, r.height() + size );
		}

	private:
		ShadowPainter ();

		void setRadius( double radius );
		void setOpacity( double opacity );
		void draw( Picture destination, const QRect &r, double opacity = 1.0 );
		void draw( Picture destination, const QRect &r, Picture shadow, double opacity = 1.0 );
		Picture createShadow( const QSize &size );
		double gaussian( double x, double y );
		unsigned char sum (double *kernel, int x, int y);
		Picture createPicture( Picture source, int x, int y,
			   int width, int height, int flags = NoFlags );
		Picture createPicture( int width, int height, int flags = NoFlags );
		void createTiles ();
		void blendTile( Tile, Picture, int x, int y);
		void blendTile( Tile, Picture, int x, int y, int width, int height);
		void setupForOpacity( double opacity );
	
		Picture tiles[NTiles];
		Picture source;
		double radius;
		double shadowOpacity;
		double lastOpacity;
		int size;
		bool tilesValid;

		static ShadowPainter *s_self;
};

#endif
