#include <stdlib.h>
#include <time.h>
#include <gdk/gdkkeysyms.h>
#include "TetrisGameProcess.h"

#include <stdio.h>    //TODO: nur fuer Debugging


#define no_shapes 7
static struct stone_shape
 shape_avail [no_shapes]     /* verfuegbare Steinformen */
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


GdkColor CL_BLACK = RGB(0,0,0);

const int
    CTetrisGameProcess::inc_stone = 50,        // 1 abgesetzten Tetris-Stein
    CTetrisGameProcess::inc_drop1line = 5,     // "Abwerfen" (SPACE) pro Linie
    CTetrisGameProcess::inc_line = 100;        // 1 volle Linie


CTetrisGameProcess::CTetrisGameProcess(GtkBrickViewer* PlayField, GtkBrickViewer* NextField) :
    m_bvPlayField (PlayField), PFExtend (gtk_brick_viewer_GetCols(PlayField),gtk_brick_viewer_GetRows(PlayField)),
    m_bvNextField (NextField), GameRunning (false), GameEndNotify(NULL), ScoreChangeNotify(NULL),
    m_score(0), m_lines(0), m_StoneColorRange(scrWide)
{
    srand( (unsigned)time( NULL ) );
}

CTetrisGameProcess::~CTetrisGameProcess()
{
}


void CTetrisGameProcess::drop_down (void)                /* Flugstein "abwerfen" */
{

    gtk_brick_viewer_PasteShape
        ( m_bvPlayField,
          current_stone.shape,
          current_stone.position.x,
          current_stone.position.y,
          CL_BLACK);                         //current_stone l˜schen

    current_stone.valid=FALSE;                //und ungÅltig machen

    int  depth=127;

    GdkPoint* curr_point;
    for (curr_point=current_stone.shape.relpos; /* fuer alle Punkte des Flugsteins */
         curr_point-current_stone.shape.relpos<current_stone.shape.no_points;
         curr_point++)
    {
        int posx=current_stone.position.x+curr_point->x,
            posy=current_stone.position.y+curr_point->y;
        int sy;
        for (sy=posy;    /* y-Pos. des obersten darunter liegenden Haufen-Punktes ermitteln */
             !PointInHeap (posx,sy) && (sy<PFExtend.y);
             sy++);
        if (sy-posy-1<depth)
            depth=sy-posy-1;        /* kleinster aller FS-Punkt->Haufen-Punkt-AbstÑnde*/
                                    /* = "Falltiefe" depth des Flugsteins */
    }

    gtk_brick_viewer_PasteShape
        (m_bvPlayField,
         current_stone.shape,
         current_stone.position.x,
         current_stone.position.y + depth,
         current_stone.colour);                  //current_stone in den Haufen "einfÅgen"



    int line[30],
        no_lines=0;

    for (curr_point=current_stone.shape.relpos;  // alle Punkte des Flugsteins
         curr_point-current_stone.shape.relpos<current_stone.shape.no_points;
         curr_point++)
    {
        int posy=current_stone.position.y+curr_point->y+depth; // um depth nach unten setzen

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
        if (FullLine)  // Linie voll?
            line [no_lines++]=posy;             // ja -> y-Pos. der Linie merken
    }

    GdkColor** cntnts = gtk_brick_viewer_GetContents(m_bvPlayField);
    for (int i=0;i<no_lines;i++)   /* fÅr alle vollen Linien */
    {
        delete[] cntnts[line[i]];
        for (int l=line[i]; l > 0; l--)
            cntnts[l] = cntnts[l-1];
        cntnts[0] = new GdkColor[PFExtend.x];
        for (int i2=0; i2<PFExtend.x; i2++)
            cntnts[0][i2] = CL_BLACK;
        //        memmove (cntnts[1], cntnts,
        //           PFExtend.x*sizeof(COLORREF)*line[i]);  /* darÅberliegenden Teil des Haufens 1 nach unten schieben*/
        for (int n=i+1;n<no_lines;n++)         /* andere, darÅberliegende Linien-Positionen */
            if (line[n]<line[i]) line[n]++;  /* auch um 1 nach unten setzen */
    }

/*  if (EnableSounds)
    if (no_lines>0)
         sndPlaySound (wavLine,SND_ASYNC);
    else sndPlaySound (wavTouchDown,SND_ASYNC);
*/
//  RepaintHeap(); //Haufen neu zeichnen

    gtk_brick_viewer_SetContents (m_bvPlayField,cntnts);
    gtk_brick_viewer_FreeRect (cntnts,gtk_brick_viewer_GetRows(m_bvPlayField));

    m_score += inc_stone+depth*inc_drop1line+no_lines*inc_line;  /* Score erh˜hen */
    m_lines += no_lines;                                         /* Linienzehl erh˜hen */
}

