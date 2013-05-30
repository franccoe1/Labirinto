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

=============================================================================
#### Podes alterar este ficheiro ############################################
=============================================================================
*/


#ifndef VIEW2D_H
#define VIEW2D_H

#include <QtOpenGL>
#include "map.h"
#include "compass.h"

// Isto e' quer a largura quer a altura, porque vamos usar um quadrado
// (em termos de coordenadas) e o OpenGL faz a conversao para o rectangulo
// visivel.
#define VIEW2D_SIZE    100.0

// Estas coordenadas (com o 0,0 ao centro) facilitam o calculo das coordenadas
// da vista, ja' que te^m uma simetria ao centro
#define VIEW2D_LEFT    (-VIEW2D_SIZE / 2.0)
#define VIEW2D_RIGHT   ( VIEW2D_SIZE / 2.0)
#define VIEW2D_TOP     ( VIEW2D_SIZE / 2.0)
#define VIEW2D_BOTTOM  (-VIEW2D_SIZE / 2.0)

// Numero de celulas do mapa desenhadas no ecra
#define VIEW2D_DEPTH   100

// Cores
#define VIEW2D_COLOR_3UB_CEILING       0xBD, 0xBE, 0xC0
#define VIEW2D_COLOR_3UB_WALL          0xB8, 0xB8, 0xF0  // 0x99, 0x99, 0xCB
#define VIEW2D_COLOR_3UB_WALL_OUTLINE  0x99, 0x99, 0xCB
#define VIEW2D_COLOR_3UB_DOOR_OPEN     VIEW2D_COLOR_3UB_WALL  // Nao usada
#define VIEW2D_COLOR_3UB_DOOR_CLOSED   VIEW2D_COLOR_3UB_WALL  // Nao usada
#define VIEW2D_COLOR_3UB_FLOOR         0xB1, 0x89, 0x4E  // 0x66, 0x4F, 0x2D
#define VIEW2D_COLOR_3UB_PLAYER1       0xFF, 0x33, 0x33  // Nao usada
#define VIEW2D_COLOR_3UB_CHAVE         0xFF, 0xFF, 0xFF
#define VIEW2D_COLOR_3UB_BURACO        0xFF, 0x33, 0x33

class View2D
{
private:
    Map *map;
    void drawWall( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 );

public:
    View2D( Map *map );  // Atencao que "map" pode ser NULL!
    void resize( int view_width, int view_height );
    void paint( int x, int y, Compass compass );
};


#endif // VIEW2D_H
