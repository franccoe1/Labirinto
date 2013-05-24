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


#include <cassert>
#include <vector>
#include <QtGlobal>
#include "map.h"
#include "cell.h"
#include "compass.h"


/* Construtor
   Nao permite que a classe seja criada com um labirinto de tamanho invalido.
*/
Map::Map( int width, int height )
{
    // Verificacao basica
    assert( width >= 1  &&  height >= 1 );

    this->width  = width;
    this->height = height;
    this->map    = new Cell[ width * height ];

    // O construtor Cell() ja' limpou todas as celulas para serem FLOOR;
    // Agora limpo a area de desenho do mapa
    emit clearedMap();
}


/* Destrutor
   Assegurar-me que liberto a memoria alocada pelo construtor.
*/
Map::~Map()
{
    // Verificacao basica
    assert( map != NULL );
    delete( map );
}


/* Metodo privado que escreve c numa celula do labirinto nas coordenadas x,y
   representa essa escrita visualmente, de forma animada (lenta) se animated
   for true.
*/
void Map::writeCell( int x, int y, Cell c, bool animated )
{
    // Verificacao basica
    assert( map != NULL );

    if( badCoord(x,y) )
        {
        qCritical( "writeCell(%d,%d,...) out of maze bounds! (width=%d, height=%d)",
                   x, y, width, height );
        return;  // Ignorar escritas fora dos limites do labirinto
        }
    map[ convCoord(x,y) ] = c;
    emit wroteCell( x, y, c, animated );
}


/* Alteram o estado de uma porta.
*/
void Map::setDoorOpen( int x, int y )
{
    Cell c = getCell( x, y );
    if( c.type == Cell::DOOR_CLOSED )
        {
        c.type = Cell::DOOR_OPEN;
        writeCell( x, y, c, false );
            // Nao fazer animacao ao alterar o estado da porta
        }
}

void Map::setDoorClose( int x, int y )
{
    Cell c = getCell( x, y );
    if( c.type == Cell::DOOR_OPEN )
        {
        c.type = Cell::DOOR_CLOSED;
        writeCell( x, y, c, false );
            // Nao fazer animacao ao alterar o estado da porta
        }
}

void Map::setDoorToggle( int x, int y )
{
    Cell c = getCell( x, y );
    if( c.isDoor() )
        {
        c.type = ( c.type == Cell::DOOR_OPEN ? Cell::DOOR_CLOSED : Cell::DOOR_OPEN );
        writeCell( x, y, c, false );
            // Nao fazer animacao ao alterar o estado da porta
        }
}


/* Marca uma celula como tendo o objecto "obj".
   O objecto e' o inteiro definido pelo utilizador que se pode associar a cada
   celula. Note-se que *todo* esse inteiro e' *substituido* por "obj".
*/
void Map::setObject( int x, int y, int obj )
{
    Cell c = getCell( x, y );
    c.object = obj;  // Assegurar-me que so' vou alterar o objecto
    writeCell( x, y, c, false );  // Nao fazer animacao ao alterar o objecto
}


/* Marca uma celula como tendo o jogador "player" ja' passado por la'.
   Lembra-te que "player" e' uma mascara de bits, onde o bit 0 representa o
   jogador 1, o bit 1 representa o jogador 2, etc.. Se so' tens um jogador
   no teu jogo, usa sempre "1U" para "player".
*/
void Map::setPlayer( int x, int y, unsigned int player )
{
    Cell c = getCell( x, y );
    c.players |= player;  // Assegurar-me que so' vou alterar os bits dos jogadores
    writeCell( x, y, c, false );  // Nao fazer animacao ao alterar os jogadores
}


/* Actualiza as coordenadas para avancar uma posicao na direccao pretendida
   em "c". Nao verifica se as coordenadas ficam dentro do labirinto ou nao!
*/
void Map::moveFwd( int *px, int *py, Compass c )
{
    // Verificacoes basicas
    assert( px != NULL  &&  py != NULL );
    switch( c.getDirection() )
        {
        case Compass::SOUTH:  (*py)--;  break;
        case Compass::WEST:   (*px)--;  break;
        case Compass::NORTH:  (*py)++;  break;
        case Compass::EAST:   (*px)++;  break;
        default:              break;
        }
}




// ==========================================================================
// #### Codigo antigo  ######################################################
// ==========================================================================


