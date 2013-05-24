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


#ifndef MAPCREATE_H
#define MAPCREATE_H

#include "cell.h"
#include "compass.h"
#include "map.h"


namespace MapCreate
{
    /* Gera um novo mapa (paredes e "caracteristicas" de jogo), assumindo que
       "map" esta' "apagado" para ser so' chao (Cell::FLOOR).
    */
    void walls   ( Map *map, int complexity = 20 );


    /* Gera as caracteristicas de jogo apos a criacao do mapa.
    */
    void features( Map *map );
}


#endif // MAPCREATE_H
