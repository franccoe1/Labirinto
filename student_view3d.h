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


#ifndef VIEW3D_H
#define VIEW3D_H

#include <QtOpenGL>
#include <QImage>
#include "map.h"
#include "compass.h"

// Numero de celulas do mapa desenhadas no ecra
#define VIEW3D_DEPTH                   100

// Numero de celulas parte do rectangulo 'a volta que e' sempre desenhado
#define VIEW3D_VICINITY                5

#define VIEW3D_IX_TEXTURE_CEILING      0
#define VIEW3D_IX_TEXTURE_WALL         1
#define VIEW3D_IX_TEXTURE_DOOR_OPEN    2
#define VIEW3D_IX_TEXTURE_DOOR_CLOSED  3
//#define VIEW3D_IX_TEXTURE_FLOOR        4
#define VIEW3D_IX_TEXTURE_CHAO_PEDRA   4
#define VIEW3D_IX_TEXTURE_BURACO       5
#define VIEW3D_IX_TEXTURE_CHAVE        6
#define VIEW3D_TEXTURES_NUMBER         7

// Cores
#define VIEW3D_COLOR_3UB_DOOR        0x00, 0x00, 0x00
#define VIEW3D_COLOR_3UB_WALL_LIGHT  0xFF, 0xFF, 0xFF
#define VIEW3D_COLOR_3UB_FLOOR_PIT   0xFF, 0xFF, 0xFF
#define VIEW3D_COLOR_3UB_FLOOR_KEY   0x00, 0x00, 0x00
#define VIEW3D_COLOR_3UB_PLAYER1     0xFF, 0x33, 0x33






class View3D
{
private:
    Map *map;
    GLuint id_textures[ VIEW3D_TEXTURES_NUMBER ];
    void drawMapBlock( int x1, int y1, int x2, int y2 );
    void drawCell ( GLfloat x, GLfloat y, Cell c );
    void drawWall ( GLfloat x, GLfloat y, Cell *const pc );
    void drawFloor( GLfloat x, GLfloat y, Cell *const pc );

public:
    View3D( Map *map, const QImage textures[VIEW3D_TEXTURES_NUMBER] );
    // Atencao que "map" pode ser NULL!
    // Para as funcoes OpenGL glTexImage2D() ou gluBuild2DMipmaps(), acedemos
    // 'a largura, altura, formato (GLenum) e dados das imagens com:
    // textures[#].width(), textures[#].height(), GL_RGBA e textures[#].bits()

    void resize( int view_width, int view_height );
    void paint( float x, float y, float compass_direction );
};


#endif // VIEW3D_H
