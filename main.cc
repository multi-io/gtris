#include <gtk/gtk.h>
#include <string>
#include <stdio.h>
#include "gtkbrickviewer.h"
#include "types.h"
#include "TetrisGameProcess.h"
#include "HighscoresManager.h"
#include "options.h"
#include "msgbox.h"
#include "registry.h"


using namespace std;

static GtkBrickViewer*  m_bvPlayField;
static GtkBrickViewer*  m_bvNextField;

static CTetrisGameProcess* m_pGameProcess;

static bool GamePaused;
static unsigned m_Level, m_NewLevel;

const CPoint PlayFieldSize (12,23);
const CPoint NextFieldSize (5,5);

static HighscoresManager* m_pHighscoresManager;

static string m_HscFile;
static string m_HscUserName;

static uint timeout_tag;
static bool timer_installed = false;
const timer_priority = -10;

static void InitStatusbars (GtkStatusbar** sbLevel, GtkStatusbar** sbScore, GtkStatusbar** sbLines);
static void UpdateStatusbars ();


static gint OnTimeout (gpointer)
{
    if (!GamePaused)
    {
        m_pGameProcess->StepForth ();
        UpdateStatusbars();
    }
}



static void OnKeyPressed(GtkWidget*, GdkEventKey* event, gpointer) 
{
    if (!GamePaused)
    {
        if (event->keyval == 'u')
        {
            m_pGameProcess->StepForth ();
        }
        else
        {
            m_pGameProcess->ProcessKey (event->keyval);
        }
        UpdateStatusbars();
    }
}


static void OnGameNew()
{
    if (m_pGameProcess->IsGameRunning())
        if (MB_NO == MsgBox ("Warning", "Game still running. Proceed?", MB_YESNO))
            return;

    GamePaused = false;
    if (timer_installed)
        gtk_timeout_remove (timeout_tag);
    m_Level = m_NewLevel;
    timeout_tag = g_timeout_add_full (timer_priority, 55 * (5 - m_Level), OnTimeout, NULL, NULL);
    timer_installed = true;
    m_pGameProcess->StartNewGame ();
}

static void OnGameStop(GtkWidget*, gpointer)
{
    if (m_pGameProcess->IsGameRunning())
        if (MB_NO == MsgBox ("Warning", "Game still running. Proceed?", MB_YESNO))
            return;

    GamePaused = false;
    m_pGameProcess->StopGame ();
    if (timer_installed)
        gtk_timeout_remove (timeout_tag);
}

static void OnGameRun(GtkWidget*, gpointer) 
{
    if (GamePaused)
        GamePaused = false;
    else
    {
        GamePaused = false;
        if (timer_installed)
            gtk_timeout_remove (timeout_tag);
        m_Level = m_NewLevel;
        timeout_tag = g_timeout_add_full (timer_priority, 55 * (5 - m_Level), OnTimeout, NULL, NULL);;
        timer_installed = true;
        m_pGameProcess->StartNewGame ();
    }
}

static void OnGamePause(GtkWidget*, gpointer) 
{
    GamePaused = true;
}


static void OnViewHighscores(GtkWidget*, gpointer) 
{
    m_pHighscoresManager->ShowDialog (m_pHighscoresManager->IsDialogVisible()?false:true);
}


// void CTetrisWnd::OnUpdateGamePause(CCmdUI* pCmdUI) 
// {
//     pCmdUI->Enable (m_gpGameProcess.IsGameRunning() && !GamePaused);
// }

// void CTetrisWnd::OnUpdateGameRun(CCmdUI* pCmdUI) 
// {
//     pCmdUI->Enable ((!m_gpGameProcess.IsGameRunning()) ||
//                     (GamePaused && m_gpGameProcess.IsGameRunning()) );
// }

// void CTetrisWnd::OnUpdateGameStop(CCmdUI* pCmdUI) 
// {
//     pCmdUI->Enable (m_gpGameProcess.IsGameRunning());
// }



// void CTetrisWnd::OnClose() 
// {
//     OnGameStop();
//     if (m_gpGameProcess.IsGameRunning())
//         return;

