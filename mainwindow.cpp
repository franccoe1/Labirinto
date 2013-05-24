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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialognew.h"
#include "map.h"
#include "compass.h"
#include "view_gl.h"
#include "student_mapcreate.h"


MainWindow::MainWindow( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::MainWindow )
{
    map = NULL;
    num_players = 0;
    current_player = 0;
    ui->setupUi( this );
}


MainWindow::~MainWindow()
{
    delete ui;

    /* Atencao: os objectos que pertencem a este objecto, so' sao destruidos
       depois, pelo que os seus destrutores nao devem mexer em "map".
       Para evitar erros, ignoramos este codigo e deixamos o sistema operativo
       apagar o mapa:
    if( map != NULL )
	delete map;
    */
}


void MainWindow::changeEvent( QEvent *e )
{
    QMainWindow::changeEvent( e );
    switch( e->type() )
        {
        case QEvent::LanguageChange:
            ui->retranslateUi( this );
            break;
        default:
            break;
        }
}


/* Este metodo e' chamado no arranque da aplicacao e quando o utilizador
   carrega em "OK" apos escolher o tamanho do labirinto para (re)comecar um
   jogo.
   Ele ajusta a janela (separadores, bussula, etc.) para o estado inicial.
*/
void MainWindow::resetStateNew()
{
    ui->tabSet->setCurrentIndex( MainWindow::TAB_MAP_INDEX );
    ui->tabSet->setTabEnabled( MainWindow::TAB_FP2D_INDEX, false );
    ui->tabSet->setTabEnabled( MainWindow::TAB_FP3D_INDEX, false );
    /* Nao e' preciso...
    ui->tabFP2D->setEnabled( false );
    ui->tabFP3D->setEnabled( false );
    */
}


/* Este metodo e' chamado quando o labirinto fica pronto, e o utilizador ja'
   pode comecar a jogar.
   Ele ajusta a janela (separadores, bussula, etc.) para o estado de jogo.
*/
void MainWindow::resetStatePlay()
{
    ui->tabSet->setTabEnabled( MainWindow::TAB_FP2D_INDEX, true );
    ui->tabSet->setTabEnabled( MainWindow::TAB_FP3D_INDEX, true );
    /* Nao e' preciso...
    ui->tabFP2D->setEnabled( true );
    ui->tabFP3D->setEnabled( true );
    */
}


/* Este metodo e' chamado quando o a aplicacao arranca, ou quando o jogador
   carrega em "Novo" para re-gerar novo labirinto.
*/
void MainWindow::requestNewMaze()
{
    int w, h;

    // Abrir a caixa de dialogo com os valores iniciais identicos aos do
    // labirinto agora aberto
    if( map == NULL )
        {
        w = 25;
        h = 15;
        }
    else
        {
        w = (map->getWidth()  - 1) / 2;
        h = (map->getHeight() - 1) / 2;
        }

    DialogNew d( this, w, h );
    if( d.exec() == DialogNew::Accepted )
        {
        resetStateNew();
        buildMaze( d.getCorridorWidth()*2 + 1, d.getCorridorHeight()*2 + 1, d.getComplexity() );
        }
}


/* Este metodo e' chamado quando o utilizador carrega em "OK" apos escolher o
   tamanho do labirinto, e apos ter sido chamado resetStateNew().
   Ele inicia o processo de desenhar um novo labirinto.

   Restricoes (verificadas no assert(), abaixo):
	width  >= 3 e impar
	height >= 3 e impar
	0 < complexity <= 100
*/
void MainWindow::buildMaze( int width, int height, int complexity )
{
    Map *map_old;
    int p, num;

    // Verificacoes basicas
    assert( ui != NULL                          &&
            width  >= 3  &&  (width  & 1) == 1  &&
            height >= 3  &&  (height & 1) == 1  &&
            complexity > 0  &&  complexity <= 100 );

    // Feedback visual de "a trabalhar"
    ui->pushNew->setEnabled( false );
    setCursor( QCursor(Qt::WaitCursor) );

    num_players = 0;

    // Criar o labirinto
    map_old = map;  // Para o apagar *depois* de associar o novo aos QGLWidgets
    map = new Map( width, height );
    ui->glViewMap->setMap( map );
    ui->glView2D ->setMap( map );
    ui->glView3D ->setMap( map );

    // Apagar o labirinto anterior (se existir)
    if( map_old != NULL )
        delete map_old;

    // Construir agora o labirinto
    MapCreate::walls   ( map, complexity );
    MapCreate::features( map );

    // Posicionar os jogadores
    // Nota: por enquanto so' usamos um jogador (o jogador 0). Futuramente,
    // se quisermos mais de um jogador, eles devem ter um posicionamento
    // aleatorio dentro do labirinto, ou obtido pela rede.
    num = 1;
    for( p = 0;  p < num;  p++ )
        {
        players[p].x = players[p].y = 1;
        players[p].compass = Compass( Compass::NORTH );
        // this->map->setPlayer() e' chamado por this->draw2d() e this->draw3d()
        }
    current_player = 0;
    num_players = num;
    draw2d();
    draw3d();

    // Feedback visual de "pronto a jogar"
    setCursor( QCursor(Qt::ArrowCursor) );
    ui->pushNew->setEnabled( true );
    resetStatePlay();
}


