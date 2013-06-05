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
#include <GL/glu.h>
#include "student_view3d.h"
#include "map.h"
#include "compass.h"


/* Construtor
   Atencao que "map" pode ser NULL!
*/
View3D::View3D( Map *map, const QImage textures[VIEW3D_TEXTURES_NUMBER] )
{
    int n;

    this->map = map;

    // Inicializar as texturas do OpenGL
    glGenTextures( VIEW3D_TEXTURES_NUMBER, id_textures );
    for( n = 0;  n < VIEW3D_TEXTURES_NUMBER;  n++ )
    {
        glBindTexture    ( GL_TEXTURE_2D, id_textures[n] );
        glTexParameteri  ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri  ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
        gluBuild2DMipmaps( GL_TEXTURE_2D, 3, textures[n].width(), textures[n].height(),
                           GL_RGBA, GL_UNSIGNED_BYTE, textures[n].bits() );
    }

    /* Se definisse uma propriedade "GLuint id_list", poderia usar este codigo
       aqui para acelerar o processo de desenho do mapa em paint() com:
        glCallList( id_list );
       No entanto, com mapas bastante grandes, nem isto ajuda.

    id_list = glGenLists( 1 );
    glNewList( id_list, GL_COMPILE );
    drawMapBlock( 0, 0, map->getWidth()-1, map->getHeight()-1 );
    glEndList();
    */

    // Inicializar OpenGL
    glDisable( GL_TEXTURE_1D );
    glEnable ( GL_TEXTURE_2D );
    glShadeModel( GL_SMOOTH );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );  // Fundo preto
    glClearDepth( 1.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );


    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 95.0f, 1.0f, 0.1f, (GLfloat)VIEW3D_DEPTH );
    // A relacao de aspecto 1.0 vai ser substituida pelo valor correcto
    // quando o metodo resize() for chamado

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}


/* Metodo chamado quando a janela do jogo e' redimensionada, pelo que deve ser
   usado para (re-)configurar o OpenGL para uma nova dimensao da janela do
   jogo (ve^ glViewport()).
   Tambem e' chamado no arranque, logo apos o construtor.
   Atencao que "this->map" pode ser NULL!
*/
void View3D::resize( int view_width, int view_height )
{
    if( view_width  < 1 )  view_width  = 1;
    if( view_height < 1 )  view_height = 1;

    glViewport( 0, 0, view_width, view_height );

    // Aplicar a relacao de aspecto correcta
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 95.0f, (GLfloat)view_width / (GLfloat)view_height, 0.1f, (GLfloat)VIEW3D_DEPTH );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}


/* Desenha a vista com centro em x,y e orientacao compass_direction
   (que podem ser todos numeros reais para animacao de movimento).
   Atencao que "this->map" pode ser NULL!
*/
void View3D::paint( float x, float y, float compass_direction )
{
    int mx, my, m_compass_direction;
    GLfloat xc, yc, deg, rad;

    // Apagar o viewport
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if( map == NULL )
        return;

    // Aplicar a perspectiva correcta
    xc = x + 0.5f;
    yc = y + 0.5f;
    deg = 270.0f - ( compass_direction * 90.0f );
    rad = ( deg / 180.0f ) * M_PI;
    glLoadIdentity();
    gluLookAt( xc,            yc,            0.5f,    // Posicao do olho
               xc + cos(rad), yc + sin(rad), 0.5f,    // Centro de visualizacao
               0.0f,          0.0f,          1.0f );  // Vector "tecto"


    /* Esta e' a forma de "forca bruta" para desenhar a vista:
       desenho simplesmente todo o mapa e deixo a placa grafica decidir
       o que e' visivel e o que nao e'. Mas isto e' lento com mapas grandes...
    drawMapBlock( 0, 0, map->getWidth()-1, map->getHeight()-1 );
    */


    /* Esta e' uma forma razoavelmente rapida e inteligente de desenhar apenas
       a parte do mapa que e' visivel: desenho apenas o corredor em frente
       (e respectivas lateriais), assim como a vizinhanca da minha posicao.
       Desenho tambem sempre os corredores das duas direccoes laterais 'a
       direccao em que estou virado.
    */
    mx = trunc( x );
    my = trunc( y );
    m_compass_direction = (int) round(compass_direction);

    // Desenha o rectangulo da "vizinhanca" (isto e' a forma rapida e facil de
    // evitar que certas partes da vizinhanca sejam desenhadas 2x)
    drawMapBlock( mx-VIEW3D_VICINITY, my-VIEW3D_VICINITY, mx+VIEW3D_VICINITY, my+VIEW3D_VICINITY );

    if( m_compass_direction != Compass::SOUTH )
        drawMapBlock( mx-2, my+VIEW3D_VICINITY+1, mx+2, my+VIEW3D_DEPTH+1 );
    if( m_compass_direction != Compass::WEST )
        drawMapBlock( mx+VIEW3D_VICINITY+1, my-2, mx+VIEW3D_DEPTH+1, my+2 );
    if( m_compass_direction != Compass::NORTH )
        drawMapBlock( mx-2, my-VIEW3D_VICINITY-1, mx+2, my-VIEW3D_DEPTH+1 );
    if( m_compass_direction != Compass::EAST )
        drawMapBlock( mx-VIEW3D_VICINITY-1, my-2, mx-VIEW3D_DEPTH+1, my+2 );
}