//     if (!m_HighscoresDialog.SaveToFile (m_HscFile))
//     {
//         if (IDNO == AfxMessageBox (IDS_ERROR_HSCFILE, MB_YESNO))
//             return;
//     }

//     CWnd::OnClose();
// }



// void CTetrisWnd::OnUpdateGameStatus(CCmdUI* pCmdUI)
// {
//     CString StatusText;
//     if (m_gpGameProcess.IsGameRunning())
//     {
//         if (GamePaused)
//             StatusText.LoadString (IDS_MSG_GAME_PAUSED);
//         else
//             StatusText.LoadString (IDS_MSG_GAME_RUNNING);
//     }
//     else
//         StatusText.LoadString (IDS_MSG_GAME_STOPPED);

//     pCmdUI->SetText (StatusText);
// }


// void CTetrisWnd::OnUpdateScore(CCmdUI* pCmdUI)
// {
//     char txt[10];
//     _itoa (m_gpGameProcess.GetScore(),txt,10);
//     pCmdUI->SetText (txt);
// }


// void CTetrisWnd::OnUpdateLines(CCmdUI* pCmdUI)
// {
//     char txt[10];
//     _itoa (m_gpGameProcess.GetLines(),txt,10);
//     pCmdUI->SetText (txt);
// }


// void CTetrisWnd::OnUpdateLevel(CCmdUI* pCmdUI)
// {
//     char txt[10];
//     _itoa (m_Level,txt,10);
//     pCmdUI->SetText (txt);
// }


static void GameEndNotify ()
{
    int score = m_pGameProcess->GetScore ();
    if (score > m_pHighscoresManager->GetLeastScore (m_Level))
    {
        THscEntry hscentry;
        hscentry.score = score;
        hscentry.lines = m_pGameProcess->GetLines ();
        time (&hscentry.date);
        hscentry.name = m_HscUserName;

        if (HighscoresManager::HighscoresUserQuery (&hscentry, m_Level))
        {
            m_pHighscoresManager->AddNewEntry (hscentry,m_Level);
            m_HscUserName = hscentry.name;
        }
    }
}


// void CTetrisWnd::OnSize(UINT nType, int cx, int cy) 
// {
// 	CWnd::OnSize(nType, cx, cy);
    
//     int bwx = (cx-10) / (PlayFieldSize.cx + NextFieldSize.cx +1),
//         bwy = (cy-10) / PlayFieldSize.cy,
//         bw = bwx < bwy? bwx:bwy;

//     m_bvPlayField.SetBrickSize (bw);
//     m_bvNextField.SetBrickSize (bw);

//     m_bvNextField.SetWindowPos
//         (NULL, 5 + bw * (PlayFieldSize.cx+1), 5, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
// }


// void CTetrisWnd::CalcNearestOptimalSize(CSize& fromSize, CSize& result)
// {
//     int bwx = (fromSize.cx-10) / (PlayFieldSize.cx + NextFieldSize.cx +1),
//         bwy = (fromSize.cy-10) / PlayFieldSize.cy,
//         bw = bwx < bwy? bwx:bwy;
//     if (bw < 5)
//         bw = 5;

//     result.cx = 15 + bw * (PlayFieldSize.cx + NextFieldSize.cx + 1);
//     result.cy = 15 + bw * PlayFieldSize.cy;
// }


static void OnOptionsGameOptions(GtkWidget*, gpointer) 
{
    int level = m_Level;
    CTetrisGameProcess::StoneColorRange scr = m_pGameProcess->m_StoneColorRange;
    string hscfile = m_HscFile;

    if (GetOptions (&level, &hscfile, &scr))
    {
        m_NewLevel = level;
        if (!m_pGameProcess->IsGameRunning ())
            m_Level = m_NewLevel;
        m_pGameProcess->m_StoneColorRange = scr;
        if (m_HscFile != hscfile)
        {
            m_HscFile = hscfile;
            m_pHighscoresManager->LoadFromFile (m_HscFile);
        }
    }
}


// void CTetrisWnd::OnOptionsLevel( UINT nID )
// {
//     m_NewLevel = nID - ID_OPTIONS_LEVEL0;
//     ASSERT (m_NewLevel>=0 && m_NewLevel<5);
//     if (!m_gpGameProcess.IsGameRunning ())
//         m_Level = m_NewLevel;
// }

