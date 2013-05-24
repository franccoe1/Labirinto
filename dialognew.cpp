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
#include "dialognew.h"
#include "ui_dialognew.h"
#include "mainwindow.h"


DialogNew::DialogNew( QWidget *parent, int width, int height, int complexity ) :
    QDialog( parent ),
    ui( new Ui::DialogNew )
{
    // Verificacoes basicas
    assert( ui != NULL  &&  width > 0  &&  height > 0  &&
            complexity > 0  &&  complexity <= 100 );

    ui->setupUi( this );
    ui->spinCorridorWidth ->setValue( width      );
    ui->spinCorridorHeight->setValue( height     );
    ui->sliderComplexity  ->setValue( complexity );
}


DialogNew::~DialogNew()
{
    delete ui;
}


int DialogNew::getCorridorWidth()
{
    // Verificacao basica
    assert( ui != NULL );
    return ui->spinCorridorWidth->value();
}


int DialogNew::getCorridorHeight()
{
    // Verificacao basica
    assert( ui != NULL );
    return ui->spinCorridorHeight->value();
}


int DialogNew::getComplexity()
{
    // Verificacao basica
    assert( ui != NULL );
    return ui->sliderComplexity->value();
}


void DialogNew::changeEvent( QEvent *e )
{
    QDialog::changeEvent( e );
    switch( e->type() )
        {
        case QEvent::LanguageChange:
            ui->retranslateUi( this );
            break;
        default:
            break;
        }
}