//TODO: Zur Performance - Verbesserung in drop_down:
// - Tabelle verwalten, die fÅr jede Zeile die Anzahl der noch
//   zur vollen Linie fehlenden Steine gespeichert wird (die lÑòt
//   sich einfach bei jedem neuen Stein updaten => man kann schneller
//   die vollen Linien finden).
// - evtl. den Inhalt des Spielfeldes nicht bei jedem drop_down - Aufruf
//   neu mit GetContents() holen, sondern ihn zwischenspeichern


bool CTetrisGameProcess::does_fit_in_pfield (struct tetr_stone tstone)  /* Test, ob tstone ins SFeld paòt */
{
    bool  fits=TRUE; /* fits speichert Info, ob tstone paòt oder nicht */
    for (GdkPoint* curr_point=tstone.shape.relpos;     /* fÅr alle Punkte von tstone */
         (curr_point-tstone.shape.relpos<tstone.shape.no_points) && fits; /* Schleife lÑuft nur, solange */
         curr_point++)                                                        /* fits==TRUE */
    {
        int pfx = tstone.position.x+curr_point->x,  /* Posit. des Punkts auf dem SFeld */
            pfy = tstone.position.y+curr_point->y;  /* berechnen */
        fits = (pfx>=0) && (pfx<PFExtend.x) &&      /* Punkt passt, wenn: */
            (pfy>=0) && (pfy<PFExtend.y) &&         /*      - er innerhalb des SFeldes */
            !PointInHeap (pfx,pfy);                 /*      - und nicht im Haufen liegt */
    }
    return (fits);
}


bool CTetrisGameProcess::PointInHeap (int col, int row)
{
    if (current_stone.valid)
    {
        for (GdkPoint* curr_point=current_stone.shape.relpos;  /* alle Punkte des Flugsteins */
             curr_point-current_stone.shape.relpos < current_stone.shape.no_points;
             curr_point++)
        if ((col == current_stone.position.x + curr_point->x) &&
            (row == current_stone.position.y + curr_point->y))
            return false;
    }

    return gtk_brick_viewer_GetBrickColor (m_bvPlayField,col,row) != CL_BLACK;
}


void CTetrisGameProcess::fade_into (struct tetr_stone new_one) /* new_one zum Flugstein machen, */
{                                          /* BS entspr. aktualisieren */
    gtk_brick_viewer_PasteShape
        (m_bvPlayField,
         current_stone.shape,
         current_stone.position.x,
         current_stone.position.y,
         CL_BLACK);                         //current_stone loeschen

    current_stone=new_one;         /* current_stone aktualisieren */
    current_stone.valid = TRUE;
    RepaintCurrentStone();
}


void CTetrisGameProcess::RepaintCurrentStone()
{
    gtk_brick_viewer_PasteShape
        (m_bvPlayField,
         current_stone.shape,
         current_stone.position.x,
         current_stone.position.y,
         current_stone.colour);
}


