/*  $Id: TetrisGameProcess.cc,v 1.6.2.1 1999/08/29 18:28:32 olaf Exp $ */

/*  GTris
 *  $Name:  $
 *  Copyright (C) 1999  Olaf Klischat
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

#include <stdlib.h>
#include <time.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>
#include "TetrisGameProcess.h"


#define no_shapes 7
static stone_shape
 shape_avail [no_shapes]
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



const int                                      // Punktgutschriften fuer
    CTetrisGameProcess::inc_stone = 50,        // 1 abgesetzten Tetris-Stein
    CTetrisGameProcess::inc_drop1line = 5,     // "Abwerfen" (SPACE) pro Linie
    CTetrisGameProcess::inc_line = 100,        // 1 volle Linie
    CTetrisGameProcess::m_nbasiccols = 7;

CTetrisGameProcess::CTetrisGameProcess(GtkBrickViewer* PlayField, GtkBrickViewer* NextField) :
    m_bvPlayField (PlayField), m_bvNextField (NextField),
    PFExtend (gtk_brick_viewer_GetCols(PlayField), gtk_brick_viewer_GetRows(PlayField)),
    GameRunning (false), GameEndNotify(NULL), ScoreChangeNotify(NULL),
    m_score(0), m_lines(0), m_StoneColorRange(scrWide),
    CL_BLACK ( RGB(0,0,0) ), CL_WHITE( RGB(0xffff,0xffff,0xffff) )
{
    srand( (unsigned)time( NULL ) );

    m_basiccols = new GdkColor[m_nbasiccols];
    m_basiccols[0] = RGB(0xffff,0,0);
    m_basiccols[1] = RGB(0,0xffff,0);
    m_basiccols[2] = RGB(0,0,0xffff);
    m_basiccols[3] = RGB(0,0xffff,0xffff);
    m_basiccols[4] = RGB(0xffff,0,0xff);
    m_basiccols[5] = RGB(0xffff,0xffff,0);
    m_basiccols[6] = RGB(0xffff,0xffff,0xffff);

    if (GTK_WIDGET_REALIZED(GTK_WIDGET(m_bvPlayField)))
        on_playfield_realized (GTK_WIDGET(m_bvPlayField),this);
    else
        gtk_signal_connect (GTK_OBJECT (m_bvPlayField), "realize",
                            GTK_SIGNAL_FUNC (on_playfield_realized),
                            this);
}

void CTetrisGameProcess::on_playfield_realized
    (GtkWidget* playfield, CTetrisGameProcess* static_this)
{
    GdkColormap* cm = gdk_window_get_colormap(playfield->window);
    gdk_color_alloc (cm, &static_this->CL_BLACK);
    gdk_color_alloc (cm, &static_this->CL_WHITE);
    for (int i=0; i<static_this->m_nbasiccols; i++)
        gdk_color_alloc (cm, &static_this->m_basiccols[i]);

}

CTetrisGameProcess::~CTetrisGameProcess()
{
    delete[] m_basiccols;
}


void CTetrisGameProcess::DropDownCurrentStone (void)
{

    gtk_brick_viewer_PasteShape
        ( m_bvPlayField,
          current_stone.shape,
          current_stone.position.x,
          current_stone.position.y,
          CL_BLACK);

    current_stone.valid=FALSE;

    int  depth=127;

    FOR_EACH_SHAPE_POINT(current_stone.shape,curr_point)
    {
        int posx=current_stone.position.x+curr_point->x,
            posy=current_stone.position.y+curr_point->y;
        int sy;
        for (sy=posy;
             !PointInHeap (posx,sy) && (sy<PFExtend.y);
             sy++);
        if (sy-posy-1<depth)
            depth=sy-posy-1;
    }

    gtk_brick_viewer_PasteShape
        (m_bvPlayField,
         current_stone.shape,
         current_stone.position.x,
         current_stone.position.y + depth,
         current_stone.colour);

    int line[30],
        no_lines=0;

    //TODO: curr_point wird neu deklariert (siehe Definition
    //von FOR_EACH_SHAPE_POINT). Einige Compiler moegen das vielleicht nicht...
    FOR_EACH_SHAPE_POINT(current_stone.shape,curr_point)
    {
        int posy=current_stone.position.y+curr_point->y+depth;

        bool found = false;
        int i;
        for (i=0; i<no_lines; i++)
            if (line[i] == posy)
                found = true;

        if (found)
            continue;

        bool FullLine = true;
        for (i=0; i<PFExtend.x; i++)
            if (gtk_brick_viewer_GetBrickColor(m_bvPlayField,i,posy) == CL_BLACK)
            {
                FullLine = false;
                break;
            }
        if (FullLine)
            line [no_lines++]=posy;
    }

#   ifdef SPEED_TEST
    line[0]=22; line[1]=21; line[2]=20; line[3]=19; no_lines=4;
#   endif

    GdkColor** cntnts = gtk_brick_viewer_GetContents(m_bvPlayField);
    for (int i=0;i<no_lines;i++)
    {
        delete[] cntnts[line[i]];
        for (int l=line[i]; l > 0; l--)
            cntnts[l] = cntnts[l-1];
        cntnts[0] = new GdkColor[PFExtend.x];
        for (int i2=0; i2<PFExtend.x; i2++)
            cntnts[0][i2] = CL_BLACK;
        //        memmove (cntnts[1], cntnts,
        //           PFExtend.x*sizeof(COLORREF)*line[i]);
        for (int n=i+1;n<no_lines;n++)
            if (line[n]<line[i]) line[n]++;
    }

/*  if (EnableSounds)
    if (no_lines>0)
         sndPlaySound (wavLine,SND_ASYNC);
    else sndPlaySound (wavTouchDown,SND_ASYNC);
*/
//  RepaintHeap(); //Haufen neu zeichnen

    gtk_brick_viewer_SetContents (m_bvPlayField,cntnts);
    gtk_brick_viewer_FreeRect (cntnts,gtk_brick_viewer_GetRows(m_bvPlayField));

    m_score += inc_stone+depth*inc_drop1line+no_lines*inc_line;
    m_lines += no_lines;
}

