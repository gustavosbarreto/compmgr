/*
 * $Id$
 *
 * KDE Composite Manager
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

#include "main.h"
#include "workspace.h"

#include <X11/Xlib.h>

Atom net_wm_window_opacity;
Atom net_current_desktop;

Atom xa_xrootpmap_id;
Atom xa_xsetroot_id;
Atom xa_esetroot_pmap_id;

Display *dpy;

static int error( Display *, XErrorEvent * )
{
	return 0;
}

static void register_cm()
{
    Window w;
    Atom a;

    w = XCreateSimpleWindow (dpy, RootWindow (dpy, 0), 0, 0, 1, 1, 0, None,
			     None);

    Xutf8SetWMProperties (dpy, w, "xcompmgr", "xcompmgr", NULL, 0, NULL, NULL,
			  NULL);

    a = XInternAtom (dpy, "_NET_WM_CM_S0", False);

    XSetSelectionOwner (dpy, a, w, 0);
}

int main(/* int argc, char **argv*/)
{
	dpy = XOpenDisplay( 0 );

	const char *names[5];
	Atom atoms_return[5];
	Atom *atoms[5];
	int n = 0;

	names[n]   = "_NET_WM_WINDOW_OPACITY";
	atoms[n++] = &net_wm_window_opacity;

	names[n]   = "_NET_CURRENT_DESKTOP";
	atoms[n++] = &net_current_desktop;

	names[n]   = "_XROOTPMAP_ID";
	atoms[n++] = &xa_xrootpmap_id;

	names[n]   = "ESETROOT_PMAP_ID";
	atoms[n++] = &xa_esetroot_pmap_id;

	names[n]   = "_XSETROOT_ID";
	atoms[n++] = &xa_xsetroot_id;

	XInternAtoms( dpy, (char**)names, n, false, atoms_return );
	for ( int i = 0; i < n; i++ )
		*atoms[i] = atoms_return[i];	

	XSetErrorHandler (error);

	register_cm();

	Workspace *workspace = Workspace::instance();

	while (true)
	{
		XEvent event;
		XNextEvent( dpy, &event );
		workspace->x11Event( &event );
	}

	delete workspace;
	XCloseDisplay (dpy);
}