/* Este metodo e' chamado quando o jogador pede para virar 'a esquerda.
*/
void MainWindow::turnLeft()
{
    // Verificacoes basicas
    assert( current_player >= 0  &&  current_player < MAX_PLAYERS  &&
            current_player < num_players );

    Player *pp = &( players[current_player] );
    pp->compass.turnLeft();
    ui->dialCompass->setValue( pp->compass.getDirection() );
}


/* Este metodo e' chamado quando o jogador pede para virar 'a direita.
*/
void MainWindow::turnRight()
{
    // Verificacoes basicas
    assert( current_player >= 0  &&  current_player < MAX_PLAYERS  &&
            current_player < num_players );

    Player *pp = &( players[current_player] );
    pp->compass.turnRight();
    ui->dialCompass->setValue( pp->compass.getDirection() );
}


/* Este metodo e' chamado quando o jogador pede para avancar no labirinto.
*/
void MainWindow::moveFwd()
{
    int x, y;

    // Verificacoes basicas
    assert( map != NULL  &&
            current_player >= 0  &&  current_player < MAX_PLAYERS  &&
            current_player < num_players );

    Player *pp = &( players[current_player] );
    x = pp->x;
    y = pp->y;
    map->moveFwd( &x, &y, pp->compass );
    if( map->isFloorOrOpen(x, y) )
        {
        pp->x = x;
        pp->y = y;
        }
    // else fazer algum tipo de som ou efeito?
}


/* Este metodo e' chamado para refrescar a vista de dentro do labirinto em 2D.
*/
void MainWindow::draw2d()
{
    // Verificacoes basicas
    assert( map != NULL  &&
            current_player >= 0  &&  current_player < MAX_PLAYERS  &&
            current_player < num_players );

    Player *pp = &( players[current_player] );
    map->setPlayer( pp->x, pp->y, (1 << current_player) );
    ui->glView2D->paintGL( pp->x, pp->y, pp->compass );
}


/* Este metodo e' chamado para refrescar a vista de dentro do labirinto em 3D.
*/
void MainWindow::draw3d( bool animated )
{
    // Verificacoes basicas
    assert( map != NULL  &&
            current_player >= 0  &&  current_player < MAX_PLAYERS  &&
            current_player < num_players );

    Player *pp = &( players[current_player] );
    map->setPlayer( pp->x, pp->y, (1 << current_player) );
    ui->glView3D->paintGL( (double) pp->x, (double) pp->y,
                           (double)(int) pp->compass.getDirection(),
                           (animated ? 0.4f : 0.0f) );
}


void MainWindow::on_pushTurnLeft2D_clicked()
{
    turnLeft();
    draw2d();
}


void MainWindow::on_pushMoveFwd2D_clicked()
{
    moveFwd();
    draw2d();
}


void MainWindow::on_pushTurnRight2D_clicked()
{
    turnRight();
    draw2d();
}


void MainWindow::on_pushTurnLeft3D_clicked()
{
    turnLeft();
    draw3d( true );  // Animar esta accao
}


void MainWindow::on_pushMoveFwd3D_clicked()
{
    moveFwd();
    draw3d( true );  // Animar esta accao
}


void MainWindow::on_pushTurnRight3D_clicked()
{
    turnRight();
    draw3d( true );  // Animar esta accao
}


/* Metodo invocado quando trocamos de separador
*/
void MainWindow::on_tabSet_currentChanged( int index )
{
    Compass::Direction dir;

    switch( index )
        {
        case MainWindow::TAB_FP2D_INDEX:
            draw2d();
            assert( current_player >= 0  &&  current_player < MAX_PLAYERS  &&
                    current_player < num_players );  // Verificacoes basicas
            dir = players[current_player].compass.getDirection();
            break;
        case MainWindow::TAB_FP3D_INDEX:
            draw3d();
            assert( current_player >= 0  &&  current_player < MAX_PLAYERS  &&
                    current_player < num_players );  // Verificacoes basicas
            dir = players[current_player].compass.getDirection();
            break;
        default:  // MainWindow::TAB_MAP_INDEX e MainWindow::TAB_ABOUT_INDEX
            assert( num_players >= 0 );  // Verificacao basica
            dir = Compass::NORTH;
            break;
        }
    ui->dialCompass->setValue( dir );
}


void MainWindow::on_pushNew_clicked()
{
    requestNewMaze();
}
