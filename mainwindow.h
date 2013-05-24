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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <cassert>
#include <QMainWindow>
#include <QGLWidget>
#include "map.h"
#include "compass.h"
#include "view_gl.h"

#define MAX_PLAYERS  32  // Limitado pelo numero de bits em Cell::players


namespace Ui
    {
    class MainWindow;
    }


class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    // Posicoes (indices) dos separadores
    enum Tab { TAB_MAP_INDEX   = 0,
               TAB_FP2D_INDEX  = 1,
               TAB_FP3D_INDEX  = 2,
               TAB_ABOUT_INDEX = 3 };

    MainWindow( QWidget *parent = NULL );
    ~MainWindow();

    void resetStateNew();
    void resetStatePlay();
    void requestNewMaze();
    void buildMaze( int width, int height, int complexity );

protected:
    void changeEvent( QEvent *e );
    void turnLeft();
    void turnRight();
    void moveFwd();
    void draw2d();
    void draw3d( bool animated = false );

private:
    Ui::MainWindow *ui;
    Map *map;
    struct Player { int x, y; Compass compass; } players[ MAX_PLAYERS ];
    int num_players;
    int current_player;

private slots:
    void on_pushTurnRight3D_clicked();
    void on_pushMoveFwd3D_clicked();
    void on_pushTurnLeft3D_clicked();
    void on_pushTurnRight2D_clicked();
    void on_pushMoveFwd2D_clicked();
    void on_pushTurnLeft2D_clicked();
    void on_tabSet_currentChanged( int index );
    void on_pushNew_clicked();
};


#endif // MAINWINDOW_H
