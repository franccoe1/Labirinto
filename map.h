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


#ifndef MAP_H
#define MAP_H

#include <cassert>
#include <QObject>
#include "cell.h"
#include "compass.h"


class Map : public QObject
{
    Q_OBJECT
    // Derivacao de QObject e macro Q_OBJECT necessarios devido aos
    // signals emitidos a favor do view_gl.cpp
private:
    int width;
    int height;
    Cell *map;

    /* Repare-se no sistema de coordenadas usado:
	    x = coluna do mapa do labirinto, sendo a coluna 0 a da esquerda;
	    y = linha do mapa do labirinto, sendo a linha 0 a de baixo;
       Este sistema de coordenadas e' identico ao sistema base do glViewport
       do OpenGL.
       Detalhe: E' VALIDO LER FORA DOS LIMITES DO LABIRINTO. Nesse caso todos
       os metodos agem como se fora estivessem sempre paredes sem objectos e
       sem jogadores. Apesar do programa nao arrebentar se tentar escrever fora
       dos limites do labirinto, e' escrita uma mensagem de erro na consola.
    */
    int convCoord( int x, int y )  { return (y * width) + x; }
	// Converte uma coordenada x,y num indice dentro do vector map
    void writeCell( int x, int y, Cell c, bool animated = false );

public:
    Map( int width, int height );
    ~Map();
    bool badCoord( int x, int y )
        { return x < 0 || x >= width || y < 0 || y >= height; }
        // True se as coordenadas estiverem fora dos limites do labirinto

    int getWidth()   { return width;  }
    int getHeight()  { return height; }

    void setCell( int x, int y, Cell c )  { writeCell(x, y, c, true); }
      // IMPORTANTE: Este metodo faz uma animacao apos desenhar a celula, pelo
      // que apenas deve ser chamado *durante* a criacao do labirinto
    Cell getCell( int x, int y )
        { assert(map != NULL);  return badCoord(x,y) ? Cell(Cell::WALL) : map[convCoord(x,y)]; }

    void setDoorOpen  ( int x, int y );
    void setDoorClose ( int x, int y );
    void setDoorToggle( int x, int y );
    void setObject    ( int x, int y, int obj );
    void setPlayer    ( int x, int y, unsigned int player = 1U );

    bool isWall        ( int x, int y )  { return getCell(x,y).isWall();         }
    bool isWallOrClosed( int x, int y )  { return getCell(x,y).isWallOrClosed(); }
    bool isWallOrDoor  ( int x, int y )  { return getCell(x,y).isWallOrDoor();   }
    bool isFloor       ( int x, int y )  { return getCell(x,y).isFloor();        }
    bool isFloorOrOpen ( int x, int y )  { return getCell(x,y).isFloorOrOpen();  }
    bool isFloorOrDoor ( int x, int y )  { return getCell(x,y).isFloorOrDoor();  }
    bool isDoor        ( int x, int y )  { return getCell(x,y).isDoor();         }

    void moveFwd  ( int *px, int *py, Compass c );
    void moveLeft ( int *px, int *py, Compass c )  { moveFwd(px, py, c.turnLeft() ); }
    void moveRight( int *px, int *py, Compass c )  { moveFwd(px, py, c.turnRight()); }
    void moveBack ( int *px, int *py, Compass c )  { moveFwd(px, py, c.turnBack() ); }

signals:
    void clearedMap();
    void wroteCell( int x, int y, Cell c, bool animated = false );
};


#endif // MAP_H
