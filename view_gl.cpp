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


#include <queue>
#include <cassert>
#include <cmath>
#include <QGLWidget>
#include <QFrame>
#include <QTimer>
#include <QImage>
#include <QColor>
#include "view_gl.h"
#include "student_viewmap.h"
#include "student_view2d.h"
#include "student_view3d.h"
#include "map.h"
#include "cell.h"
#include "compass.h"


/*
=============================================================================
#### ViewMapGL ##############################################################
=============================================================================
*/


/* Construtor
*/
ViewMapGL::ViewMapGL( QFrame *&parent ) :
    QGLWidget( QGLFormat( QGL::SingleBuffer     |
                          QGL::NoDepthBuffer    |
                          QGL::Rgba             |
                          QGL::NoAlphaChannel   |
                          QGL::NoStereoBuffers  |
                          QGL::DirectRendering  |
                          QGL::NoOverlay ),
               parent )
{
    setAutoBufferSwap( false );
    // Vamos escrever progressivamente para o ecra por isso nao queremos
    // buffering (ve^tambem QGL::SingleBuffer, acima)

    delayed.wait_ms          = 0;
    delayed.cells_per_sec    = 1;
    map                      = NULL;
    viewmap                  = NULL;
    view_width = view_height = -1;  // "Ainda nao sei"

    // Incializa o QTimer que ira' desenhar as paredes 'a velocidade desejada
    delayed.paint_timer = new QTimer( this );
    delayed.paint_timer->setSingleShot( false );
    connect( delayed.paint_timer, SIGNAL(timeout()), this, SLOT(timerPaintGL()) );
}


/* Destrutor
   Atencao! Nao aceder a "map" aqui, porque esse objecto pode ja' ter sido
   destruido!
*/
ViewMapGL::~ViewMapGL()
{
    // Verificacao basica
    assert( delayed.paint_timer != NULL );

    if( delayed.paint_timer->isActive() )
        delayed.paint_timer->stop();
    disconnect( delayed.paint_timer, 0, 0, 0 );
    delete delayed.paint_timer;
    if( viewmap != NULL )
        {
        makeCurrent();
        delete viewmap;
        }
    /* Ver comentario acima. O sinais+slots sao desligados automaticamente,
       de qualquer forma:
    if( map != NULL )
        disconnect( map, 0, this, 0 );
    */
}


/* Metodo chamado quando um mapa (novo) e' criado.
   "seconds_to_completion" e' o numero de segundos que devem decorrer durante
   a construcao visual do labirinto, e para tal o metodo mapPaintGL() fara' uma
   animacao adequada (se animated==true) de acordo com o tamanho do labirinto
   para assegurar essa duracao aproximada.
*/
void ViewMapGL::setMap( Map *map, int seconds_to_completion )
{
    double ms;

    // Verificacoes basicas
    assert( map != NULL                  &&
            delayed.paint_timer != NULL  &&
            seconds_to_completion > 0 );

    if( delayed.paint_timer->isActive() )
        delayed.paint_timer->stop();
    delayed.paint_queue = std::queue<int>();

    if( this->map != NULL )
        disconnect( this->map, 0, this, 0 );
    this->map = map;
    connect( this->map, SIGNAL(clearedMap()),                 this, SLOT(mapClearGL())                  );
    connect( this->map, SIGNAL(wroteCell(int,int,Cell,bool)), this, SLOT(mapPaintGL(int,int,Cell,bool)) );

    // Calcula a pausa necessaria para writeCell()
    // Da area do labirinto, aproximadamente 60% serao paredes
    ms = (seconds_to_completion * 1000.0) / (this->map->getWidth()*this->map->getHeight() * 0.60);
    delayed.wait_ms = ceil( ms );
    delayed.cells_per_sec = 1;
    if( delayed.wait_ms < 20 )
        {
        // Nao atrapalhar o sistema com atrasos muito curtos...
        // (mais de 50fps (quadros por segundo) e' inutil)
        delayed.wait_ms = 20;
        delayed.cells_per_sec = ceil( 20.0 / ms );
        }
    else if( delayed.wait_ms > 50 )
        delayed.wait_ms = 50;  // ...nem muito longos!

    // Criar um novo objecto ViewMap relativo ao novo tamanho do labirinto
    makeCurrent();
    initializeGL();
}


