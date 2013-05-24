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


#include <cassert>
#include <cmath>
#include <QtOpenGL>
#include "student_view2d.h"
#include "map.h"
#include "cell.h"
#include "compass.h"


/* Construtor
   Atencao que "map" pode ser NULL!
*/
View2D::View2D( Map *map )
{
    this->map = map;

    // Inicializar OpenGL
    glShadeModel( GL_SMOOTH );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );  // Fundo preto
    glDisable( GL_DEPTH_TEST );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
#ifdef QT_OPENGL_ES_1
    glOrthof( (GLfloat)VIEW2D_LEFT,  (GLfloat)VIEW2D_RIGHT,  (GLfloat)VIEW2D_BOTTOM,  (GLfloat)VIEW2D_TOP, 0.0f, 1.0f );
#else
    glOrtho( (GLdouble)VIEW2D_LEFT, (GLdouble)VIEW2D_RIGHT, (GLdouble)VIEW2D_BOTTOM, (GLdouble)VIEW2D_TOP, 0.0,  1.0  );
#endif

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}


/* Metodo chamado quando a janela do jogo e' redimensionada, pelo que deve ser
   usado para (re-)configurar o OpenGL para uma nova dimensao da janela do
   jogo (ve^ glViewport()).
   Tambem e' chamado no arranque, logo apos o construtor.
   Atencao que "this->map" pode ser NULL!
*/
void View2D::resize( int view_width, int view_height )
{
    if( view_width  < 1 )  view_width  = 1;
    if( view_height < 1 )  view_height = 1;

    glViewport( 0, 0, view_width, view_height );
}


/* Desenha a vista com centro em x,y e orientacao compass.
   Atencao que "this->map" pode ser NULL!
*/
void View2D::paint( int x, int y, Compass compass )
{
    int n;
    GLfloat wall_x1, wall_x2, wall_y, wall_step;

    // Apagar o viewport
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if( map == NULL )
        return;

    // Desenhar o horizonte (teto ao fundo e chao na metade inferior)
    glColor3ub( VIEW2D_COLOR_3UB_CEILING );
    glBegin( GL_QUADS );
	glVertex2f(  VIEW2D_LEFT, VIEW2D_BOTTOM );  // Canto inferior esquerdo
	glVertex2f(  VIEW2D_LEFT,    VIEW2D_TOP );  // Canto superior esquerdo
	glVertex2f( VIEW2D_RIGHT,    VIEW2D_TOP );  // Canto superior direito
	glVertex2f( VIEW2D_RIGHT, VIEW2D_BOTTOM );  // Canto inferior direito
    glEnd();
    // Desenhar o chao na metade inferior
    glColor3ub( VIEW2D_COLOR_3UB_FLOOR );
    glBegin( GL_QUADS );
	glVertex2f(  VIEW2D_LEFT, VIEW2D_BOTTOM );  // Canto inferior esquerdo
	glVertex2f(  VIEW2D_LEFT,             0 );  // Canto superior esquerdo
	glVertex2f( VIEW2D_RIGHT,             0 );  // Canto superior direito
	glVertex2f( VIEW2D_RIGHT, VIEW2D_BOTTOM );  // Canto inferior direito
    glEnd();

    // Desenhar a vista em frente (paredes laterais)
    wall_x1   = VIEW2D_LEFT;
    wall_x2   = VIEW2D_LEFT + VIEW2D_SIZE/20;
    wall_step = VIEW2D_SIZE / 8;
    for( n = 0;  n < VIEW2D_DEPTH;  n++ )
        {
        map->moveLeft( &x, &y, compass );
        wall_y = fabs( map->isWallOrDoor(x, y) ? wall_x1 : wall_x2 );
        drawWall( wall_x1, wall_y, wall_x2, fabs(wall_x2) );
        map->moveRight( &x, &y, compass );
        map->moveRight( &x, &y, compass );
        wall_y = fabs( map->isWallOrDoor(x, y) ? wall_x1 : wall_x2 );
        drawWall( -wall_x1, wall_y, -wall_x2, fabs(wall_x2) );
        map->moveLeft( &x, &y, compass );
        map->moveFwd ( &x, &y, compass );
        if( wall_x2 >= 0.0  ||  map->isWallOrDoor(x, y) )
            break;
        wall_x1 = wall_x2;
        wall_x2 += wall_step;
        if( wall_x2 > 0.0 )
            wall_x2 = 0.0;
        wall_step *= 0.725;
        if( wall_step < 0.25 )
            wall_step = 0.25;
        }

    if( n < VIEW2D_DEPTH )
        drawWall( wall_x2, wall_x2, -wall_x2, wall_x2 );
        // Desenhar parede em frente

    glFlush();
}


/* Desenha uma parede. x1,y1 e x2,y2 sao dois pontos superiores da parede.
   A parede e' desenhada com simetria horizontal.
   Atencao que "this->map" pode ser NULL!
*/
void View2D::drawWall( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 )
{
    glColor3ub( VIEW2D_COLOR_3UB_WALL );
    glBegin( GL_QUADS );
	glVertex2f( x1,  y1 );  // Canto superior esquerdo
	glVertex2f( x2,  y2 );  // Canto superior direito
	glVertex2f( x2, -y2 );  // Canto inferior direito
	glVertex2f( x1, -y1 );  // Canto inferior esquerdo
    glEnd();
    glColor3ub( VIEW2D_COLOR_3UB_WALL_OUTLINE );
    glBegin( GL_LINE_LOOP );
	glVertex2f( x1,  y1 );  // Canto superior esquerdo
	glVertex2f( x2,  y2 );  // Canto superior direito
	glVertex2f( x2, -y2 );  // Canto inferior direito
	glVertex2f( x1, -y1 );  // Canto inferior esquerdo
    glEnd();
}