//TODO: Zur Performance - Verbesserung in DropDownCurrentStone:
// - Tabelle verwalten, die fuer jede Zeile die Anzahl der noch
//   zur vollen Linie fehlenden Steine gespeichert wird (die laesst
//   sich einfach bei jedem neuen Stein updaten => man kann schneller
//   die vollen Linien finden).
// - evtl. den Inhalt des Spielfeldes nicht bei jedem DropDownCurrentStone - Aufruf
//   neu mit GetContents() holen, sondern ihn zwischenspeichern


bool CTetrisGameProcess::FitsInPlayfield (/*const*/ tetr_stone& tstone)
{
    bool  fits=TRUE;
    FOR_EACH_SHAPE_POINT(tstone.shape,curr_point)
    {
        int pfx = tstone.position.x+curr_point->x,
            pfy = tstone.position.y+curr_point->y;
        fits = (pfx>=0) && (pfx<PFExtend.x) &&
            (pfy>=0) && (pfy<PFExtend.y) &&
            !PointInHeap (pfx,pfy);
        if (!fits) break;
    }
    return (fits);
}


bool CTetrisGameProcess::PointInHeap (int col, int row)
{
    if (current_stone.valid)
    {
        FOR_EACH_SHAPE_POINT (current_stone.shape, curr_point)
            if ((col == current_stone.position.x + curr_point->x) &&
                (row == current_stone.position.y + curr_point->y))
                return false;
    }

    return gtk_brick_viewer_GetBrickColor (m_bvPlayField,col,row) != CL_BLACK;
}


void CTetrisGameProcess::SetCurrentStone (const tetr_stone& new_one)
{
    gtk_brick_viewer_PasteShape
        (m_bvPlayField,
         current_stone.shape,
         current_stone.position.x,
         current_stone.position.y,
         CL_BLACK);

    current_stone = new_one;
    current_stone.valid = TRUE;

    gtk_brick_viewer_PasteShape
        (m_bvPlayField,
         current_stone.shape,
         current_stone.position.x,
         current_stone.position.y,
         current_stone.colour);
}


