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

#include <cstdlib>
#include <ctime>
#include "TetrisGameProcess.h"
#include <QDebug>

using namespace std;


static const stone_shape shape_avail[]
  =  { {4,
	{{0,-1},{-1,0},{0,0},{0,1}},
	stone_shape::on_point
       },
       {4,
	{{-1,-1},{-1,0},{0,0},{0,1}},
	stone_shape::on_point
       },
       {4,
	{{1,-1},{0,0},{1,0},{0,1}},
	stone_shape::on_point
       },
       {4,
	{{0,-2},{0,-1},{-1,0},{0,0}},
	stone_shape::on_point
       },
       {4,
	{{0,-2},{0,-1},{0,0},{1,0}},
	stone_shape::on_point
       },
       {4,
	{{0,-2},{0,-1},{0,0},{0,1}},
	stone_shape::on_point
       },
       {4,
	{{0,0},{0,1},{1,0},{1,1}},
	stone_shape::on_dr_cross
       },
     };

const int no_shapes = sizeof(shape_avail) / sizeof(shape_avail[0]);


const QColor m_basiccols[] = {
    QColor(0xff,0,0),
    QColor(0,0xff,0),
    QColor(0,0,0xff),
    QColor(0,0xff,0xff),
    QColor(0xff,0,0xff),
    QColor(0xff,0xff,0),
    QColor(0xff,0xff,0xff)
};

const int m_nbasiccols = sizeof(m_basiccols) / sizeof(m_basiccols[0]);

const int                                      // Punktgutschriften fuer
    TetrisGameProcess::inc_stone = 50,        // 1 abgesetzten Tetris-Stein
    TetrisGameProcess::inc_drop1line = 5,     // "Abwerfen" (SPACE) pro Linie
    TetrisGameProcess::inc_line = 100;        // 1 volle Linie

TetrisGameProcess::TetrisGameProcess(BrickViewer* PlayField, BrickViewer* NextField) :
    m_bvPlayField (PlayField),
    m_bvNextField (NextField),
    m_score(0),
    m_lines(0),
    GameRunning (false),
    CL_BLACK(Qt::black),
    CL_WHITE(Qt::white),
    PFExtend (PlayField->GetCols(), PlayField->GetRows()),
    m_StoneColorRange(scrWide)
{
    srand( (unsigned)time( NULL ) );
}

TetrisGameProcess::~TetrisGameProcess()
{
}


bool operator< (const QColor& c1, const QColor& c2)
{
    if (c1.red() != c2.red())
        return c1.red() < c2.red();
    else
    {
        if (c1.green() != c2.green())
            return c1.green() < c2.green();
        else
        {
            return c1.blue() < c2.blue();
        }
    }
}

void TetrisGameProcess::DropDownCurrentStone (void)
{

    m_bvPlayField->PasteShape(current_stone.shape,
          current_stone.position.x(),
          current_stone.position.y(),
          CL_BLACK);

    int  depth=127;

    FOR_EACH_SHAPE_POINT(current_stone.shape,curr_point)
    {
        int posx=current_stone.position.x()+curr_point->x(),
            posy=current_stone.position.y()+curr_point->y();
        int sy;
        for (sy=posy;
             !PointInHeap (posx,sy) && (sy<PFExtend.height());
             sy++);
        if (sy-posy-1<depth)
            depth=sy-posy-1;
    }

    m_bvPlayField->PasteShape
        (current_stone.shape,
         current_stone.position.x(),
         current_stone.position.y() + depth,
         current_stone.colour);

    int line[30],
        no_lines=0;

    //TODO: curr_point wird neu deklariert (siehe Definition
    //von FOR_EACH_SHAPE_POINT). Einige Compiler moegen das vielleicht nicht...
    FOR_EACH_SHAPE_POINT(current_stone.shape,curr_point)
    {
        int posy=current_stone.position.y()+curr_point->y()+depth;

        bool found = false;
        int i;
        for (i=0; i<no_lines; i++)
            if (line[i] == posy)
                found = true;

        if (found)
            continue;

        bool FullLine = true;
        for (i=0; i<PFExtend.width(); i++)
        {
            QColor cl = m_bvPlayField->GetBrickColor(i,posy);
            if (cl == CL_BLACK)
            {
                FullLine = false;
            }
        }
        if (FullLine)
        {
            line[no_lines++]=posy;
        }
    }

#   ifdef SPEED_TEST
    line[0]=22; line[1]=21; line[2]=20; line[3]=19; no_lines=4;
#   endif

    if (no_lines) {
        QColor** cntnts = m_bvPlayField->GetContents();
        for (int i=0;i<no_lines;i++)
        {
            delete[] cntnts[line[i]];
            for (int l=line[i]; l > 0; l--)
                cntnts[l] = cntnts[l-1];
            cntnts[0] = new QColor[PFExtend.width()];
            for (int i2=0; i2<PFExtend.width(); i2++)
                cntnts[0][i2] = CL_BLACK;
            for (int n=i+1;n<no_lines;n++)
                if (line[n]<line[i]) line[n]++;
        }

    /*  if (EnableSounds)
        if (no_lines>0)
             sndPlaySound (wavLine,SND_ASYNC);
        else sndPlaySound (wavTouchDown,SND_ASYNC);
    */

        m_bvPlayField->SetContents (cntnts);
        m_bvPlayField->FreeRect(cntnts,m_bvPlayField->GetRows());
    }

    m_score += inc_stone+depth*inc_drop1line+no_lines*inc_line;
    m_lines += no_lines;

    current_stone.valid=false;
}