/* Desenha um um bloco (rectangulo) do mapa.
   Atencao que "this->map" pode ser NULL!
*/
void View3D::drawMapBlock( int x1, int y1, int x2, int y2 )
{
    int x;

    if( map == NULL )
        return;

    // Garantir que o ponto x1,y1 e' do canto inferior esquerdo e o
    // x2,y2 e' do canto superior direito
    if( x2 < x1 )
        qSwap( x1, x2 );
    if( y2 < y1 )
        qSwap( y1, y2 );

    // Se o rectangulo esta' completamente fora do mapa, nao faz nada
    if( x1 >= map->getWidth()   ||  x2 < 0  ||
            y1 >= map->getHeight()  ||  y2 < 0 )
        return;

    // Limita o rectangulo 'a parte que esta' dentro do mapa
    x1 = qBound( 0, x1, map->getWidth() -1 );
    x2 = qBound( 0, x2, map->getWidth() -1 );
    y1 = qBound( 0, y1, map->getHeight()-1 );
    y2 = qBound( 0, y2, map->getHeight()-1 );

    // Desenha o rectangulo
    for( ;  y1 <= y2;  y1++ )
        for( x = x1;  x <= x2;  x++ )
            drawCell( (GLfloat) x, (GLfloat) y1, map->getCell(x, y1) );
}


/* Desenha uma celula do mapa.
   Atencao que "this->map" pode ser NULL!
*/
void View3D::drawCell( GLfloat x, GLfloat y, Cell c )
{
    if( c.isWallOrDoor() )
        drawWall( x, y, &c );
    else
        drawFloor( x, y, &c );
}


