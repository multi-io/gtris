/*  $Id: TetrisGameProcess.h,v 1.2.4.2 2000/01/08 15:08:29 olaf Exp $ */

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

#ifndef TETRISGAMEPROCESS_H
#define TETRISGAMEPROCESS_H

#include "gtkbrickviewer.h"
#include "utils.h"
#include <map>


struct tetr_stone     /* ein Tetris-Stein */
{
    CPoint position;           /* position relativ zur oberen, linken Ecke des Spielfeldes */
    struct stone_shape shape;  /* Form des Steins */
    GdkColor colour;           /* Farbe des Steins */
    bool valid;                /* Info, ob Stein guetig ist */
};


enum TGameStatus {gsStopped,gsRunning,gsPaused};

//Das ist eigentlich nicht so toll, da der globale Namespace polluted wird, nur um
//eine privates Objekt (CTetrisGameProcess::m_allocatedColors), das diesen operator< benoetigt,
//zum Funktionieren zu bringen
bool operator< (const GdkColor& c1, const GdkColor& c2);

class CTetrisGameProcess
{

public:
    typedef void (*NotifyFunc) ();
    typedef enum {scrWide, scrBasic, scrBlackWhite} StoneColorRange;

private:
    GtkBrickViewer* m_bvPlayField;
    GtkBrickViewer* m_bvNextField;

    tetr_stone current_stone;      /* der gerade fliegende Stein ("Flugstein") */
    stone_shape *next_shape;       /* Form des naechsten Steins ("Next"-Feld) */
    int m_score, m_lines;

    NotifyFunc GameEndNotify, ScoreChangeNotify;

    void DropDownCurrentStone (void);
    bool FitsInPlayfield (tetr_stone& tstone);
    bool PointInHeap (int col, int row);
    void SetCurrentStone (const tetr_stone& new_one);

    bool GameRunning;

    GdkColor CL_BLACK,CL_WHITE;

    GdkColor* m_basiccols;
    static const int m_nbasiccols;

    //TODO: So gehts nicht ("incomplete Type CTetrisGameProcess has no member compare_colors")
    //compare_colors global definieren (nicht besser als jetziger globaler operator<)
    //geht auch nicht ("warning: ANSI C++ forbids initialization of member `m_allocatedColors")
    //in MSVC++ testen, was das soll!
    //static bool compare_colors (const GdkColor& c1, const GdkColor& c2);
    //std::map<GdkColor,int> m_allocatedColors(CTetrisGameProcess::compare_colors);

    std::map<GdkColor,int> m_allocatedColors;

    void AllocateColor (const GdkColor& cl, int number);
    void FreeColor (const GdkColor& cl, int number);

    static void on_playfield_realized
        (GtkWidget* playfield, CTetrisGameProcess* static_this);

public:
    //Konstruktor
    //PlayField, NextField sind die GtkBrickViewer, in denen das Spiel dargestellt wird
    CTetrisGameProcess(GtkBrickViewer* PlayField, GtkBrickViewer* NextField);
    virtual ~CTetrisGameProcess();

    const CPoint PFExtend;  //Spielfeld-Ausmasse

    StoneColorRange m_StoneColorRange;

    //StepForth: Spiel einen Schritt weitersetzen
    //Rueckgabe false, falls Spielende
    bool StepForth ();
    //Tastendruck verarbeiten
    void ProcessKey(unsigned Key);

    //Spiel anhalten
    void StopGame ();
    //Spiel neustarten
    void StartNewGame ();
    bool IsGameRunning ()
    { return GameRunning; }

    static const int      //Punktgutschriften fuer
        inc_stone,        // 1 abgesetzten Tetris-Stein
        inc_drop1line,    // "Abwerfen" (SPACE) pro Linie
        inc_line;         // 1 volle Linie

    int GetScore ()
    { return m_score; }
    int GetLines ()
    { return m_lines; }
};


#endif
