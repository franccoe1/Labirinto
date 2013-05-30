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
#include <cstring>
#include <QtOpenGL>
#include <GL/glu.h>
#include "student_viewmap.h"
#include "cell.h"


/* Construtor
*/
ViewMap::ViewMap( int map_width, int map_height )
{
    // Verificacoes basicas
    assert( map_width >= 1  &&  map_height >= 1 );

    this->paint_flushes = true;
    this->map_width     = map_width;
    this->map_height    = map_height;
    this->map_painted   = new Cell[ map_width * map_height ];
    // Mantenho uma copia do mapa *desenhado* porque ele pode estar definido
    // no objecto de classe Map, mas ainda nao ter sido todo desenhado (devido
    // a uma animacao com QTimer)

    // Inicializar OpenGL
    glShadeModel( GL_SMOOTH );
    glClearColor( VIEWMAP_COLOR_3F_FLOOR, 1.0f );
    glDisable( GL_DEPTH_TEST );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( 0.0f, (GLfloat)this->map_width, 0.0f, (GLfloat)this->map_height );
    /*
    #ifdef QT_OPENGL_ES_1
    glOrthof( 0.0f, (GLfloat)this->map_width, 0.0f, (GLfloat)this->map_height, 0.0f, 1.0f );
    #else
    glOrtho(  0.0, (GLdouble)this->map_width, 0.0, (GLdouble)this->map_height, 0.0,  1.0  );
    #endif
    */
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}


/* Metodo chamado quando a janela do jogo e' redimensionada, pelo que deve ser
   usado para (re-)configurar o OpenGL para uma nova dimensao da janela do
   jogo (ve^ glViewport()).
   Tambem e' chamado no arranque, logo apos o construtor.
*/
void ViewMap::resize( int view_width, int view_height )
{
    if( view_width  < 1 )  view_width  = 1;
    if( view_height < 1 )  view_height = 1;

    glViewport( 0, 0, view_width, view_height );
}


/* (Re-)desenhar o mapa todo.
*/
void ViewMap::paint()
{
    int x, y;

    // Verificacao basica
    assert( map_painted != NULL );

    /* Util para verificar se o labirinto esta' a ser devidamente gerado
    char line[map_width*2+1];
    qDebug( "\nFull paint:" );
    line[map_width*2] = '\0';
    for( y = map_height-1;  y >= 0;  y-- )
        {
        for( x = 0;  x < map_width;  x++ )
            line[x*2+1] = line[x*2] = map_painted[convCoord(x,y)].isWallOrDoor() ? '$' : '_';
        qDebug( line );
        }
    qDebug( " " );
    */

    paint_flushes = false;
    for( y = 0;  y < map_height;  y++ )
        for( x = 0;  x < map_width;  x++ )
            this->paint( x, y, map_painted[convCoord(x,y)] );
    paint_flushes = true;
    glFlush();
}


/* Desenhar uma celula do mapa
*/
void ViewMap::paint( int x, int y, Cell c )
{
    GLfloat x0, x1, y0, y1;
    float angle;

    // Verificacoes basicas
    assert( x >= 0  &&  x < map_width   &&
            y >= 0  &&  y < map_height  &&
            map_painted != NULL );

    map_painted[ convCoord(x,y) ] = c;

    if( c.isDoor() )
        glColor3ub( VIEWMAP_COLOR_3UB_DOOR );
    else if( c.object == OBJ_WALL_LIGHT )
        glColor3ub( VIEWMAP_COLOR_3UB_WALL_LIGHT );
    else if( c.object == OBJ_FLOOR_PIT )
        glColor3ub( VIEWMAP_COLOR_3UB_FLOOR_PIT );
    else if( c.object == OBJ_FLOOR_KEY )
        glColor3ub( VIEWMAP_COLOR_3UB_FLOOR_KEY );
    else if( c.isWall() )
        glColor3ub( VIEWMAP_COLOR_3UB_WALL );
    else if( (c.players & 1) != 0 )
        glColor3ub( VIEWMAP_COLOR_3UB_PLAYER1 );
    else
        glColor3ub( VIEWMAP_COLOR_3UB_FLOOR );
    if( c.object == OBJ_FLOOR_PIT )
    {
        glBegin(GL_POLYGON);
        x0 = x; x1 = x +0.5f;
        y0 = y; y1 = y+ 0.5f;
        for (int i = 0; i< 100; i++)
        {
            angle=i*2*M_PI/100;             // vai fazer mini fatias de pizza de forma a fazer um circulo. quanto mais pequeno o andulo mais perfeito o circulo
            glVertex2f((x1) + (cos(angle)*0.5),(y1)+(sin(angle)*0.5));
        }
        glEnd();
    }
    else
    {
    glBegin( GL_QUADS );
    x0 = x;  x1 = x + 1.0f;
    y0 = y;  y1 = y + 1.0f;
    glVertex2f( x0, y0 );  // Canto inferior esquerdo
    glVertex2f( x0, y1 );  // Canto superior esquerdo
    glVertex2f( x1, y1 );  // Canto superior direito
    glVertex2f( x1, y0 );  // Canto inferior direito
    glEnd();
}
    if( paint_flushes )
        glFlush();
}


/* Limpar a area de desenho do mapa.
*/
void ViewMap::clear()
{
    static const Cell c = Cell( Cell::FLOOR );

    // Verificacao basica
    assert( map_painted != NULL );

    // Apagar o mapa desenhado
    map_painted[0] = c;
    memcpy( &map_painted[1], &map_painted[0], (map_height*map_width - 1) * sizeof(map_painted[0]) );

    /* As duas linhas acima sao uma versao mais rapida disto:
    int x, y;
    for( y = 0;  y < map_height;  y++ )
        for( x = 0;  x < map_width;  x++ )
            map_painted[ convCoord(x,y) ] = c;
    */

    // Apagar visualmente o mapa
    glLoadIdentity();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glFlush();
}
