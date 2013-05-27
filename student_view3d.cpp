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
    else if( pc->object == OBJ_WALL_LIGHT )
        {
        glDisable( GL_TEXTURE_2D );  // desliga texturas
        glColor3ub( VIEW3D_COLOR_3UB_WALL_LIGHT );
        }
    else
        {
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, id_textures[VIEW3D_IX_TEXTURE_WALL] );
        glColor3f( 1.0f, 1.0f, 1.0f );  // 1.0f = nao adulterar cores da textura
        }
    // porta
    if( pc->isDoor() )
{

}
    else
    {
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

    // Tecto
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

    // Chao
    if( pc->object == OBJ_FLOOR_PIT )
       {
        //glDisable( GL_TEXTURE_2D );
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, id_textures[VIEW3D_IX_TEXTURE_BURACO]);

        glBegin( GL_QUADS );
        glDisable(GL_TEXTURE_2D);
               glEnable(GL_BLEND);
                glColor4ub(VIEW3D_COLOR_3UB_BURACO, 90);

                glTexCoord2i( 0, 0 );  glVertex3f(  x,  y, 0.0f );  // Canto inf-esq
                glTexCoord2i( 1, 0 );  glVertex3f( x,  y1, 0.0f );  // Canto inf-dir
                glTexCoord2i( 1, 1 );  glVertex3f( x1, y1, 0.0f );  // Canto sup-dir
                glTexCoord2i( 0, 1 );  glVertex3f(  x1, y, 0.0f );  // Canto sup-esq
                glEnd();

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
        /*


        //DESENHAR QUADRADO DE CIMA//*/
    }


else
    {
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