void CTetrisGameProcess::StepForth ()
{
    if (GameRunning) /* Spiel lÑuft und ist nicht unterbrochen? */
    {
        if (current_stone.valid) /* Flugstein gÅltig? */
        { 
            tetr_stone new_stone=current_stone;  /* ja -> Kopie new_stone vom FS erzeugen, */
            new_stone.position.y++;   /* 1 runtersetzen */
            if (does_fit_in_pfield (new_stone)) /* paòt new_stone noch ins SFeld? */
            {
                fade_into (new_stone);         /* ja -> current_stone aktualisieren */
                //                      if (EnableSounds) sndPlaySound (wavMove,SND_ASYNC);
            }
            else
                drop_down ();   /* sonst: "abwerfen" (in diesem Fall um 0 Linien) */
        }
        else
        {                   /* Flugstein nicht gueltig -> */
            /* neuen Flugstein "auf die Reise schicken": */
            if (!next_shape) /* noch keine next-Form vorhanden (nur am Anfang des Spiels)? */
            {
                next_shape = shape_avail + (rand()/20-1) * no_shapes / (RAND_MAX/20); /* ja -> sowohl next-Form */  //TODO: kann nicht ganz stimmen...
                current_stone.shape = shape_avail[(rand()/20-1) * no_shapes / (RAND_MAX/20)];
                /* als auch Form des Flugsteins zufaellig erzeugen */
            }
            else
            {    /* next-Form schon vorhanden */
                current_stone.shape = *next_shape; /* -> Flugstein-Form = next-Form setzen */
                next_shape =  shape_avail + (rand()/20-1) * no_shapes / (RAND_MAX/20); /* und nur next-Form */
            }                                   /* zufaellig erzeugen */

            if (next_shape-shape_avail >= no_shapes)       //TODO: nur fuer Debugging
                printf ("wuaaaa...\n");


            //Next-Feld neu zeichnen
            gtk_brick_viewer_FillAll (m_bvNextField,CL_BLACK);
            gtk_brick_viewer_PasteShape (m_bvNextField,*next_shape,2,2,RGB(0xff,0xff,0xff));

            /* Form des neuen Flugsteins ist jetzt definiert */

            /* nun wird er ins Spielfeld gesetzt: */
            //current_stone.valid=TRUE;  /* Flugstein gueltig machen */
            current_stone.position.x=PFExtend.x/2; /* neue Spaltenposition=Spielfeldmitte */
            int maxy;
            GdkPoint* curr_point;
            for (curr_point=current_stone.shape.relpos,maxy=-128; /* oberste(n) Punkt(e) */
                 curr_point-current_stone.shape.relpos
                     <current_stone.shape.no_points;  /* des Flugsteins ermitteln */
                 curr_point++)
                if (-curr_point->y>maxy) maxy=-curr_point->y;
            current_stone.position.y=maxy;  /* neue Zeilenpos. so setzen, dass der */
            /* Flugstein den oberen SFeld-Rand beruehrt */
            /* aber nicht ueberschneidet */

            //Farbe des neuen Flugsteins bestimmen
            GdkColor c;
            GdkColor basiccols[] =
            {
                RGB(0xff,0,0),
                RGB(0,0xff,0),
                RGB(0,0,0xff),
                RGB(0,0xff,0xff),
                RGB(0xff,0,0xff),
                RGB(0xff,0xff,0),
                RGB(0xff,0xff,0xff),
            };
            switch (m_StoneColorRange)
            {
//             case scrWide:
//                 c = RGB(rand()*0x100/(RAND_MAX+1),
//                         rand()*0x100/(RAND_MAX+1),
//                         rand()*0x100/(RAND_MAX+1));
//                 if (c.red+c.green+c.blue < 300)
//                     *(((BYTE*)&c)+rand()*3/(RAND_MAX+1)) = 0xff;
//                 break;
//             case scrBasic:
//                 c = basiccols[rand()*sizeof(basiccols)/sizeof(COLORREF)/(RAND_MAX+1)];
//                 break;
            case scrBlackWhite:
                c = RGB(0xff,0xff,0xff);
                break;
            }

            current_stone.colour = c;

            if (does_fit_in_pfield(current_stone)) /* paòt der neue Flugstein noch ins SFeld? */
            {
                fade_into (current_stone);  /* ja -> gÅltig machen */
            }
            else
            {
                StopGame ();         /* sonst: Spielende */
                if (GameEndNotify != NULL)
                    (*GameEndNotify) ();
            }
        }
    }
}


void CTetrisGameProcess::ProcessKey(unsigned Key)
{
    if (GameRunning && current_stone.valid)
        /* Spiel muss laufen u. nicht unterbrochen sein; Flugstein muss gueltig sein */
    {
        tetr_stone new_stone=current_stone; /* dann: Flugstein-Kopie new_stone erzeugen */
        int temp;
        GdkPoint* curr_point;
        switch (Key)  /* gedrÅckte Taste ermitteln */
        {
        case GDK_Left:                     /* wenn crsr LEFT gedrueckt: */
            new_stone.position.x--;      /* new_stone 1 nach links */
            if (does_fit_in_pfield (new_stone))
                fade_into (new_stone);
            /* wenn er noch ins SFeld passt - Flugstein aktualisieren */
            /* sonst Taste ignorieren */
            break;

        case GDK_Right:                     /* analog fuer crsr RIGHT */
            new_stone.position.x++;
            if (does_fit_in_pfield (new_stone))
                fade_into (new_stone);
            break;

        case GDK_Down:                                 /* wenn crsr DOWN gedrueckt: */
            for (curr_point=new_stone.shape.relpos;  /* new_stone */
                 curr_point-new_stone.shape.relpos  /* um 90 Grad */
                     < new_stone.shape.no_points;
                 curr_point++)       /* im Uhrzeigersinn drehen */
            {
                temp=curr_point->x;
                curr_point->x=-curr_point->y
                    +(new_stone.shape.rot_point==stone_shape::on_point?0:1);
                curr_point->y=temp;
            }
            if (does_fit_in_pfield (new_stone))
                fade_into (new_stone);
            /* wenn er noch ins SFeld passt - Flugstein aktualisieren */
            /* sonst Taste ignorieren */
            break;

        case GDK_Up:                                   /* analog fuer crsr UP */
            for (curr_point=new_stone.shape.relpos;
                 curr_point-new_stone.shape.relpos
                     < new_stone.shape.no_points;
                 curr_point++)
            {
                temp=curr_point->x;
                curr_point->x=curr_point->y;
                curr_point->y=-temp+(new_stone.shape.rot_point==stone_shape::on_point?0:1);
            }
            if (does_fit_in_pfield (new_stone))
                fade_into (new_stone);
            break;

        case ' ':                     /* wenn SPACE gedrueckt: */
            drop_down ();                  /* Flugstein abwerfen */
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

