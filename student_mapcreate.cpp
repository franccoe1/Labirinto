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


#include <stack>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include "student_mapcreate.h"


namespace MapCreate
{

/* Esta funcao e' chamada quando o utilizador carrega em "OK" apos escolher o
   tamanho do labirinto, devendo gerar o labirinto em si. Ela assume que "map"
   vem com um mapa nao vazio e ja' preenchido com "chao" em todas as posicoes.
   Ela deve chamar map->setCell() para ir escrevendo as paredes. Sugere-se que
   tambem use map->getCell(), map->move*() e/ou Compass::turn*() para a ajudar
   a atingir este objectivo.

   Restricoes (verificadas no assert(), abaixo):
        map != NULL
        map->getWidth()  >= 1
        map->getHeight() >= 1
        0 < complexity <= 100
*/
void walls( Map *map, int complexity )
{
    static const Cell wall = Cell(Cell::WALL);
    Cell c;
    std::stack<int> vertex;  // push(x), push(y); pop(y), pop(x)
    int x, y, x2, y2, w, h, w1, h1, wh, r;
    Compass compass = Compass( Compass::EAST );  // Para a direita
    Compass::Direction avail_dirs[4];  // Direccoes disponiveis
    int num_avail_dirs, d;

    // Verificoes basicas
    assert( map != NULL  &&  complexity > 0  &&  complexity <= 100  &&
            map->getWidth()  >= 1  &&
            map->getHeight() >= 1 );

    // Desenhar a borda lateral
    w = map->getWidth();   w1 = w - 1;
    h = map->getHeight();  h1 = h - 1;
    wh = w + h;
    for( r = 0;  r < wh;  r++ )
        {
        if( r < w )
            map->setCell(   r,       h1, wall );  // Parede de cima
        else
            map->setCell(  w1, h1-(r-w), wall );  // Parede da direita

        if( r < h )
            map->setCell(   0,     h1-r, wall );  // Parede da esquerda
        else
            map->setCell( r-h,        0, wall );  // Parede de baixo
        }
    if( w <= 3  ||  h <= 3 )
        return;

    srand( (unsigned int) time(NULL) );
        // Assegurar-me que os labirintos sao *sempre* diferentes
    x = y = 2;  // Canto inferior direito, dentro da borda lateral

    // Comeca a "pintar" as paredes no canto inicial
    map->setCell( x, y, wall );
    vertex.push( x );
    vertex.push( y );

    for(;;)
        {
        // Escolhe uma (nova) direccao onde se mover: para tras pode tambem ser
        // uma opcao porque podemos ter voltado a um vertice anterior
        num_avail_dirs = 0;
        for( d = 0;  d < 4;  d++ )
            {
            x2 = x;
            y2 = y;
            map->moveFwd( &x2, &y2, compass );
            map->moveFwd( &x2, &y2, compass );
            if( map->isFloor(x2, y2) )
                avail_dirs[num_avail_dirs++] = compass.getDirection();
            compass.turnRight();
            }
        // Verificacao de teste
        assert( num_avail_dirs >= 0  &&  num_avail_dirs < 4 );

        // Se nao nos pudermos virar para lado nenhum, estamos num beco sem
        // saida! Voltar atras ao ultimo vertice e repetir o processo...
        if( num_avail_dirs <= 0 )
            {
            if( vertex.size() < 2 )
                {
                assert( vertex.empty() );  // Verificacao de teste
                break;  // Nao temos mais vertices para testar: acabamos!
                }
            y = vertex.top();
            vertex.pop();
            x = vertex.top();
            vertex.pop();
            continue;
            }

        if( num_avail_dirs > 1 )
            {
            // Se temos mais de uma direccao disponivel, guarda ainda este
            // vertice para voltarmos mais tarde
            vertex.push( x );
            vertex.push( y );
            // Escolher aleatoriamente uma das direccoes disponiveis
            r = rand() % num_avail_dirs;  // r = numero aleatorio entre 0 e num_avail_dirs-1
            compass.setDirection( avail_dirs[r] );
            }
        else
            compass.setDirection( avail_dirs[0] );
            // Escolher a unica direccao disponivel

        // Faz paredes nessa direccao ate' nao poder avancar, ou aleatoriamente
        // decidir mudar de direccao
        do  {
            // Escreve uma parede nessa direccao
            map->moveFwd( &x, &y, compass );
            map->setCell(  x,  y, wall );
            map->moveFwd( &x, &y, compass );
            map->setCell(  x,  y, wall );

            // r = "urgencia" de se mudar de direccao (mais perto de 0 = mais urgente)
            r = rand() % 100;  // r = numero aleatorio entre 0 e 99

            // Testa a posicao 'a frente
            x2 = x;
            y2 = y;
            map->moveFwd( &x2, &y2, compass );
            map->moveFwd( &x2, &y2, compass );
            if( map->isWallOrDoor(x2, y2) )
                r = 0;  // Maxima "urgencia"
            }
            while( complexity < r );

        // Coloca a posicao actual na pilha para voltar para ca e testar
        // novas direccoes.
        vertex.push( x );
        vertex.push( y );
        }
}


/* Cria agora as "caracteristicas" de jogo do mapa (objectos nos corredores,
   portas e outras). Esta funcao e' chamada imediatamente a seguir a walls().
*/
void features( Map *map )
{
    int x, y, w, h, f, numf, corridor_vert, corridor_horiz;
    Cell c;

    // Verificoes basicas
    assert( map != NULL );

    srand( (unsigned int) time(NULL) );
        // Assegurar-me que os labirintos sao *sempre* diferentes

    w = map->getWidth();
    h = map->getHeight();
    numf = (w * h) / 30;  // a cada 30 celulas ha' uma nova caracteristica
    for( f = 0;  f < numf;  f++ )
        {
        switch( f )
            {
            case 0:
                x = 2;
                y = 3;
                c = map->getCell( x, y );
                break;
            case 1:
                x = 3;
                y = 1;
                c = map->getCell( x, y );
                break;
            default:
                do  {
                    x = 1 + (rand() % (w-2));  // x = numero aleatorio entre 1 e map->getWidth()-2
                    y = 1 + (rand() % (h-2));  // y = numero aleatorio entre 1 e map->getHeight()-2
                    c = map->getCell( x, y );
                    }
                    while( c.object != 0  ||  c.isDoor() );
                break;
            }

        if( c.isWall() )
            {
            // "Caracteristica" da parede: porta ou lampada
            // Primeiro verifica se a parede esta' a meio de um corredor
            // (em vez de ser no fim ou ser uma esquina)
            corridor_vert  = ( map->getCell(x-1, y).isFloor()  &&
                               map->getCell(x+1, y).isFloor()  &&
                               map->getCell(x, y+1).isWall()   &&
                               map->getCell(x, y-1).isWall() );
            corridor_horiz = ( map->getCell(x-1, y).isWall()   &&
                               map->getCell(x+1, y).isWall()   &&
                               map->getCell(x, y+1).isFloor()  &&
                               map->getCell(x, y-1).isFloor() );
            if( (rand() & 1) == 0  &&  (corridor_vert || corridor_horiz) )
                {
                c.type = Cell::DOOR_CLOSED;
                c.object = ( corridor_vert ? OBJ_DOOR_HORIZ : OBJ_DOOR_VERT );
                }
            else
                c.object = OBJ_WALL_LIGHT;
            }
        else
            {
            // "Caracteristica" do chao: buraco ou chave
            if( (rand() & 1) == 0 )
                c.object = OBJ_FLOOR_PIT;
            else
                c.object = OBJ_FLOOR_KEY;
            }
        map->setCell( x, y, c );
        }
}

}

