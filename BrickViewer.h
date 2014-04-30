/*  GTris
 *  Copyright (C) Olaf Klischat
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef BRICK_VIEWER_H
#define BRICK_VIEWER_H

#include <QWidget>

struct stone_shape    /* Struktur für die Form eines Tetris-Steins */
  {
    int no_points;
    QPoint		     /* Posit. der einzelnen Punkte relativ */
       relpos [4]; 	     /* zum 'Bezugspunkt' */
    enum {on_point,on_dr_cross} rot_point; /* Drehpunkt auf dem Bezugspunkt */
  };                                       /* oder auf dem Kreuz rechts darunter */

/* Kopf einer Schleife, die ueber alle Punkte einer Form iteriert */
#define FOR_EACH_SHAPE_POINT(shape, point)           \
    for (QPoint* point = (shape).relpos;              \
             point - (shape).relpos < (shape).no_points;  \
         point++)

/*
 * BrickViewer: widget that displays a rectangular grid of
 * colored squares, called "bricks". The number of brick columns and
 * rows is set at initialization time and can'T be changed
 * afterwards. All bricks are squares, i.e. their with and
 * height are equal. All bricks in a BrickViewer have the same size
 * (edge length) a a given time. The edge length (and thus the size of
 * the whole BrickViewer) is set at initialization time and may be
 * changed any time afterwards.
 *
 * Each brick has a color, which may be changed at any time.
 */
class BrickViewer: QWidget {

    Q_OBJECT

    unsigned m_Cols, m_Rows, m_BrickSize;
    QColor** m_Contents;  //TODO: may use unique_ptr?

public:
    BrickViewer(QWidget* parent, unsigned cols, unsigned rows, unsigned BrickSize);

    unsigned GetRows();
    unsigned GetCols ();
    unsigned GetBrickSize ();
    void SetBrickSize (unsigned s);
    QSize sizeHint() const;

    QColor GetBrickColor (int col, int row);
    void SetBrickColor (QColor color, int col, int row);

    void FillRect (int col1, int row1, int col2, int row2, QColor color);
    void FillAll (QColor color);

    void PasteRect (QColor** rect, int width, int height, int colOrig, int rowOrig);

    void PasteShape (stone_shape& shape, int col, int row, QColor color);

    QColor** GetRect (int colOrig, int rowOrig, int width, int height);
    QColor** GetContents ();
    void SetContents (QColor** rect);

    ~BrickViewer();

protected:
    void paintEvent(QPaintEvent * event);

    void InvalidateBrickArea (int col1, int row1, int col2, int row2);
};


#endif
