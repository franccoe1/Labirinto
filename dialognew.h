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


#ifndef DIALOGNEW_H
#define DIALOGNEW_H

#include <QDialog>


namespace Ui
    {
    class DialogNew;
    }


class DialogNew : public QDialog
{
    Q_OBJECT
private:
    Ui::DialogNew *ui;

public:
    DialogNew( QWidget *parent = 0, int width = 25, int height = 15, int complexity = 40 );
    ~DialogNew();
    int getCorridorWidth();   // Nao pode ser inline por "ui" ser um tipo incompleto
    int getCorridorHeight();  // Nao pode ser inline por "ui" ser um tipo incompleto
    int getComplexity();      // Nao pode ser inline por "ui" ser um tipo incompleto

protected:
    void changeEvent(QEvent *e);
};


#endif // DIALOGNEW_H