/* Desenha o mapa tal como tinha sido colocado no ecra ate' agora.
*/
void ViewMapGL::paintGL()
{
    bool was_active;

    // Verificacoes basicas
    assert( viewmap != NULL  &&  delayed.paint_timer != NULL );

    was_active = delayed.paint_timer->isActive();
    if( was_active )
        delayed.paint_timer->stop();

    makeCurrent();  // Ver comentario no ficheiro .h
    viewmap->paint();

    if( was_active )
        delayed.paint_timer->start();
}


void ViewMapGL::initializeGL()
{
    if( viewmap != NULL )
        delete viewmap;

    if( map != NULL )
        viewmap = new ViewMap( map->getWidth(), map->getHeight() );
    else
        viewmap = new ViewMap( 1, 1 );

    if( view_width >= 0  &&  view_height >= 0 )
        {
        viewmap->resize( view_width, view_height );
        viewmap->clear();
        // Necessario se o objecto foi re-criado, pois nesse caso nao recebe
        // o evento resize() nem clear()
        }
}


void ViewMapGL::resizeGL( int view_width, int view_height )
{
    this->view_width  = view_width;
    this->view_height = view_height;

    assert( viewmap != NULL );  // Verificacao basica

    makeCurrent();
    viewmap->resize( view_width, view_height );
    if( map == NULL )
        viewmap->clear();
        // Necessario se este e' um objecto "dummy" para limpar o viewport
}


/* Slot ligada ao sinal do Map, emitido quando este e' inicialmente apagado
*/
void ViewMapGL::mapClearGL()
{
    assert( viewmap != NULL );  // Verificacao basica
    makeCurrent();
    viewmap->clear();
}


/* Slot ligada ao sinal do Map, emitido quando uma celula e' escrita
*/
void ViewMapGL::mapPaintGL( int x, int y, Cell c, bool animated )
{
    // Verificacao basica
    assert( delayed.paint_timer != NULL );

    if( animated  &&  delayed.wait_ms > 0 )
        {
        delayed.paint_queue.push( x );
        delayed.paint_queue.push( y );
        if( !delayed.paint_timer->isActive() )
            delayed.paint_timer->start( delayed.wait_ms );
        }
    else
        paintGL( x, y, c );
}


/* Chamada periodicamente para desenhar uma celula do mapa do labirinto.
*/
void ViewMapGL::timerPaintGL()
{
    int c, x, y;

    // Verificacoes basicas
    assert( map != NULL                  &&
            delayed.paint_timer != NULL  &&
            delayed.cells_per_sec > 0 );

    for( c = 0;  c < delayed.cells_per_sec;  c++ )
        {
        if( delayed.paint_queue.size() < 2 )
            break;
        x = delayed.paint_queue.front();
            delayed.paint_queue.pop();
        y = delayed.paint_queue.front();
            delayed.paint_queue.pop();
        assert( !map->badCoord(x,y) );  // Verificacao basica
        paintGL( x, y, map->getCell(x,y) );
        }
    // Nao usar size()<2 porque o processo principal pode estar a colocar
    // elementos na fila
    if( delayed.paint_queue.empty() )
        delayed.paint_timer->stop();
}


/*
=============================================================================
#### View2DGL ###############################################################
=============================================================================
*/


View2DGL::View2DGL( QFrame *&parent ) :
    QGLWidget( QGLFormat( QGL::DoubleBuffer     |
                          QGL::NoDepthBuffer    |
                          QGL::Rgba             |
                          QGL::NoAlphaChannel   |
                          QGL::NoStereoBuffers  |
                          QGL::DirectRendering  |
                          QGL::NoOverlay ),
               parent )
{
    map = NULL;
    view2d = NULL;
    view_width = view_height = -1;  // "Ainda nao sei"
}


/* Destrutor
   Atencao! Nao aceder a "map" aqui, porque esse objecto pode ja' ter sido
   destruido!
*/
View2DGL::~View2DGL()
{
    if( view2d != NULL )
        {
        makeCurrent();
        delete view2d;
        }
}