//TODO: Performance - Verbesserung und Vereinfachungen in DropDownCurrentStone:
//Man koennte die in allen Zeilen des Spielfelds enthaltenen Farben 
//in einem PFExtend.y - stelligen Array aus map<GdkColor,int> 's abspeichern. :)
//Dann braeuchte man noch in den stone_shapes ein Array, das fuer jede
//Zeile der Steinform die Anzahl der dort enthaltenen Bricks speichert.
//Wenn man nun in drop_down_current_stone einen Stein in den Haufen absetzt,
//koennte man leicht die o.g. maps der betroffenen Zeilen mit den Eintraegen
//aus dem Array der Steinform aktualisieren. So koennte man einerseits schnell
//volle Linien finden und andererseits auch effizient die Anzahl der ggf. freizugebenen
//Farben ( FreePlayfieldColor() ) ermitteln, ohne kompilzierte Konstrukte wie oben
//(viele Schleifen, "colors2bfreed"-Map) verwenden zu muessen.


bool TetrisGameProcess::FitsInPlayfield (/*const*/ tetr_stone& tstone)
{
    bool fits = true;
    FOR_EACH_SHAPE_POINT(tstone.shape,curr_point)
    {
        int pfx = tstone.position.x()+curr_point->x(),
            pfy = tstone.position.y()+curr_point->y();
        fits = (pfx>=0) && (pfx<PFExtend.width()) &&
            (pfy>=0) && (pfy<PFExtend.height()) &&
            !PointInHeap (pfx,pfy);
        if (!fits) break;
    }
    return (fits);
}


bool TetrisGameProcess::PointInHeap (int col, int row)
{
    if (current_stone.valid)
    {
        FOR_EACH_SHAPE_POINT (current_stone.shape, curr_point)
            if ((col == current_stone.position.x() + curr_point->x()) &&
                (row == current_stone.position.y() + curr_point->y()))
                return false;
    }

    return m_bvPlayField->GetBrickColor(col,row) != CL_BLACK;
}


void TetrisGameProcess::SetCurrentStone (const tetr_stone& new_one)
{
    if (current_stone.valid)
    {
        m_bvPlayField->PasteShape
            (current_stone.shape,
             current_stone.position.x(),
             current_stone.position.y(),
             CL_BLACK);
    }

    current_stone = new_one;
    current_stone.valid = true;

    m_bvPlayField->PasteShape
        (current_stone.shape,
         current_stone.position.x(),
         current_stone.position.y(),
         current_stone.colour);
}