// void CTetrisWnd::OnUpdateOptionsLevel (CCmdUI* pCmdUI)
// {
//     pCmdUI->SetRadio (pCmdUI->m_nID - ID_OPTIONS_LEVEL0 == m_Level?1:0);
// }


// void CTetrisWnd::OnViewHighscores() 
// {
//     m_HighscoresDialog.ShowWindow
//         (m_HighscoresDialog.IsWindowVisible()?SW_HIDE:SW_SHOW);
// }

// void CTetrisWnd::OnUpdateViewHighscores(CCmdUI* pCmdUI) 
// {
//     pCmdUI->SetCheck (m_HighscoresDialog.IsWindowVisible()?1:0);
// }

// void CTetrisWnd::OnShowWindow(BOOL bShow, UINT nStatus) 
// {
// 	CWnd::OnShowWindow(bShow, nStatus);

//     if (bShow)
//     {
//         DWORD w;
//         if (ERROR_SUCCESS == registry.QueryValue ("HscVisible", &w))
//             m_HighscoresDialog.ShowWindow (w==0?SW_HIDE:SW_SHOW);
//     }
//     else
//     {
//         registry.SetValue ("HscVisible", m_HighscoresDialog.IsWindowVisible ()?1:0);
//     }
	
// }


// void CTetrisWnd::OnClose() 
// {
//     OnGameStop();
//     if (m_gpGameProcess.IsGameRunning())
//         return;

//     if (!m_HighscoresDialog.SaveToFile (m_HscFile))
//     {
//         if (IDNO == AfxMessageBox (IDS_ERROR_HSCFILE, MB_YESNO))
//             return;
//     }

//     CWnd::OnClose();
// }



static void get_main_menu( GtkWidget  *window, GtkWidget **menubar );


static gint delete_wnd_event
    ( GtkWidget*, GdkEvent, gpointer )
{
    OnGameStop(NULL,NULL);
    if (m_pGameProcess->IsGameRunning())
        return TRUE;

    if (!m_pHighscoresManager->SaveToFile (m_HscFile))
    {
        if (MB_NO == MsgBox
            ("Warning", "Error while saving the highscores file.\n Path of this file can be set via Options/Game Options.\nQuit Game?", MB_YESNO))
            return FALSE;
    }

    return FALSE;
}


static void destroy_wnd_event (GtkWidget*, gpointer)
{
    gtk_main_quit ();
}



