#ifndef TETRISGAMEPROCESS_H
#define TETRISGAMEPROCESS_H

#include "gtkbrickviewer.h"
#include "utils.h"


struct tetr_stone     /* ein Tetris-Stein */
{
    CPoint position;           /* position relativ zur oberen, linken Ecke des Spielfeldes */
    struct stone_shape shape;  /* Form des Steins */
    GdkColor colour;           /* Farbe des Steins */
    bool valid;                /* Info, ob Stein guetig ist */
};


enum TGameStatus {gsStopped,gsRunning,gsPaused};


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