bool TetrisGameProcess::StepForth ()
{
    if (GameRunning)
    {
        if (current_stone.valid)
        {
            tetr_stone new_stone=current_stone;
            new_stone.position.setY(new_stone.position.y() + 1);
            if (FitsInPlayfield (new_stone))
            {
                SetCurrentStone (new_stone);
                //if (EnableSounds) sndPlaySound (wavMove,SND_ASYNC);
            }
            else
                DropDownCurrentStone ();
        }
        else
        {

            if (next_shape)
            {
                current_stone.shape = *next_shape;
            }
            else
            {
                //TODO: kann da nicht manchmal shape_avail + no_shapes herauskommen?
                current_stone.shape = shape_avail[(rand()/20-1) * no_shapes / (RAND_MAX/20)];
            }
            next_shape = shape_avail + (rand()/20-1) * no_shapes / (RAND_MAX/20);

            m_bvNextField->FillAll (CL_BLACK);
            m_bvNextField->PasteShape (*next_shape,2,2,CL_WHITE);

            current_stone.position.setX(PFExtend.width()/2);
            int maxy;
            maxy = -128;
            FOR_EACH_SHAPE_POINT (current_stone.shape, curr_point)
                if (-curr_point->y() > maxy) maxy = - curr_point->y();
            current_stone.position.setY(maxy);

            QColor c;
            switch (m_StoneColorRange)
            {
            case scrWide:
                //TODO: Das ist nicht wirklich portabel, weil RAND_MAX auf
                //verschiedenen Plattformen unterschiedliche gross sein kann.
                c = QColor(rand()/(RAND_MAX/0x100+1),
                           rand()/(RAND_MAX/0x100+1),
                           rand()/(RAND_MAX/0x100+1));
                //Farbe zu dunkel?
                if (c.red()+c.green()+c.blue() < 200) {
                    switch (rand() / (RAND_MAX/3 + 1)) {
                    case 0:
                        c.setRed(0xff);
                        break;
                    case 1:
                        c.setGreen(0xff);
                        break;
                    default:
                        c.setBlue(0xff);
                        break;
                    }
                }
                break;
            default:
            case scrBasic:
                c = m_basiccols[rand()/20*m_nbasiccols/(RAND_MAX/20+1)];
                break;
            case scrBlackWhite:
                c = CL_WHITE;
                break;
            }

            current_stone.colour = c;

            if (FitsInPlayfield(current_stone))
            {
                SetCurrentStone (current_stone);
            }
            else
            {
                StopGame ();
                return false;
            }
        }
    }
    return true;
}


void TetrisGameProcess::ProcessKey(int Key)
{
    if (GameRunning && current_stone.valid)

    {
        tetr_stone new_stone=current_stone;
        int temp;
        switch (Key)
        {
        case Qt::Key_Left:
            new_stone.position.setX(new_stone.position.x() - 1);
            if (FitsInPlayfield (new_stone))
                SetCurrentStone (new_stone);
            break;

        case Qt::Key_Right:
            new_stone.position.setX(new_stone.position.x() + 1);
            if (FitsInPlayfield (new_stone))
                SetCurrentStone (new_stone);
            break;

        case Qt::Key_Down:
            FOR_EACH_SHAPE_POINT (new_stone.shape, curr_point)
            {
                temp = curr_point->x();
                curr_point->setX(-curr_point->y() +(new_stone.shape.rot_point==stone_shape::on_point ? 0 : 1));
                curr_point->setY(temp);
            }
            if (FitsInPlayfield (new_stone))
                SetCurrentStone (new_stone);
            break;

        case Qt::Key_Up:
            FOR_EACH_SHAPE_POINT (new_stone.shape, curr_point)
            {
                temp = curr_point->x();
                curr_point->setX(curr_point->y());
                curr_point->setY(-temp + (new_stone.shape.rot_point==stone_shape::on_point ? 0 : 1));
            }
            if (FitsInPlayfield (new_stone))
                SetCurrentStone (new_stone);
            break;

        case Qt::Key_Space:
            DropDownCurrentStone ();
            break;

        }
    }
}


void TetrisGameProcess::StopGame ()
{
    GameRunning = false;
}


void TetrisGameProcess::StartNewGame ()
{
    m_bvPlayField->FillAll(CL_BLACK);
    next_shape = NULL;
    current_stone.valid = false;
    m_score = m_lines = 0;
    GameRunning = true;
}