int main (int argc, char* argv[])
{
    gtk_init (&argc,&argv);

    m_pHighscoresManager = new HighscoresManager;

    m_bvPlayField = GTK_BRICK_VIEWER(gtk_brick_viewer_new (PlayFieldSize.x,PlayFieldSize.y,15));
    m_bvNextField = GTK_BRICK_VIEWER(gtk_brick_viewer_new (NextFieldSize.x,NextFieldSize.y,15));

    m_pGameProcess = new CTetrisGameProcess (m_bvPlayField, m_bvNextField);

    m_pGameProcess->SetGameEndNotify (GameEndNotify);

    GamePaused = false;
    m_Level =  m_NewLevel = 0;

    int regbuffer;

    if ( registry.QueryValue ("Speed", &regbuffer) )
        if (regbuffer <= 4)
            m_Level = m_NewLevel = regbuffer;

    if ( registry.QueryValue ("StoneColorRange", &regbuffer) )
        m_pGameProcess->m_StoneColorRange = (CTetrisGameProcess::StoneColorRange)regbuffer;

    m_pGameProcess->m_StoneColorRange = CTetrisGameProcess::scrBasic;

    if ( !registry.QueryValue ("HscFile", m_HscFile) )
    {
        m_HscFile = "/home/olaf/.gtris/hscores.bin";
    }
    m_HscFile = "/home/olaf/.gtris/hscores.bin";

    if ( !registry.QueryValue ("HscUserName", m_HscUserName) )
    {
        m_HscUserName = "olaf";
    }

    m_pHighscoresManager->LoadFromFile (m_HscFile);

    GtkWidget* wnd;
    wnd = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW(wnd),"GTris");

    gtk_signal_connect (GTK_OBJECT (wnd), "delete_event", GTK_SIGNAL_FUNC (delete_wnd_event), NULL);
    gtk_signal_connect (GTK_OBJECT(wnd), "destroy", GTK_SIGNAL_FUNC(destroy_wnd_event), NULL);

    GtkVBox* main_vbox = GTK_VBOX(gtk_vbox_new (FALSE, 1));
    gtk_container_border_width (GTK_CONTAINER(main_vbox), 1);
    gtk_container_add (GTK_CONTAINER(wnd), GTK_WIDGET(main_vbox));

    GtkWidget* menubar;
    get_main_menu (wnd, &menubar);
    gtk_box_pack_start (GTK_BOX(main_vbox), menubar, FALSE, TRUE, 0);
    gtk_widget_show (menubar);

    GtkHBox* centerBox = GTK_HBOX(gtk_hbox_new(FALSE,0));
    gtk_box_pack_start (GTK_BOX(main_vbox), GTK_WIDGET(centerBox), FALSE, TRUE, 0);


    GtkHBox* statusBox = GTK_HBOX(gtk_hbox_new(FALSE,0));
    gtk_box_pack_start (GTK_BOX(main_vbox), GTK_WIDGET(statusBox), FALSE, TRUE, 0);
    gtk_widget_show (GTK_WIDGET(statusBox));

    GtkStatusbar *sbLevel, *sbScore, *sbLines;
    InitStatusbars (&sbLevel, &sbScore, &sbLines);
    gtk_box_pack_start (GTK_BOX(statusBox), GTK_WIDGET(sbScore), FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX(statusBox), GTK_WIDGET(sbLines), FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX(statusBox), GTK_WIDGET(sbLevel), FALSE, TRUE, 0);
    gtk_widget_show (GTK_WIDGET(sbScore));
    gtk_widget_show (GTK_WIDGET(sbLines));
    gtk_widget_show (GTK_WIDGET(sbLevel));


    GtkWidget* sep = gtk_vseparator_new();
    gtk_box_pack_start (GTK_BOX(centerBox),GTK_WIDGET(m_bvPlayField),TRUE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(centerBox),sep,TRUE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(centerBox),GTK_WIDGET(m_bvNextField),TRUE,TRUE,0);

    gtk_signal_connect (GTK_OBJECT(wnd), "key_press_event", GTK_SIGNAL_FUNC(OnKeyPressed), NULL);

    gtk_widget_show (GTK_WIDGET(m_bvPlayField));
    gtk_widget_show (GTK_WIDGET(m_bvNextField));
    gtk_widget_show (GTK_WIDGET(sep));
    gtk_widget_show (GTK_WIDGET(centerBox));
    gtk_widget_show (GTK_WIDGET(menubar));
    gtk_widget_show (GTK_WIDGET(main_vbox));
    gtk_widget_show (wnd);


//     GtkButton* btn = GTK_BUTTON(gtk_button_new_with_label("manuelles Update"));
//     gtk_box_pack_start (GTK_BOX(centerBox),GTK_WIDGET(btn),TRUE,TRUE,0);
//     gtk_widget_show (GTK_WIDGET(btn));
//     gtk_signal_connect (GTK_OBJECT(btn), "clicked", GTK_SIGNAL_FUNC(manUpdate), NULL);

    gtk_main ();

    delete m_pGameProcess;
    delete m_pHighscoresManager;

    registry.SetValue ("Speed", m_Level);
    registry.SetValue ("StoneColorRange", (int)m_pGameProcess->m_StoneColorRange);
    registry.SetValue ("HscFile", m_HscFile);
    registry.SetValue ("HscUserName", m_HscUserName);

    return 0;
}



