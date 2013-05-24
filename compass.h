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


#ifndef COMPASS_H
#define COMPASS_H

// **** Ficheiro independente para evitar referencia circular
// **** entre map.h e student_mapdraw.h


/* Definicao dos valores para os pontos cardeais
   (da bussula, "compass" em Ingles).
   Definidos por ordem dos ponteiros do relogio, comecando em Sul. Isto e'
   a mesma ordem pela qual um QDial exibe visualmente os pontos cardeais.
   Por coincidencia, os pontos que definem uma orientacao horizontal te^m
   valores impares, e os que definem uma orientacao vertical te^m valores
   pares.
*/

class Compass
{
public:
    enum Direction { SOUTH = 0, WEST = 1, NORTH = 2, EAST = 3 };

private:
    Direction direction;

public:
    Compass( Direction d = NORTH )  { this->direction = d; }

    Direction getDirection()	           { return this->direction; }
    void      setDirection( Direction d )  { this->direction = (Direction) ((int)d & 3); }

    bool isHoriz()    { return  (bool) ((int)this->direction & 1); }
    bool isVert()     { return !(bool) ((int)this->direction & 1); }

    Compass turnLeft()   { this->direction = (Direction) (((int)this->direction - 1) & 3);  return *this; }
    Compass turnRight()  { this->direction = (Direction) (((int)this->direction + 1) & 3);  return *this; }
    Compass turnBack()   { this->direction = (Direction) (((int)this->direction + 2) & 3);  return *this; }
};


#endif // COMPASS_H
