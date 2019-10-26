# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# these are the headers for your project
noinst_HEADERS = qaudioplayer.h qaudiodg2.h qinterp.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/qaudioplayer.pot

KDE_ICON = qaudioplayer

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = qaudioplayer

# the application source, library search path, and link libraries
qaudioplayer_SOURCES = main.cpp qaudioplayer.cpp qaudiodg.ui qaudiodg2.cpp qinterp.cpp
qaudioplayer_LDFLAGS = $(KDE_RPATH) $(all_libraries)
qaudioplayer_LDADD = -lqint $(LIB_KDEUI) $(LIB_KFILE)

# this is where the desktop file will go 
shelldesktopdir = $(kde_appsdir)/Multimedia
shelldesktop_DATA = qaudioplayer.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir = $(kde_datadir)/qaudioplayer
shellrc_DATA = qaudioplayerui.rc qaudioplayer.q