static GtkItemFactoryEntry menu_items[] = {
  { "/_Game",        (char*)NULL,  (GtkItemFactoryCallback)NULL, 0, "<Branch>" },
  { "/Game/_New",    "<control>N", OnGameNew,   0, NULL },
  { "/Game/_Run",    "F6", OnGameRun,   0, NULL },
  { "/Game/_Stop",   "F8", OnGameStop,  0, NULL },
  { "/Game/_Pause",  "F5",  OnGamePause, 0, NULL },
  { "/_View",        (char*)NULL,  (GtkItemFactoryCallback)NULL, 0, "<Branch>" },
  { "/View/_Highscores",  (char*)NULL,  OnViewHighscores, 0, NULL },
  { "/_Options",        (char*)NULL,  (GtkItemFactoryCallback)NULL, 0, "<Branch>" },
  { "/Options/_Game Options",  (char*)NULL,  OnOptionsGameOptions, 0, NULL }
};


static void get_main_menu( GtkWidget  *window, GtkWidget **menubar )
{
    GtkItemFactory *item_factory;
    GtkAccelGroup *accel_group;
    gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);

    accel_group = gtk_accel_group_new ();

    /* This function initializes the item factory.
       Param 1: The type of menu - can be GTK_TYPE_MENU_BAR, GTK_TYPE_MENU,
       or GTK_TYPE_OPTION_MENU.
       Param 2: The path of the menu.
       Param 3: A pointer to a gtk_accel_group.  The item factory sets up
       the accelerator table while generating menus.
    */
           
    item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",
                                         accel_group);
           
    /* This function generates the menu items. Pass the item factory,
       the number of items in the array, the array itself, and any
       callback data for the the menu items. */
    gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);
     
    /* Attach the new accelerator group to the window. */
    gtk_accel_group_attach (accel_group, GTK_OBJECT (window));
           
    if (menubar)
        *menubar = gtk_item_factory_get_widget (item_factory, "<main>");
}



static GtkStatusbar
    *m_sbLevel, *m_sbScore, *m_sbLines;

static unsigned m_cidLevel,m_midLevel;
static unsigned m_cidScore,m_midScore;
static unsigned m_cidLines,m_midLines;

static void InitStatusbars (GtkStatusbar** sbLevel, GtkStatusbar** sbScore, GtkStatusbar** sbLines)
{
    m_sbLevel = GTK_STATUSBAR( gtk_statusbar_new () );
    m_cidLevel = gtk_statusbar_get_context_id (m_sbLevel, "level_ctx");
    m_midLevel = gtk_statusbar_push (m_sbLevel, m_cidLevel, "         ");
    *sbLevel = m_sbLevel;

    m_sbScore = GTK_STATUSBAR( gtk_statusbar_new () );
    m_cidScore = gtk_statusbar_get_context_id (m_sbScore, "score_ctx");
    m_midScore = gtk_statusbar_push (m_sbScore, m_cidScore, "         ");
    *sbScore = m_sbScore;

    m_sbLines = GTK_STATUSBAR( gtk_statusbar_new () );
    m_cidLines = gtk_statusbar_get_context_id (m_sbLines, "lines_ctx");
    m_midLines = gtk_statusbar_push (m_sbLines, m_cidLines, "         ");
    *sbLines = m_sbLines;
}

static void UpdateStatusbars ()
{
    static int
        prevLevel = -1,
        prevLines = -1,
        prevScore = -1;

    char buf[20];

    if (m_Level != prevLevel)
    {
        gtk_statusbar_remove (m_sbLevel, m_cidLevel, m_midLevel);
        sprintf (buf,"Level %i",m_Level);
        m_midLevel = gtk_statusbar_push (m_sbLevel, m_cidLevel, buf);
        prevLevel = m_Level;
    }

    if (m_pGameProcess->GetScore() != prevScore)
    {
        gtk_statusbar_remove (m_sbScore, m_cidScore, m_midScore);
        sprintf (buf,"Score: %i",m_pGameProcess->GetScore());
        m_midScore = gtk_statusbar_push (m_sbScore, m_cidScore, buf);
        prevScore = m_pGameProcess->GetScore();
    }

    if (m_pGameProcess->GetLines() != prevLines)
    {
        gtk_statusbar_remove (m_sbLines, m_cidLines, m_midLines);
        sprintf (buf,"Lines: %i",m_pGameProcess->GetLines());
        m_midLines = gtk_statusbar_push (m_sbLines, m_cidLines, buf);
        prevLines = m_pGameProcess->GetLines();
    }
}
