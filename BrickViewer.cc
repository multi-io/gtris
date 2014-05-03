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

#include "BrickViewer.h"

#include <QPainter>
#include <QPaintEvent>
#include <qnamespace.h>

#include <cstdio>
#include <cassert>

static QColor** AllocateRect (int width, int height);


BrickViewer::BrickViewer(QWidget* parent, unsigned cols, unsigned rows, unsigned BrickSize) :
        QWidget(parent),
        m_Cols(cols),
        m_Rows(rows),
        m_BrickSize(BrickSize) {

    m_Contents = new QColor* [m_Cols];
    QColor clBlack(0,0,0);
    for (unsigned col = 0; col < m_Cols; col++)
    {
        m_Contents[col] = new QColor[m_Rows];
        for (unsigned row = 0; row < m_Rows; row++)
        {
            m_Contents[col][row] = clBlack;
        }
    }
    //setSizePolicy(...);  //TODO
}

BrickViewer::BrickViewer(unsigned cols, unsigned rows, unsigned BrickSize) :
    BrickViewer(0, cols, rows, BrickSize) {
}



BrickViewer::~BrickViewer() {
    for (unsigned col = 0; col < m_Cols; col++)
    {
        delete[] m_Contents[col];
    }
    delete[] m_Contents;
}


QSize BrickViewer::sizeHint() const {
    return QSize(m_BrickSize * m_Cols, m_BrickSize * m_Rows);
}

unsigned BrickViewer::GetRows ()
{ return m_Rows; }

unsigned BrickViewer::GetCols ()
{ return m_Cols; }

unsigned BrickViewer::GetBrickSize ()
{ return m_BrickSize; }

QColor BrickViewer::GetBrickColor (int col, int row)
{ return m_Contents[col][row]; }


//TODO: asserts einbauen



static QColor** AllocateRect (int width, int height)
{
    QColor** result = new QColor* [height];

    for (int row = 0; row < height; row++)
        result[row] = new QColor [width];

    return result;
}


void BrickViewer::FreeRect (QColor** rect, int height)
{
    for (int row = 0; row < height; row++)
        delete[] rect[row];
    delete[] rect;
}


void BrickViewer::SetBrickSize (unsigned s)
{
    if (s==0) return;
    m_BrickSize = s;
    updateGeometry();
}


void BrickViewer::InvalidateBrickArea (int col1, int row1, int col2, int row2)
{
    update(m_BrickSize * col1, m_BrickSize * row1, m_BrickSize * (col2-col1+1), m_BrickSize * (row2-row1+1));
}


void BrickViewer::SetBrickColor (QColor color, int col, int row)
{
    m_Contents[col][row] = color;
    InvalidateBrickArea(col,row,col,row);
}

void BrickViewer::FillRect (int col1, int row1, int col2, int row2, QColor color)
{
    for (int col = col1; col <= col2; col++)
        for (int row = row1; row <= row2; row++)
        {
            m_Contents[col][row] = color;
        }
    
    InvalidateBrickArea (col1,row1,col2,row2);
}

void BrickViewer::FillAll (QColor color)
{
    FillRect (0, 0, m_Cols - 1, m_Rows - 1, color);
}

void BrickViewer::PasteShape (const stone_shape& shape, int col, int row, const QColor& color)
{
    int minCol=100000, minRow=100000,
        maxCol=-100000, maxRow=-100000;
    for (const QPoint* p = shape.relpos;
         p - shape.relpos < shape.no_points;
         p++)
    {
        int x = col + p->x(), y = row + p->y();
        if (x < minCol) minCol = x;
        if (x > maxCol) maxCol = x;
        if (y < minRow) minRow = y;
        if (y > maxRow) maxRow = y;
        m_Contents[x][y] = color;
    }

    InvalidateBrickArea (minCol,minRow,maxCol,maxRow);
}


void BrickViewer::PasteRect (QColor** rect, int width, int height,
                                 int colOrig, int rowOrig)
{
    for (int row = 0; row < height; row++)
        for (int col = 0; col < width; col++)
        {
            m_Contents[colOrig + col][rowOrig + row] = rect[row][col];
        }
    update();
}


//TODO: return unique_ptr?
QColor** BrickViewer::GetRect (int colOrig, int rowOrig, int width, int height)
{
    QColor** result = AllocateRect (width,height);

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
            result[row][col] = m_Contents[colOrig+col][rowOrig+row];
    }

    return result;
}


QColor** BrickViewer::GetContents ()
{
    return GetRect (0,0,m_Cols,m_Rows);
}


void BrickViewer::SetContents (QColor** rect)
{
    PasteRect (rect,m_Cols,m_Rows,0,0);
}


void BrickViewer::paintEvent(QPaintEvent* event) {
    QRect rect = event->rect();

    unsigned minCol = rect.left() / m_BrickSize, minRow = rect.top() / m_BrickSize,
             maxCol = rect.right() / m_BrickSize, maxRow = rect.bottom() / m_BrickSize;

    if (maxCol >= m_Cols) maxCol = m_Cols-1;
    if (maxRow >= m_Rows) maxRow = m_Rows-1;

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    for (unsigned col = minCol; col <= maxCol; col++) {
        for (unsigned row = minRow; row <= maxRow; row++) {
            painter.setBrush(m_Contents[col][row]);
            painter.drawRect(col * m_BrickSize, row * m_BrickSize,
                             m_BrickSize, m_BrickSize);
        }
    }
}