bool CTetrisGameProcess::StepForth ()
{
    if (GameRunning)
    {
        if (current_stone.valid)
        {
            tetr_stone new_stone=current_stone;
            new_stone.position.y++;
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

            if (!next_shape)
            {
                //TODO: kann da nicht manchmal shape_avail + no_shapes herauskommen?
                next_shape = shape_avail + (rand()/20-1) * no_shapes / (RAND_MAX/20);
                current_stone.shape = shape_avail[(rand()/20-1) * no_shapes / (RAND_MAX/20)];
            }
            else
            {
                current_stone.shape = *next_shape;
                next_shape =  shape_avail + (rand()/20-1) * no_shapes / (RAND_MAX/20);
            }

            gtk_brick_viewer_FillAll (m_bvNextField,CL_BLACK);
            gtk_brick_viewer_PasteShape (m_bvNextField,*next_shape,2,2,CL_WHITE);

            current_stone.position.x=PFExtend.x/2;
            int maxy;
            maxy = -128;
            FOR_EACH_SHAPE_POINT (current_stone.shape, curr_point)
                if (-curr_point->y>maxy) maxy=-curr_point->y;
            current_stone.position.y=maxy;

            //TODO: StoneColorRange::scrWide (viele Steinfarben) noch nicht implementiert
            //(verhaelt sich bisher genauso wie scrBasic)
            //Problem: Jede Farbe muss vor ihrer Benutzung 'allokiert' werden (gdk_color_alloc)
            //bei scrBasic geht das, da nur begrenzte Anzahl (7) von Farben
            // ==> Allokierung in CTetrisGameProcess::on_playfield_realized
            //bei scrWide koennen aber praktisch unendlich viele Farben auftreten
            //Moegliche Loesung: Tabelle verwalten, die fuer jede benutze Farbe die aktuelle Anzahl der
            //Bricks mit dieser Farbe speichert. Geht der Wert fuer eine Farbe in dieser Tabelle auf 0,
            //kann die Farbe mit gdk_color_free wieder freigegeben werden.
            GdkColor c;
            switch (m_StoneColorRange)
            {
            case scrWide:
                /*
                c = RGB(0x100 * rand()*0x100/(RAND_MAX+1),
                        0x100 * rand()*0x100/(RAND_MAX+1),
                        0x100 * rand()*0x100/(RAND_MAX+1));
                //Farbe zu dunkel?
                if (((unsigned)c.red)+c.green+c.blue < 50000)
                    //TODO: fiese  Konstruktion, die einen der 3 RGB-Werte
                    //auf 0xffff setzt. Geht nur, wenn red der erste Wert in GdkColor ist
                    *((&c.red)+rand()*3/(RAND_MAX+1)) = 0xffff;
                break;
                */
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


void CTetrisGameProcess::ProcessKey(unsigned Key)
{
    if (GameRunning && current_stone.valid)

    {
        tetr_stone new_stone=current_stone;
        int temp;
        GdkPoint* curr_point;
        switch (Key)
        {
        case GDK_Left:
            new_stone.position.x--;
            if (FitsInPlayfield (new_stone))
                SetCurrentStone (new_stone);
            break;

        case GDK_Right:
            new_stone.position.x++;
            if (FitsInPlayfield (new_stone))
                SetCurrentStone (new_stone);
            break;

        case GDK_Down:
            FOR_EACH_SHAPE_POINT (new_stone.shape, curr_point)
            {
                temp=curr_point->x;
                curr_point->x=-curr_point->y
                    +(new_stone.shape.rot_point==stone_shape::on_point?0:1);
                curr_point->y=temp;
            }
            if (FitsInPlayfield (new_stone))
                SetCurrentStone (new_stone);
            break;

        case GDK_Up:
            FOR_EACH_SHAPE_POINT (new_stone.shape, curr_point)
            {
                temp=curr_point->x;
                curr_point->x=curr_point->y;
                curr_point->y=-temp+(new_stone.shape.rot_point==stone_shape::on_point?0:1);
            }
            if (FitsInPlayfield (new_stone))
                SetCurrentStone (new_stone);
            break;

        case ' ':
            DropDownCurrentStone ();
            break;

        }
    }
}


void CTetrisGameProcess::StopGame ()
{
    GameRunning = false;
}


void CTetrisGameProcess::StartNewGame ()
{
    gtk_brick_viewer_FillAll (m_bvPlayField,CL_BLACK);
    next_shape = NULL;
    current_stone.valid = false;
    m_score = m_lines = 0;
    GameRunning = true;
}