/* Desenha uma celula do mapa que e' uma parede.
   Atencao que "this->map" pode ser NULL!
*/
void View3D::drawWall( GLfloat x, GLfloat y, Cell *const pc )
{
    GLfloat x1 = x + 1.0f;
    GLfloat y1 = y + 1.0f;

    // Verificoes basicas
    assert( pc != NULL );

    if( pc->isDoor() )
    {
        glDisable( GL_TEXTURE_2D );  // desliga texturas
        glColor3ub( VIEW3D_COLOR_3UB_DOOR );
    }
    else
    {
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, id_textures[VIEW3D_IX_TEXTURE_WALL] );
        glColor3f( 1.0f, 1.0f, 1.0f );  // 1.0f = nao adulterar cores da textura
    }

                        //=================[PORTA]=================
    if( pc->isDoor() )
    {
        // Tecto DA PORTA*************************
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, id_textures[VIEW3D_IX_TEXTURE_CEILING] );
        glColor3f( 1.0f, 1.0f, 1.0f );  // 1.0f = nao adulterar cores da textura
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, -1 );  // Normal a apontar para baixo
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, 1.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1,  y, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x, y1, 1.0f );  // Canto sup-esq
        glEnd();


        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, id_textures[VIEW3D_IX_TEXTURE_CHAO_PEDRA] );
        glColor3f( 1.0f, 1.0f, 1.0f );  // 1.0f = nao adulterar cores da textura


                        //=================[DEGRAU DA PORTA]=================

        //chao azul*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.05f, 0.54f, 0.69f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1,  y, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, 0.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x, y1, 0.0f );  // Canto sup-esq
        glEnd();

        //lado de um degrau frente*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.45f, 0.64f, 0.69f );
        glTexCoord2i( 0, 0 );  glVertex3f(  x+0.1,  y+0.1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.1,  y+0.1, 0.1f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1-0.1, y+0.1, 0.1f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1-0.1, y+0.1, 0.0f );  // Canto sup-esq
        glEnd();


        //lado de um degrau tras*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.45f, 0.64f, 0.69f );
        glTexCoord2i( 0, 0 );  glVertex3f(  x+0.1,  y1-0.1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.1,  y1-0.1, 0.1f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1-0.1, y1-0.1, 0.1f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1-0.1, y1-0.1, 0.0f );  // Canto sup-esq
        glEnd();

        //lado de um degrau esquerdo*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.45f, 0.64f, 0.69f );
        glTexCoord2i( 0, 0 );  glVertex3f(  x1-0.1,  y+0.1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1-0.1,  y+0.1, 0.1f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1-0.1, y1-0.1, 0.1f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1-0.1, y1-0.1, 0.0f );  // Canto sup-esq
        glEnd();

        //lado de um degrau direito*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.45f, 0.64f, 0.69f );
        glTexCoord2i( 0, 0 );  glVertex3f(  x+0.1, y+0.1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.1, y+0.1, 0.1f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x+0.1, y1-0.1, 0.1f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x+0.1, y1-0.1, 0.0f );  // Canto sup-esq
        glEnd();

                        //=================[CHAO DA PORTA]=================

        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.4f, 0.0f, 0.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x+0.1,  y+0.1, 0.1f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.1,  y1-0.1, 0.1f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1-0.1, y1-0.1, 0.1f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1-0.1, y+0.1, 0.1f );  // Canto sup-esq
        glEnd();

                        //=================[TECTO DA PORTA]=================
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.4f, 0.0f, 0.0f );
        glTexCoord2i( 0, 0 );  glVertex3f(  x+0.01,  y+0.01, 0.807f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.01,  y1-0.01, 0.807f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1-0.01, y1-0.01, 0.807f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1-0.01, y+0.01, 0.807f );  // Canto sup-esq
        glEnd();

        //lado frente do tecto*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 1.0f, 0.0f, 0.0f );
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y-0.0001, 0.8f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x,  y-0.0001, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y-0.0001, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1, y-0.0001, 0.8f );  // Canto sup-esq
        glEnd();

       //lado tras do tecto*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 1.0f, 0.0f, 0.0f );
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y1+0.0001, 0.8f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x,  y1+0.0001, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1+0.0001, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1, y1+0.0001, 0.8f );  // Canto sup-esq
        glEnd();

        //lado esquerdo do tecto*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 1.0f, 0.0f, 0.0f );
        glTexCoord2i( 0, 0 );  glVertex3f(  x1,  y+0.1, 0.8f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1,  y+0.1, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1-0.1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1, y1-0.1, 0.8f );  // Canto sup-esq
        glEnd();

        //lado direito do tecto*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 1.0f, 0.0f, 0.0f );
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y+0.1, 0.8f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x,  y+0.1, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x, y1-0.1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x, y1-0.1, 0.8f );  // Canto sup-esq
        glEnd();


                  //=================[PILARES DA PORTA]=================


        //======[PILAR 1]======

        //parede 1.1*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x,  y, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x+0.1, y, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x+0.1, y, 0.0f );  // Canto sup-esq
        glEnd();

        //parede 1.2*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x+0.1,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.1,  y, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x+0.1, y+0.1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x+0.1, y+0.1, 0.0f );  // Canto sup-esq
        glEnd();

        //parede 1.3*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f( x,  y+0.1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x,  y+0.1, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x+0.1, y+0.1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f( x+0.1, y+0.1, 0.0f );  // Canto sup-esq
        glEnd();

        //parede 1.4*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f( x,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x,  y, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x, y+0.1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f( x, y+0.1, 0.0f );  // Canto sup-esq
        glEnd();

        //======[PILAR 2]======

        //parede 2.1*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x1-0.1,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1-0.1,  y, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1, y, 0.0f );  // Canto sup-esq
        glEnd();


        //parede 2.2*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x1,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1,  y, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y+0.1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1, y+0.1, 0.0f );  // Canto sup-esq
        glEnd();

        //parede 2.3*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x1-0.1,  y+0.1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1-0.1,  y+0.1, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y+0.1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1, y+0.1, 0.0f );  // Canto sup-esq
        glEnd();

        //parede 2.4*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x1-0.1,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1-0.1,  y, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1-0.1, y+0.1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1-0.1, y+0.1, 0.0f );  // Canto sup-esq
        glEnd();


        //======[PILAR 3]======

        //parede 3.1*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y1-0.1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x,  y1-0.1, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x+0.1, y1-0.1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x+0.1, y1-0.1, 0.0f );  // Canto sup-esq
        glEnd();

        //parede 3.2*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x+0.1,  y1-0.1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.1,  y1-0.1, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x+0.1, y1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x+0.1, y1, 0.0f );  // Canto sup-esq
        glEnd();

        //parede 3.3*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x,  y1, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x+0.1, y1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x+0.1, y1, 0.0f );  // Canto sup-esq
        glEnd();

        //parede 3.4*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f( x,  y1-0.1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x,  y1-0.1, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x, y1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f( x, y1, 0.0f );  // Canto sup-esq
        glEnd();

        //======[PILAR 4]======

        //parede 4.1*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x1-0.1,  y1-0.1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1-0.1,  y1-0.1, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1-0.1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1, y1-0.1, 0.0f );  // Canto sup-esq
        glEnd();

        //parede 4.2*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x1,  y1-0.1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1,  y1-0.1, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1, y1, 0.0f );  // Canto sup-esq
        glEnd();

        //parede 4.3*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x1-0.1,  y1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1-0.1,  y1, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1, y1, 0.0f );  // Canto sup-esq
        glEnd();

        //parede 4.4*************************
        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glColor3f( 0.0f, 0.0f, 1.0f );  //cor azul bebe
        glTexCoord2i( 0, 0 );  glVertex3f(  x1-0.1,  y1-0.1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1-0.1,  y1-0.1, 1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1-0.1, y1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1-0.1, y1, 0.0f );  // Canto sup-esq
        glEnd();
    }
    else if( pc->object == OBJ_WALL_LIGHT )
    {

        //============[CANDEIRO REDONDO]=============
                glDisable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

                //glEnable( GL_LIGHTING );
               // glLightf ( GL_LIGHT1, GL_AMBIENT, -1.0  );
                glPushMatrix();//pilha

                GLUquadricObj *quadric;
                quadric = gluNewQuadric();
                glTranslatef( x,y+0.5f,1.0f);//rotacao da esfera
                glColor4f(0.7f,0.7f,1.0f,0.8f);
                gluSphere( quadric , 0.2f , 10 , 10 );//esfera

                glPopMatrix();
                glDisable(GL_LIGHTING);
                glDisable(GL_BLEND);

             //===================KEY

                glDisable(GL_TEXTURE_2D); //azul
                //glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                glPushMatrix();//pilha
                GLUquadricObj *quadric4;
                quadric = gluNewQuadric();
                glTranslatef( x-0.6,y-0.3,0.5f);//rotacao da esfera
                glColor4f(0.4235f, 0.2117f, 0.5f,0.8f);
                gluSphere( quadric , 0.05f , 10 , 10 );//esfera


                glPopMatrix();
                //glDisable(GL_BLEND);
                glDisable(GL_TEXTURE_2D); //laranja
                glPushMatrix();//pilha
                GLUquadricObj *quadric3;
                quadric = gluNewQuadric();
                glTranslatef( x-0.65,y-0.3,0.5f);//rotacao da esfera
                glColor4f(0.4235f, 0.2117f, 0.5f,0.8f);
                gluSphere( quadric , 0.05f , 10 , 10 );//esfera
                glPopMatrix();

                glBegin( GL_QUADS );
                glColor3f( 0.4235f, 0.2117f, 0.5f );
                glNormal3i( 0, 0, 1 );  // Normal a apontar para a direita
                glTexCoord2i( 0, 0 );  glVertex3f( x-0.6 , y-0.3, 0.48f );  // Canto inf-esq
                glTexCoord2i( 1, 0 );  glVertex3f( x-0.6, y-0.3, 0.52f );  // Canto inf-dir
                glTexCoord2i( 1, 1 );  glVertex3f( x-0.4, y-0.3, 0.52f );  // Canto sup-dir
                glTexCoord2i( 0, 1 );  glVertex3f( x-0.4,  y-0.3, 0.48f );  // Canto sup-esq
//dentes da chave
                glNormal3i( 0, 0, 1 );  // Normal a apontar para a direita
                glTexCoord2i( 0, 0 );  glVertex3f( x-0.5 , y-0.3, 0.48f );  // Canto inf-esq
                glTexCoord2i( 1, 0 );  glVertex3f( x-0.48, y-0.3, 0.48f );  // Canto inf-dir
                glTexCoord2i( 1, 1 );  glVertex3f( x-0.48, y-0.3, 0.45f );  // Canto sup-dir
                glTexCoord2i( 0, 1 );  glVertex3f( x-0.5,  y-0.3, 0.45f );  // Canto sup-esq


                glNormal3i( 0, 0, 1 );  // Normal a apontar para a direita
                glTexCoord2i( 0, 0 );  glVertex3f( x-0.47 , y-0.3, 0.48f );  // Canto inf-esq
                glTexCoord2i( 1, 0 );  glVertex3f( x-0.46, y-0.3, 0.48f );  // Canto inf-dir
                glTexCoord2i( 1, 1 );  glVertex3f( x-0.46, y-0.3, 0.465f );  // Canto sup-dir
                glTexCoord2i( 0, 1 );  glVertex3f( x-0.47,  y-0.3, 0.465f );  // Canto sup-esq

                glNormal3i( 0, 0, 1 );  // Normal a apontar para a direita
                glTexCoord2i( 0, 0 );  glVertex3f( x-0.45 , y-0.3, 0.48f );  // Canto inf-esq
                glTexCoord2i( 1, 0 );  glVertex3f( x-0.44, y-0.3, 0.48f );  // Canto inf-dir
                glTexCoord2i( 1, 1 );  glVertex3f( x-0.44, y-0.3, 0.465f );  // Canto sup-dir
                glTexCoord2i( 0, 1 );  glVertex3f( x-0.45,  y-0.3, 0.465f );  // Canto sup-esq

                glNormal3i( 0, 0, 1 );  // Normal a apontar para a direita
                glTexCoord2i( 0, 0 );  glVertex3f( x-0.43 , y-0.3, 0.48f );  // Canto inf-esq
                glTexCoord2i( 1, 0 );  glVertex3f( x-0.41, y-0.3, 0.48f );  // Canto inf-dir
                glTexCoord2i( 1, 1 );  glVertex3f( x-0.41, y-0.3, 0.45f );  // Canto sup-dir
                glTexCoord2i( 0, 1 );  glVertex3f( x-0.43,  y-0.3, 0.45f );  // Canto sup-esq
                glEnd();





        //============[2 CANDEIRO REDONDO]=============
                glDisable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

                glPushMatrix();//pilha

                GLUquadricObj *quadric2;
                quadric2 = gluNewQuadric();
                glTranslatef( x+0.5,y,1.0f);//rotacao da esfera
                glColor4f(0.5019607843f,0.9490196078f,1.0f,0.8f);
                gluSphere( quadric , 0.2f , 10 , 10 );//esfera

                glPopMatrix();

                glDisable(GL_BLEND);


        //====================[DESENHA PARECE COM TEXTURA]=========================

        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, id_textures[VIEW3D_IX_TEXTURE_WALL] );
        glColor3f( 1.0f, 1.0f, 1.0f );  // 1.0f = nao adulterar cores da textura

        glBegin( GL_QUADS );

        // Face da frente (dentro do labirinto, a olhar para Norte)
        glNormal3i( 0, -1, 0 );  // Normal a apontar para o observador
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1,  y, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1,  y, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x,  y, 1.0f );  // Canto sup-esq

        // Face de tras (dentro do labirinto, a olhar para Norte)
        glNormal3i( 0, 1, 0 );  // Normal a apontar para longe do observador
        glTexCoord2i( 0, 0 );  glVertex3f( x1, y1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f(  x, y1, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f(  x, y1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f( x1, y1, 1.0f );  // Canto sup-esq

        // Face da direita (dentro do labirinto, a olhar para Norte)
        glNormal3i( 1, 0, 0 );  // Normal a apontar para a direita
        glTexCoord2i( 0, 0 );  glVertex3f( x1,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1, y1, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f( x1,  y, 1.0f );  // Canto sup-esq

        // Face da esquerda (dentro do labirinto, a olhar para Norte)
        glNormal3i( -1, 0, 0 );  // Normal a apontar para a esquerda
        glTexCoord2i( 0, 0 );  glVertex3f(  x, y1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f(  x,  y, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f(  x,  y, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x, y1, 1.0f );  // Canto sup-esq
        glEnd();

    }
    else
    {

            //===================[DESENHA PAREDE NORMAL]======================
        glBegin( GL_QUADS );

        // Face da frente (dentro do labirinto, a olhar para Norte)
        glNormal3i( 0, -1, 0 );  // Normal a apontar para o observador
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1,  y, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1,  y, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x,  y, 1.0f );  // Canto sup-esq

        // Face de tras (dentro do labirinto, a olhar para Norte)
        glNormal3i( 0, 1, 0 );  // Normal a apontar para longe do observador
        glTexCoord2i( 0, 0 );  glVertex3f( x1, y1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f(  x, y1, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f(  x, y1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f( x1, y1, 1.0f );  // Canto sup-esq

        // Face da direita (dentro do labirinto, a olhar para Norte)
        glNormal3i( 1, 0, 0 );  // Normal a apontar para a direita
        glTexCoord2i( 0, 0 );  glVertex3f( x1,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1, y1, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f( x1,  y, 1.0f );  // Canto sup-esq

        // Face da esquerda (dentro do labirinto, a olhar para Norte)
        glNormal3i( -1, 0, 0 );  // Normal a apontar para a esquerda
        glTexCoord2i( 0, 0 );  glVertex3f(  x, y1, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f(  x,  y, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f(  x,  y, 1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x, y1, 1.0f );  // Canto sup-esq
        glEnd();
    }
}


/* Desenha uma celula do mapa que e' chao.
   Atencao que "this->map" pode ser NULL!
*/
void View3D::drawFloor( GLfloat x, GLfloat y, Cell *const pc )
{
    GLfloat x1 = x + 1.0f;
    GLfloat y1 = y + 1.0f;

    // Verificoes basicas
    assert( pc != NULL );

                        //================[TECTO]================

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, id_textures[VIEW3D_IX_TEXTURE_CEILING] );
    glColor3f( 1.0f, 1.0f, 1.0f );  // 1.0f = nao adulterar cores da textura
    glBegin( GL_QUADS );
    glNormal3i( 0, 0, -1 );  // Normal a apontar para baixo
    glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, 1.0f );  // Canto inf-esq
    glTexCoord2i( 1, 0 );  glVertex3f( x1,  y, 1.0f );  // Canto inf-dir
    glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, 1.0f );  // Canto sup-dir
    glTexCoord2i( 0, 1 );  glVertex3f(  x, y1, 1.0f );  // Canto sup-esq
    glEnd();

                    //=======================[BURACO]======================
    if( pc->object == OBJ_FLOOR_PIT )
    {


        glDisable(GL_TEXTURE_2D);
        glBegin( GL_QUADS );
        // Face da frente (dentro do labirinto, a olhar para Norte)
        glNormal3i( 0, -1, 0 );  // Normal a apontar para o observador
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y1, -1.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x,  y1, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1,  y1, 0.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x1,  y1, -1.0f );  // Canto sup-esq

        // Face de tras (dentro do labirinto, a olhar para Norte)
        glNormal3i( 0, 1, 0 );  // Normal a apontar para longe do observador
        glTexCoord2i( 0, 0 );  glVertex3f( x, y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f(  x1, y, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f(  x1, y, -1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f( x, y, -1.0f );  // Canto sup-esq

        // Face da direita (dentro do labirinto, a olhar para Norte)
        glNormal3i( 1, 0, 0 );  // Normal a apontar para a direita
        glTexCoord2i( 0, 0 );  glVertex3f( x1,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1, y1, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, -1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f( x1,  y, -1.0f );  // Canto sup-esq

        // Face da esquerda (dentro do labirinto, a olhar para Norte)
        glNormal3i( -1, 0, 0 );  // Normal a apontar para a esquerda
        glTexCoord2i( 0, 0 );  glVertex3f(  x, y, -1.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f(  x,  y, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f(  x,  y1, 0.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x, y1, -1.0f );  // Canto sup-esq

        // Face de baixo (dentro do labirinto, a olhar para Norte)
        glNormal3i( 0, -1, 0 );  // Normal a apontar para o observador
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, -1.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f(  x,  y1, -1.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1,  y1, -1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f( x1,  y, -1.0f );  // Canto sup-esq */
        glEnd();

                    //================[RECORTES DO BURACO (ESCAVADO)]================

        //LADO NORTE***************************************
        glDisable( GL_TEXTURE_2D );
        glColor3f( 0.4235f, 0.2117f, 0.0f );  // 1.0f = nao adulterar cores da textura
        glBegin( GL_TRIANGLE_STRIP );
        glNormal3i( 0, 0, 1 );
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, 0.006f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.5,  y+0.2, 0.006f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y, 0.006f );  // Canto sup-dir
        glEnd();

        glBegin( GL_TRIANGLE_STRIP );
        glNormal3i( 0, 0, 1 );
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y1, 0.006f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.3,  y1-0.3, 0.006f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, 0.006f );  // Canto sup-dir
        glEnd();

        glBegin( GL_TRIANGLE_STRIP );
        glNormal3i( 0, 0, 1 );
        glTexCoord2i( 0, 0 );  glVertex3f(  x1,  y, 0.006f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.7,  y1-0.4, 0.006f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, 0.006f );  // Canto sup-dir
        glEnd();

        glBegin( GL_TRIANGLE_STRIP );
        glNormal3i( 0, 0, 1 );
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, 0.006f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x,  y1, 0.006f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x+0.2, y+0.5, 0.006f );  // Canto sup-dir
        glEnd();




        //prelongamento dos triangulos dos buracos **********************


        //  Lado nº1
        glDisable( GL_TEXTURE_2D );
        glColor3f( 0.023f, 0.430f, 1.0f );  // 1.0f = nao adulterar cores da textura
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.5,  y+0.2, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x+0.5,  y+0.2, -1.0f );  // Canto sup-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x,  y, -1.0f  );
        glEnd();


        glColor3f( 0.480f, 0.96f, 0.25f );  // 1.0f = nao adulterar cores da textura
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glTexCoord2i( 0, 0 );  glVertex3f(  x+0.5,  y+0.2, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1,  y, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1,  y, -1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 0 );  glVertex3f(  x+0.5,  y+0.2, -1.0f );
        glEnd();

        // Lado nº2

        glColor3f( 0.486f, 0.85f, 0.597f );
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y1, 0.0 );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.3,  y1-0.3, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x+0.3,  y1-0.3, -1.0f  );  // Canto sup-dir
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y1, -1.0 );
        glEnd();

        glColor3f( 1.0f, 0.0f, 0.0f );
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.3,  y1-0.3, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1,  y1, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 0 );  glVertex3f( x1,  y1, -1.0f );  // Canto sup-dir
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.3,  y1-0.3, -1.0f );
        glEnd();


        // Lado nº3
        glColor3f( 0.0f, 0.0f, 1.0f );
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glTexCoord2i( 0, 0 );  glVertex3f(  x1,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.7,  y1-0.4, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.7,  y1-0.4, -1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 0 );  glVertex3f(  x1,  y, -1.0f );
        glEnd();

        glColor3f( 0.032f, 0.744f, 0.554f );
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.7,  y1-0.4, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, 0.0f );  // Canto sup-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, -1.0f );
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.7,  y1-0.4, -1.0f );
        glEnd();


        //Lado nº4
        glColor3f( 0.484f, 0.184f, 0.843f );
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.2, y+0.5, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.2, y+0.5, -1.0f );  // Canto sup-dir
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, -1.0f );
        glEnd();

        glColor3f( 0.749f, 0.475f, 0.732f );
        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.2, y+0.5, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x,  y1, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x,  y1, -1.0f );  // Canto sup-dir
        glTexCoord2i( 1, 0 );  glVertex3f( x+0.2, y+0.5, -1.0f );
        glEnd();


        //================[VIDRO (BLEND)]================
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glBegin( GL_QUADS );
        glColor4f(0.5019607843f,0.9490196078f,1.0f,0.5f);
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, 0.005f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x,  y1, 0.005f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x+0.5, y1, 0.005f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x+0.5, y, 0.005f );  // Canto sup-esq
        glEnd();
        glDisable(GL_BLEND);

    }
    else if(pc->object == OBJ_FLOOR_KEY)
    {

        //===============[PIRAMIDE 4 TRIANGULOS E 1 QUADRADO NA BASE================

        glDisable(GL_TEXTURE_2D);
        glColor3ub(VIEW3D_COLOR_3UB_FLOOR_KEY);

        glBegin(GL_QUADS);
        glColor3f(0.33f,0.33f,0.88f);
        glNormal3i(0,-1,0);
        glTexCoord2i(0,0); glVertex3f(x+0.3, y+0.3, 0.4f);
        glTexCoord2i(1,0); glVertex3f(x1-0.3, y+0.3, 0.4f);
        glTexCoord2i(1,1); glVertex3f(x+0.70, y1-0.3, 0.4f);
        glTexCoord2i(0,1); glVertex3f(x+0.3, y+0.7, 0.4f);




        glBegin(GL_TRIANGLE_STRIP);
        glColor3f(0.08f,0.8f,0.8f);
        glNormal3i(1,0,0);
        glTexCoord2i(0,1); glVertex3f(x+0.3f, y+0.3f, 0.4f);
        glTexCoord2i(1,1); glVertex3f(x1-0.5f, y1-0.5f, 0.8f);
        glTexCoord2i(1,0); glVertex3f(x1-0.3f, y+0.3f, 0.4f);
        glEnd();


        glBegin(GL_TRIANGLE_STRIP);
        glColor3f(0.0f,1.0f,0.0f);
        glNormal3i(1,0,0);
        glTexCoord2i(0,1); glVertex3f(x1-0.3f, y+0.3f, 0.4f);
        glTexCoord2i(1,1); glVertex3f(x1-0.5f, y1-0.5f, 0.8f);
        glTexCoord2i(1,0); glVertex3f(x1-0.3f, y1-0.3f, 0.4f);
        glEnd();



        glBegin(GL_TRIANGLE_STRIP);
        glColor3f(0.5f,0.0f,0.5f);
        glNormal3i(1,0,0);
        glTexCoord2i(1,0); glVertex3f(x+0.3f, y1-0.3f, 0.4f);
        glTexCoord2i(1,1); glVertex3f(x1-0.5f, y1-0.5f, 0.8f);
        glTexCoord2i(1,0); glVertex3f(x1-0.3f, y1-0.3f, 0.4f);
        glEnd();


        glBegin(GL_TRIANGLE_STRIP);
        glColor3f(0.0f,0.0f,1.0f);
        glNormal3i(1,0,0);
        glTexCoord2i(1,0); glVertex3f(x+0.3f, y1-0.3f, 0.4f);
        glTexCoord2i(1,1); glVertex3f(x1-0.5f, y1-0.5f, 0.8f);
        glTexCoord2i(0,1); glVertex3f(x+0.3f, y+0.3f, 0.4f);
        glEnd();

        glBegin(GL_TRIANGLE_STRIP);
        glColor3f(0.08f,0.8f,0.8f);
        glNormal3i(1,0,0);
        glTexCoord2i(0,1); glVertex3f(x+0.3f, y+0.3f, 0.4f);
        glTexCoord2i(1,1); glVertex3f(x1-0.5f, y1-0.5f, 0.8f);
        glTexCoord2i(1,0); glVertex3f(x1-0.3f, y+0.3f, 0.4f);
        glEnd();


             //==================[CHAO DE PEDRA  NA PIRAMIDE]==================

        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, id_textures[VIEW3D_IX_TEXTURE_CHAO_PEDRA] );
        glColor3f( 1.0f, 1.0f, 1.0f );  // 1.0f = nao adulterar cores da textura

        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );  // Normal a apontar para cima
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1,  y, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, 0.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x, y1, 0.0f );  // Canto sup-esq
        glEnd();

    }
    else
    {
                        //================[CHAO DE PEDRA]================
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, id_textures[VIEW3D_IX_TEXTURE_CHAO_PEDRA] );
        glColor3f( 1.0f, 1.0f, 1.0f );  // 1.0f = nao adulterar cores da textura

        glBegin( GL_QUADS );
        glNormal3i( 0, 0, 1 );  // Normal a apontar para cima
        glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, 0.0f );  // Canto inf-esq
        glTexCoord2i( 1, 0 );  glVertex3f( x1,  y, 0.0f );  // Canto inf-dir
        glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, 0.0f );  // Canto sup-dir
        glTexCoord2i( 0, 1 );  glVertex3f(  x, y1, 0.0f );  // Canto sup-esq
        glEnd();
    }


}