/* Metodo chamado quando um mapa (novo) e' criado.
*/
void View2DGL::setMap( Map *map )
{
    assert( map != NULL );  // Verificacao basica
    this->map = map;

    // Criar um novo objecto ViewMap relativo ao novo tamanho do labirinto
    makeCurrent();
    initializeGL();
}


void View2DGL::initializeGL()
{
    if( view2d != NULL )
        delete view2d;

    view2d = new View2D( map );

    if( view_width >= 0  &&  view_height >= 0 )
        view2d->resize( view_width, view_height );
        // Necessario se o objecto foi re-criado, pois nesse caso nao recebe
        // o evento resize()
}


/*
=============================================================================
#### View3DGL ###############################################################
=============================================================================
*/


View3DGL::View3DGL( QFrame *&parent ) :
    QGLWidget( QGLFormat( QGL::DoubleBuffer     |
                          QGL::DepthBuffer      |
                          QGL::Rgba             |
                          QGL::NoAlphaChannel   |
                          QGL::NoStereoBuffers  |
                          QGL::DirectRendering  |
                          QGL::NoOverlay ),
               parent )
{
    map = NULL;
    view3d = NULL;
    view_width = view_height = -1;  // "Ainda nao sei"

    // Incializa o QTimer que ira' fazer a transicao de vista
    transition.paint_timer = new QTimer( this );
    transition.paint_timer->setSingleShot( false );
    connect( transition.paint_timer, SIGNAL(timeout()), this, SLOT(timerPaintGL()) );
}


/* Destrutor
   Atencao! Nao aceder a "map" aqui, porque esse objecto pode ja' ter sido
   destruido!
*/
View3DGL::~View3DGL()
{
    // Verificacao basica
    assert( transition.paint_timer != NULL );

    if( transition.paint_timer->isActive() )
        transition.paint_timer->stop();
    disconnect( transition.paint_timer, 0, 0, 0 );
    delete transition.paint_timer;
    if( view3d != NULL )
        {
        makeCurrent();
        delete view3d;
        }
}


/* Metodo chamado quando um mapa (novo) e' criado.
*/
void View3DGL::setMap( Map *map )
{
    assert( map != NULL );  // Verificacao basica
    this->map = map;

    // Criar um novo objecto ViewMap relativo ao novo tamanho do labirinto
    makeCurrent();
    initializeGL();
}


void View3DGL::paintGL( float x, float y, float compass_direction, float seconds_to_completion )
{
    // Verificacao basica
    assert( transition.paint_timer != NULL );

    compass_direction = fmod( compass_direction , 4.0f );
    // Normalizar o angulo em numero positivo para que funcione bem
    // nas animacoes

    if( transition.paint_timer->isActive() )
        transition.paint_timer->stop();
        // Para que nao mude um "x" enquanto nos mudamos o "y"...

    transition.from_x = this->x;
    transition.from_y = this->y;
    transition.from_compass_direction = this->compass_direction;
    transition.complete = 0.0f;

    if( seconds_to_completion > 0.0f )
        {
        transition.to_x = x;
        transition.to_y = y;
        transition.to_compass_direction = compass_direction;
        transition.step = 1.0f / (seconds_to_completion * (float)VIEW3DGL_FPS);
        transition.paint_timer->start( 1000 / VIEW3DGL_FPS );
        }
    else
        {
        this->x = x;
        this->y = y;
        this->compass_direction = compass_direction;
        glDraw();
        }
}


