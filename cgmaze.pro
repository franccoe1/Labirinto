# Labirinto para Computacao Grafica (CG).
# (C) 2010-2013 Pedro Freire (www.pedrofreire.com)
#     Licenca Creative Commons:
#     Atribuicao - Uso Nao-Comercial - Partilha nos termos da mesma Licenca
#     http://creativecommons.org/licenses/by-nc-sa/2.5/pt/
#
# Computer Graphics Maze.
# (C) 2010-2013 Pedro Freire (www.pedrofreire.com)
#     All comments in Portuguese.
#     Creative Commons license:
#     Attribution - Noncommercial - Share Alike
#     http://creativecommons.org/licenses/by-nc-sa/2.5/pt/deed.en_US
#
# -------------------------------------------------
# Project created by QtCreator 2010-02-26T16:00:24
# -------------------------------------------------

QT += opengl \
    multimedia
TARGET = cgmaze
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    dialognew.cpp \
    map.cpp \
    student_mapcreate.cpp \
    student_view3d.cpp \
    student_view2d.cpp \
    student_viewmap.cpp \
    view_gl.cpp
HEADERS += mainwindow.h \
    dialognew.h \
    cell.h \
    compass.h \
    map.h \
    student_mapcreate.h \
    student_view3d.h \
    student_view2d.h \
    student_viewmap.h \
    view_gl.h
FORMS += mainwindow.ui \
    dialognew.ui
RESOURCES += imgs.qrc
RC_FILE = main.rc

unix|win32: LIBS += -lGLU32
