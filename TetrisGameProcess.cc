/*  $Id: TetrisGameProcess.cc,v 1.6.2.3 2000/01/08 15:08:28 olaf Exp $ */

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
//     GdkColormap* cm = gdk_window_get_colormap(playfield->window);
//     gdk_color_alloc (cm, &static_this->CL_BLACK);
//     gdk_color_alloc (cm, &static_this->CL_WHITE);
//     for (int i=0; i<static_this->m_nbasiccols; i++)
//         gdk_color_alloc (cm, &static_this->m_basiccols[i]);

}

CTetrisGameProcess::~CTetrisGameProcess()
{
    delete[] m_basiccols;
}


//TODO: Will die Map (m_allocatedColors) auch den operator== auf dem Elementtypen (GdkColor)
//haben? Wenn ja, gibt es hier ein Problem, da dieser (folgende) operator<
//das pixel-Feld von GdkColor nicht beachtet, der compilergenerierte operator==
//(element-by-element-compare) aber schon. ==> Es koennten Situationen auftreten, in denen
//trotz (a<b)==false und (b<a)==false (a==b)==false waere
//(naemlich dann, wenn die rgb-Werte gleich, die pixel-Werte aber unterschiedlich sind).
//pixel-Feld in compare_colors mit hineinnehmen waere nicht so gut, das dieser Wert von gdk_color_alloc
//ziemlich "willkürlich" (fuer den Benutzer) geaendert wird. Man muesste sicherstellen, dass diese
//geaenderten Pixelwerte nicht in m_allocatedColors landen. 
bool operator< (const GdkColor& c1, const GdkColor& c2)
{
    if (c1.red != c2.red)
        return c1.red < c2.red;
    else
    {
        if (c1.green != c2.green)
            return c1.green < c2.green;
        else
        {
            return c1.blue < c2.blue;
        }
    }
}

void CTetrisGameProcess::AllocateColor (const GdkColor& cl, int number)
{
    map<GdkColor,int>::iterator pCol = m_allocatedColors.find (cl);
    if (pCol == m_allocatedColors.end())
    {
        m_allocatedColors[cl] = number;
        GdkColor writeable_cl = cl;  //Kopie erzeugen wg. const und damit (siehe obiges TODO)
                                     //der Pixelwert der uebergenenen Variable cl nicht geaendert wird
        gdk_color_alloc (gdk_window_get_colormap(GTK_WIDGET(m_bvPlayField)->window),
                         &writeable_cl);
        //TODO: writeable_cl ist --wie eigentlich fast alle Farbwerte hier-- nur temporaer
        //(wird beim Verlassen dieses Blocks geloescht)
        //ist das ein Problem fuer gdk_color_alloc?
        //wenn ja, muesste man gdk_color_alloc vielleicht doch direkt auf den Keys von m_allocatedColors
        //arbeiten lassen. Dann aber wohl auch noch operator== fuer GdkColor redefinieren, sodass
        //der Pixel-Wert dort nicht beachtet wird (damit die map nicht
        //mit ihren Keys durcheinander kommt)
    }
    else
        pCol->second += number;
}


void CTetrisGameProcess::FreeColor (const GdkColor& cl, int number)
{
    map<GdkColor,int>::iterator pCol = m_allocatedColors.find (cl);
    pCol->second -= number;
    if (pCol->second == 0)
    {
        GdkColor writeable_cl = cl;
        gdk_colormap_free_colors (gdk_window_get_colormap(GTK_WIDGET(m_bvPlayField)->window),
                                  &writeable_cl,1);
        m_allocatedColors.erase (cl);
    }
}


void CTetrisGameProcess::DropDownCurrentStone (void)
{

    gtk_brick_viewer_PasteShape
        ( m_bvPlayField,
          current_stone.shape,
          current_stone.position.x,
          current_stone.position.y,
          CL_BLACK);

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

    map<GdkColor,int> colors2bfreed;
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
        map<GdkColor,int> line_colors;
        for (i=0; i<PFExtend.x; i++)
        {
            GdkColor cl = gtk_brick_viewer_GetBrickColor(m_bvPlayField,i,posy);
            if (cl == CL_BLACK)
            {
                FullLine = false;
                break;
            }
            line_colors[cl]++;
        }
        if (FullLine)
        {
            line [no_lines++]=posy;
            for (map<GdkColor,int>::iterator it = line_colors.begin();
                 it != line_colors.end(); it++)
            {
                colors2bfreed[it->first] += it->second;
            }
        }
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
        for (int n=i+1;n<no_lines;n++)
            if (line[n]<line[i]) line[n]++;
    }

/*  if (EnableSounds)
    if (no_lines>0)
         sndPlaySound (wavLine,SND_ASYNC);
    else sndPlaySound (wavTouchDown,SND_ASYNC);
*/

    gtk_brick_viewer_SetContents (m_bvPlayField,cntnts);
    gtk_brick_viewer_FreeRect (cntnts,gtk_brick_viewer_GetRows(m_bvPlayField));

    for (map<GdkColor,int>::iterator it = colors2bfreed.begin();
         it != colors2bfreed.end(); it++)
    {
        FreeColor (it->first,it->second);
    }

    m_score += inc_stone+depth*inc_drop1line+no_lines*inc_line;
    m_lines += no_lines;

    current_stone.valid=FALSE;
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
    bool no_allocs = false;
    if (current_stone.valid &&
        (current_stone.colour == new_one.colour) &&
        (current_stone.shape.no_points == new_one.shape.no_points))
        no_allocs = true;

    if (current_stone.valid)
    {
        gtk_brick_viewer_PasteShape
            (m_bvPlayField,
             current_stone.shape,
             current_stone.position.x,
             current_stone.position.y,
             CL_BLACK);
        if (!no_allocs)
            FreeColor (current_stone.colour, current_stone.shape.no_points);
    }

    current_stone = new_one;
    current_stone.valid = TRUE;

    if (!no_allocs)
        AllocateColor (current_stone.colour, current_stone.shape.no_points);
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