void View3DGL::initializeGL()
{
    QImage img;
    QImage textures[VIEW3D_TEXTURES_NUMBER];

    if( view3d != NULL )
        delete view3d;



    // Prepara a imagem da textura do tecto
    if( !img.load(":/mainwindow/texture/chave.PNG") )
        {
        img = QImage( 16, 16, QImage::Format_RGB888 );
        img.fill( QColor(VIEW2D_COLOR_3UB_CHAVE).rgb() );
        }
         textures[VIEW3D_IX_TEXTURE_CHAVE] = QGLWidget::convertToGLFormat( img );



    // Prepara a imagem da textura das paredes
    if( !img.load(":/mainwindow/texture/chaopedra.PNG") )
        {
        img = QImage( 16, 16, QImage::Format_RGB888 );
        img.fill( QColor(VIEW2D_COLOR_3UB_CEILING).rgb() );
        }
    textures[VIEW3D_IX_TEXTURE_WALL] = QGLWidget::convertToGLFormat( img );



    // Prepara a imagem da textura do tecto
    if( !img.load(":/mainwindow/texture/peixes.PNG") )
        {
        img = QImage( 16, 16, QImage::Format_RGB888 );
        img.fill( QColor(VIEW2D_COLOR_3UB_BURACO).rgb() );
        }
         textures[VIEW3D_IX_TEXTURE_BURACO] = QGLWidget::convertToGLFormat( img );


    // Prepara a imagem da textura do tecto
    if( !img.load(":/mainwindow/texture/mainwindow.texture.ceiling.png") )
        {
        img = QImage( 16, 16, QImage::Format_RGB888 );
        img.fill( QColor(VIEW2D_COLOR_3UB_CEILING).rgb() );
        }
    textures[VIEW3D_IX_TEXTURE_CEILING] = QGLWidget::convertToGLFormat( img );

    // Prepara a imagem da textura das paredes
    if( !img.load(":/mainwindow/texture/tijolo.PNG") )
        {
        img = QImage( 16, 16, QImage::Format_RGB888 );
        img.fill( QColor(VIEW2D_COLOR_3UB_WALL).rgb() );
        }
    textures[VIEW3D_IX_TEXTURE_WALL] = QGLWidget::convertToGLFormat( img );

    // Prepara a imagem da textura de uma porta aberta
    if( !img.load(":/mainwindow/texture/mainwindow.texture.door-open.png") )
        {
        img = QImage( 16, 16, QImage::Format_RGB888 );
        img.fill( QColor(VIEW2D_COLOR_3UB_DOOR_OPEN).rgb() );
        }
    textures[VIEW3D_IX_TEXTURE_DOOR_OPEN] = QGLWidget::convertToGLFormat( img );

    // Prepara a imagem da textura de uma porta fechada
    if( !img.load(":/mainwindow/texture/mainwindow.texture.door-closed.png") )
        {
        img = QImage( 16, 16, QImage::Format_RGB888 );
        img.fill( QColor(VIEW2D_COLOR_3UB_DOOR_CLOSED).rgb() );
        }
    textures[VIEW3D_IX_TEXTURE_DOOR_CLOSED] = QGLWidget::convertToGLFormat( img );

    // Prepara a imagem da textura do chao
    if( !img.load(":/mainwindow/texture/chaopedra.PNG") )
        {
        img = QImage( 16, 16, QImage::Format_RGB888 );
        img.fill( QColor(VIEW3D_IX_TEXTURE_CHAO_PEDRA).rgb() );
        }
    textures[VIEW3D_IX_TEXTURE_CHAO_PEDRA] = QGLWidget::convertToGLFormat( img );

    view3d = new View3D( map, textures );

    if( view_width >= 0  &&  view_height >= 0 )
        view3d->resize( view_width, view_height );
        // Necessario se o objecto foi re-criado, pois nesse caso nao recebe
        // o evento resize()
}


/* Chamada periodicamente para desenhar um quadro (frame) da animacao de
   transicao de vista.
*/
void View3DGL::timerPaintGL()
{
    float dir;

    // Verificacao basica
    assert( transition.paint_timer != NULL );

    transition.complete += transition.step;
    if( (transition.complete + 0.01f) > 1.0f )
        {
        transition.paint_timer->stop();
        transition.complete = 1.0f;
        x = transition.to_x;
        y = transition.to_y;
        compass_direction = transition.to_compass_direction;
        }
    else
        {
        dir = transition.to_compass_direction - transition.from_compass_direction;
             if( dir >  2.0f )  dir = -( dir - 2.0f );
        else if( dir < -2.0f )  dir = -( dir + 2.0f );
            // Isto garante que ao estar virado para Sul (0.0f) e virar para a
            // esquerda (Este, 3.0f), ou vice-versa, a animacao e' correcta
        x = transition.from_x + ( (transition.to_x - transition.from_x) * transition.complete );
        y = transition.from_y + ( (transition.to_y - transition.from_y) * transition.complete );
        compass_direction = transition.from_compass_direction + ( dir * transition.complete );
        }

    updateGL();
}
