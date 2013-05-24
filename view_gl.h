/*
Labirinto para Computacao Grafica (CG).
(C) 2010-2013 Pedro Freire (www.pedrofreire.com)
    Licenca Creative Commons:
    Atribuicao - Uso Nao-Comercial - Partilha nos termos da mesma Licenca
    http://creativecommons.org/licenses/by-nc-sa/2.5/pt/

Computer Graphics Maze.
(C) 2010-2013 Pedro Freire (www.pedrofreire.com)
    All comments in Portuguese.
    Creative Commons license:
    Attribution - Noncommercial - Share Alike
    http://creativecommons.org/licenses/by-nc-sa/2.5/pt/deed.en_US
*/


#ifndef VIEW_GL_H
#define VIEW_GL_H

#include <queue>
#include <cassert>
#include <QGLWidget>
#include <QFrame>
#include <QTimer>
#include "student_viewmap.h"
#include "student_view2d.h"
#include "student_view3d.h"
#include "map.h"
#include "cell.h"
#include "compass.h"

#define MAX_PLAYERS   32  // Limitado pelo numero de bits em Cell::players
#define VIEW3DGL_FPS  50  // Quadros por segundo (frames per second) na animacao


class ViewMapGL : public QGLWidget
{
    Q_OBJECT  // Necessario devido aos signals+slots usados pelo QTimer
public:
    ViewMapGL( QFrame *&parent );
    ~ViewMapGL();
    void setMap( Map *map, int seconds_to_completion = 10 );
    void paintGL();
    void paintGL( int x, int y, Cell c )
        { assert( map != NULL  &&  viewmap != NULL  && !map->badCoord(x,y) );
          makeCurrent();  viewmap->paint(x,y,c); }
        // Como ambas as paintGL() sao publicas, usamos makeCurrent() caso sejam
        // chamadas por uma funcao que nao o metodo glDraw() da Qt
protected:
    Map *map;
    ViewMap *viewmap;  // Classe definida pelos alunos
    int view_width, view_height;
    struct {
        int wait_ms;
        int cells_per_sec;
        std::queue<int> paint_queue; // push(x) e push(y) para cada chamada a setCell()
        QTimer *paint_timer;
        } delayed;
    void initializeGL();
    void resizeGL( int view_width, int view_height );
private slots:
    void mapClearGL();
    void mapPaintGL( int x, int y, Cell c, bool animated = false );
    void timerPaintGL();
};


class View2DGL : public QGLWidget
{
public:
    View2DGL( QFrame *&parent );
    ~View2DGL();
    void setMap( Map *map );
    void paintGL()
        { assert(view2d != NULL);  makeCurrent();  view2d->paint(x,y,compass); }
    void paintGL( int x, int y, Compass compass )
        { this->x = x;  this->y = y;  this->compass = compass;  glDraw(); }
        // Como a paintGL() e' publica, usamos makeCurrent() caso seja
        // chamada por uma funcao que nao o metodo glDraw() da Qt
protected:
    Map *map;
    int x, y;
    Compass compass;
    View2D *view2d;  // Classe definida pelos alunos
    int view_width, view_height;
    void initializeGL();
    void resizeGL( int view_width, int view_height )
        { this->view_width = view_width;  this->view_height = view_height;
          assert(view2d != NULL);  view2d->resize(view_width, view_height); }
};


class View3DGL : public QGLWidget
{
    Q_OBJECT  // Necessario devido aos signals+slots usados pelo QTimer
public:
    View3DGL( QFrame *&parent );
    ~View3DGL();
    void setMap( Map *map );
    void paintGL()
        { assert(view3d != NULL);  makeCurrent();  view3d->paint(x,y,compass_direction); }
    void paintGL( float x, float y, float compass_direction, float seconds_to_completion = 0.0f );
    // Como a paintGL() e' publica, usamos makeCurrent() caso seja
    // chamada por uma funcao que nao o metodo glDraw() da Qt
protected:
    Map *map;
    float x, y, compass_direction;
    struct {
        float from_x, from_y, from_compass_direction;
        float   to_x,   to_y,   to_compass_direction;
        float step, complete;
        QTimer *paint_timer;
        } transition;
    View3D *view3d;  // Classe definida pelos alunos
    int view_width, view_height;
    void initializeGL();
    void resizeGL( int view_width, int view_height )
        { this->view_width = view_width;  this->view_height = view_height;
          assert(view3d != NULL);  view3d->resize(view_width, view_height); }
private slots:
    void timerPaintGL();
};


#endif // VIEW_GL_H
