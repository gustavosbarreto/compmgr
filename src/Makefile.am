INCLUDES= $(all_includes)
KDE_CXXFLAGS = -g3

bin_PROGRAMS = kcompmgr
kcompmgr_SOURCES = main.cpp shadowpainter.cpp workspace.cpp client.cpp
kcompmgr_PCH     = kcompmgr_pch.h
kcompmgr_LDFLAGS = $(all_libraries) -L/opt/kdrive/lib $(KDE_RPATH)
kcompmgr_LDADD   = $(LIB_KDECORE) -lXfixes -lXcomposite -lXdamage
kcompmgr_METASOURCES =  AUTO
noinst_HEADERS = main.h workspace.h client.h shadowpainter.h kcompmgr_pch.h

## Messages
messages:
	$(XGETTEXT) $(kcompmgr_SOURCES) -o $(podir)/kcompmgr.pot