/* Este foi codigo que usava varios bits de um inteiro para definicao de
   "Cell". Foi revisto para se passar a usar uma estrutura, o que era mais
   facil de entender e codificar. Mantenho aqui o codigo para que vejas
   formas alterativas (e mais eficientes, apesar de serem mais complexas)
   de codificar estas coisas.

/+ Definicao dos valores para as celulas do mapa do labirinto.
   Sao valores inteiros, combinados (com "|" ou "+") com inteiros
   definidos pelo utilizador, se este o desejar. O inteiro definido pelo
   utilizador devera' estar entre 0x00 e 0xFF inclusive' e pode ser
   ajustado apos criacao do labirinto com setCellObject().
   Os bits de 8 a 23 podem ser usados para descrever se ate' 16 jogadores
   ja' visitaram aquela celula (1 bit por jogador). As constantes
   JOGADOR* ja' definidas (assim como as funcoes setCellPlayer()), ajudam
   a marcar essas celulas.
   NOTA IMPORTANTE: Apesar das facilidades existentes nesta classe, neste
   momento o labirinto e' usado apenas com:
	FLOOR, WALL e PLAYER1
+/
enum Cell { FLOOR  = 0x0000000,  DOOR_OPEN   = 0x1000000,
            WALL   = 0x2000000,  DOOR_CLOSED = 0x3000000,
            PLAYER1 = 0x000100,  PLAYER7  = 0x004000,  PLAYER12 = 0x080000,
            PLAYER2 = 0x000200,  PLAYER8  = 0x008000,  PLAYER13 = 0x100000,
            PLAYER3 = 0x000400,  PLAYER9  = 0x010000,  PLAYER14 = 0x200000,
            PLAYER4 = 0x000800,  PLAYER10 = 0x020000,  PLAYER15 = 0x400000,
            PLAYER5 = 0x001000,  PLAYER11 = 0x040000,  PLAYER16 = 0x800000,
            PLAYER6 = 0x002000 };

bool isWall        ( int x, int y )  { return ((int)map[convCoord(x,y)] & 0x3000000) == WALL; }
bool isWallOrClosed( int x, int y )  { return  (bool) ((int)map[convCoord(x,y)] & 0x2000000); }
bool isWallOrDoor  ( int x, int y )  { return ((int)map[convCoord(x,y)] & 0x3000000) != FLOOR; }
bool isFloor       ( int x, int y )  { return ((int)map[convCoord(x,y)] & 0x3000000) == FLOOR; }
bool isFloorOrOpen ( int x, int y )  { return !(bool) ((int)map[convCoord(x,y)] & 0x2000000); }
bool isFloorOrDoor ( int x, int y )  { return ((int)map[convCoord(x,y)] & 0x3000000) != WALL; }
bool isDoor        ( int x, int y )  { return  (bool) ((int)map[convCoord(x,y)] & 0x1000000); }

void setCell( int x, int y, Cell c )  { writeCell(x, y, c, true); }
  // IMPORTANTE: Este metodo faz uma animacao apos desenhar a celula, pelo
  // que apenas deve ser chamado *durante* a criacao do labirinto
Cell getCell( int x, int y )          { return map[convCoord(x,y)]; }
Cell getCellPlayer( int x, int y )    { return (int) map[convCoord(x,y)] & 0xFFFF00; }
int  getCellObject( int x, int y )    { return (int) map[convCoord(x,y)] & 0xFF; }

setCellPlayer( int x, int y,  int player );
setCellPlayer( int x, int y, Cell player = PLAYER1 );
setCellObject( int x, int y,  int obj );


/+ Marca uma celula como tendo o jogador "player" ja' passado por la'.
   Nesta versao da funcao, o 3.o argumento e' 0 para Cell::PLAYER1,
   1 para Cell::PLAYER2, etc.
   Atencao que apenas podem existir 16 jogadores pelo que "player" e' alterado
   com um "& 0xF";
+/
Map::setCellPlayer( int x, int y, int player )
{
    Cell c = map[ convCoord(x,y) ];
    c = (Cell) (  (int) c  |  ( 1 << ((player & x0F) + 8) )  );
      // Assegurar-me que so' vou alterar os bits dos jogadores
    writeCell( x, y, c, false );  // Nao fazer animacao ao alterar os jogadores
}


/+ Marca uma celula como tendo o jogador "player" ja' passado por la'.
   Nesta versao da funcao, o 3.o argumento e' Cell::PLAYER*.
+/
Map::setCellPlayer( int x, int y, Cell player )
{
    Cell c = map[ convCoord(x,y) ];
    c = (Cell) (  (int) c  |  ( ((int) player) & 0xFFFF00 )  );
      // Assegurar-me que so' vou alterar os bits dos jogadores
    writeCell( x, y, c, false );  // Nao fazer animacao ao alterar os jogadores
}


/+ Marca uma celula como tendo o objecto "obj".
   O objecto e' o inteiro de 8 bits definido pelo utilizador que se pode
   associar a cada celula. Note-se que *todo* esse inteiro e' *substituido* por
   "obj".
   Atencao que "obj" apenas pode ter valores de 0x00 a 0xFF pelo que "obj" e'
   alterado com um "& 0xFF";
+/
Map::setCellObject( int x, int y, int obj )
{
    Cell c = map[ convCoord(x,y) ];
    c = (Cell) (  ((int) c & ~0xFF)  |  (obj & 0xFF)  );
      // Assegurar-me que so' vou alterar os bits do objecto
    writeCell( x, y, c, false );  // Nao fazer animacao ao alterar o objecto
}

*/
