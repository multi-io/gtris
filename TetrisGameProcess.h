#ifndef TETRISGAMEPROCESS_H
#define TETRISGAMEPROCESS_H

#include "gtkbrickviewer.h"
#include "types.h"


struct tetr_stone     /* ein Tetris-Stein */
{
    CPoint position;           /* position relativ zur oberen, linken Ecke des SFeldes */
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

    void drop_down (void);
    bool does_fit_in_pfield (struct tetr_stone tstone);
    bool PointInHeap (int col, int row);
    void fade_into (struct tetr_stone new_one);
    void RepaintCurrentStone();

    bool GameRunning;

public:
    CTetrisGameProcess(GtkBrickViewer* PlayField, GtkBrickViewer* NextField);
    virtual ~CTetrisGameProcess();

    const CPoint PFExtend;  //Spielfeld-Ausmass

    StoneColorRange m_StoneColorRange;

    void StepForth ();
    void ProcessKey(unsigned Key);

    void SetGameEndNotify (NotifyFunc f)
    { GameEndNotify = f; }
    void SetScoreChangeNotify (NotifyFunc f)
    { ScoreChangeNotify = f; }

    void StopGame ();
    void StartNewGame ();
    bool IsGameRunning ()
    { return GameRunning; }

                            //Punktgutschriften fuer
    static const int 
        inc_stone,        // 1 abgesetzten Tetris-Stein
        inc_drop1line,    // "Abwerfen" (SPACE) pro Linie
        inc_line;         // 1 volle Linie

    int GetScore ()
    { return m_score; }
    int GetLines ()
    { return m_lines; }
};


#endif
