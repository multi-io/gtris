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
#include "xpms.h"

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

static void OnKeyPressed(GtkWidget*, GdkEventKey* event, gpointer);
static void OnGameStop(GtkWidget*, gpointer);
static void GameEndNotify ();

static void create_menu_and_toolbar( GtkWidget  *window, GtkWidget **menubar, GtkWidget **toolbar );

static gint delete_wnd_event
    ( GtkWidget*, GdkEvent, gpointer )
{
    OnGameStop(NULL,NULL);
    if (m_pGameProcess->IsGameRunning())
        return TRUE;

    if (!m_pHighscoresManager->SaveToFile (m_HscFile.c_str()))
    {
        if (MB_NO == MsgBox
            ("Warning", "Error while saving the highscores file.\n\
                         Path of this file can be set via Options/Game Options.\n\
                         Quit Game?", MB_YESNO))
            return TRUE;
    }

    return FALSE;
}

static void destroy_wnd_event (GtkWidget*, gpointer)
{
    gtk_main_quit ();
}

static void OnGameExit(GtkWidget*, gpointer) 
{
    if (!delete_wnd_event(NULL,GdkEvent(),NULL))
        destroy_wnd_event (NULL,NULL);
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

    if ( registry.QueryValue ("Level", regbuffer) )
        if (regbuffer <= 4)
            m_Level = m_NewLevel = regbuffer;

    if ( registry.QueryValue ("StoneColorRange", regbuffer) )
        m_pGameProcess->m_StoneColorRange = (CTetrisGameProcess::StoneColorRange)regbuffer;

    m_pGameProcess->m_StoneColorRange = CTetrisGameProcess::scrBasic;

    if ( !registry.QueryValue ("HscFile", m_HscFile) )
    {
        m_HscFile = g_get_home_dir();
        m_HscFile += "/.gtris/hscores.bin";
    }

    if ( !registry.QueryValue ("HscUserName", m_HscUserName) )
    {
        m_HscUserName = g_get_user_name();
    }

    m_pHighscoresManager->LoadFromFile (m_HscFile.c_str());

    GtkWidget* wnd;
    wnd = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW(wnd),"GTris");

    gtk_signal_connect (GTK_OBJECT (wnd), "delete_event", GTK_SIGNAL_FUNC (delete_wnd_event), NULL);
    gtk_signal_connect (GTK_OBJECT(wnd), "destroy", GTK_SIGNAL_FUNC(destroy_wnd_event), NULL);

    GtkVBox* main_vbox = GTK_VBOX(gtk_vbox_new (FALSE, 1));
    gtk_container_border_width (GTK_CONTAINER(main_vbox), 1);
    gtk_container_add (GTK_CONTAINER(wnd), GTK_WIDGET(main_vbox));

    GtkWidget* menubar;
    GtkWidget* toolbar;
    create_menu_and_toolbar (wnd, &menubar, &toolbar);
    GtkWidget* menuhb = gtk_handle_box_new ();
    GtkWidget* toolhb = gtk_handle_box_new ();
    gtk_box_pack_start ( GTK_BOX (main_vbox), menuhb, FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX (main_vbox), toolhb, FALSE, FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER ( menuhb ) , menubar );
    gtk_container_add ( GTK_CONTAINER ( toolhb ) , toolbar );
    gtk_widget_show (menuhb);
    gtk_widget_show (toolhb);
    gtk_widget_show (menubar);
    gtk_widget_show (toolbar);

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

    gtk_main ();

    delete m_pGameProcess;
    delete m_pHighscoresManager;

    registry.SetValue ("Level", m_Level);
    registry.SetValue ("StoneColorRange", (int)m_pGameProcess->m_StoneColorRange);
    registry.SetValue ("HscFile", m_HscFile);
    registry.SetValue ("HscUserName", m_HscUserName);

    return 0;
}


static gint OnTimeout (gpointer)
{
    if (!GamePaused)
    {
        gdk_threads_enter ();
        m_pGameProcess->StepForth ();
        UpdateStatusbars();
        gdk_threads_leave ();
    }
}



static void OnKeyPressed(GtkWidget*, GdkEventKey* event, gpointer) 
{
    if (!GamePaused)
    {
        m_pGameProcess->ProcessKey (event->keyval);
        UpdateStatusbars();
    }
}


static void OnGameNew();
static void OnGameStop(GtkWidget*, gpointer);
static void OnGameRun(GtkWidget*, gpointer);
static void OnGamePause(GtkWidget*, gpointer);
static void OnViewHighscores(GtkWidget*, gpointer);
static void OnOptionsGameOptions(GtkWidget*, gpointer);

static GtkMenuItem* m_mniGameNew;
static GtkMenuItem* m_mniGameRun;
static GtkMenuItem* m_mniGameStop;
static GtkMenuItem* m_mniGamePause;
static GtkCheckMenuItem* m_mniViewHighscores;

struct ToolbarItem
{
    char label[20];
    char tooltip[30];
    char** xpmdata;
    GtkSignalFunc callback;
    GtkWidget* widget;
};

static ToolbarItem m_tbitems[] =
{
    {"New","new game",new_xpm,&OnGameNew,NULL},
    {"Run","run game",run_xpm,&OnGameRun,NULL},
    {"Stop","stop game",stop_xpm,&OnGameStop,NULL},
    {"Pause","pause game",pause_xpm,&OnGamePause,NULL},
    {"Options","game options",options_xpm,&OnOptionsGameOptions,NULL},
    {"Highscores","show/hide highscores",highscores_xpm,&OnViewHighscores,NULL}
};

static int m_ntbitems = sizeof(m_tbitems)/sizeof(m_tbitems[0]);

typedef enum
{
    i_tbiNew = 0,
    i_tbiRun,
    i_tbiStop,
    i_tbiPause,
    i_tbiOptions,
    i_tbiHighscores
};

static GtkItemFactoryEntry menu_items[] = {
  { "/_Game",        (char*)NULL,  (GtkItemFactoryCallback)NULL, 0, "<Branch>" },
  { "/Game/_New",    "<control>N", OnGameNew,   0, NULL },
  { "/Game/_Run",    "F6", OnGameRun,   0, NULL },
  { "/Game/_Stop",   "F8", OnGameStop,  0, NULL },
  { "/Game/_Pause",  "F5",  OnGamePause, 0, NULL },
  { "/Game/",  (char*)NULL,  NULL, 0, "<Separator>" },
  { "/Game/E_xit",  "<alt>F4",  OnGameExit, 0, NULL },
  { "/_View",        (char*)NULL,  (GtkItemFactoryCallback)NULL, 0, "<Branch>" },
  { "/View/_Highscores",  (char*)NULL,  OnViewHighscores, 0, "<ToggleItem>" },
  { "/_Options",        (char*)NULL,  (GtkItemFactoryCallback)NULL, 0, "<Branch>" },
  { "/Options/_Game Options",  (char*)NULL,  OnOptionsGameOptions, 0, NULL }
};

static void UpdateItems ();

static void create_menu_and_toolbar( GtkWidget  *window, GtkWidget **menubar, GtkWidget **toolbar )
{
    if (!GTK_WIDGET_REALIZED(GTK_WIDGET(window)))
        gtk_widget_realize (GTK_WIDGET(window));

    GtkItemFactory *item_factory;
    GtkAccelGroup *accel_group;
    gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);

    accel_group = gtk_accel_group_new ();
    item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",
                                         accel_group);
    gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);
    gtk_accel_group_attach (accel_group, GTK_OBJECT (window));
           
    m_mniGameNew =
        GTK_MENU_ITEM( gtk_item_factory_get_widget (item_factory, "/Game/New") );
    m_mniGameRun =
        GTK_MENU_ITEM( gtk_item_factory_get_widget (item_factory, "/Game/Run") );
    m_mniGameStop =
        GTK_MENU_ITEM( gtk_item_factory_get_widget (item_factory, "/Game/Stop") );
    m_mniGamePause =
        GTK_MENU_ITEM( gtk_item_factory_get_widget (item_factory, "/Game/Pause") );
    m_mniViewHighscores =
        GTK_CHECK_MENU_ITEM( gtk_item_factory_get_widget (item_factory, "/View/Highscores") );

    *menubar = gtk_item_factory_get_widget (item_factory, "<main>");

    *toolbar = gtk_toolbar_new(GTK_ORIENTATION_HORIZONTAL,GTK_TOOLBAR_ICONS);
    for (int i=0; i<m_ntbitems; i++)
    {
        ToolbarItem& tbi = m_tbitems[i];
        GdkBitmap* mask;
        GdkPixmap* gdkpx = gdk_pixmap_create_from_xpm_d
            (window->window, &mask, NULL, tbi.xpmdata );
        GtkWidget* gtkpx = gtk_pixmap_new (gdkpx,mask);
        tbi.widget = gtk_toolbar_append_item (GTK_TOOLBAR(*toolbar), tbi.label, tbi.tooltip, NULL,
                                 gtkpx, tbi.callback, NULL);
    }

    UpdateItems ();
}


static void UpdateItems ()
{
    bool bEnabled;
    bEnabled =  (!m_pGameProcess->IsGameRunning()) ||
                (GamePaused && m_pGameProcess->IsGameRunning());
    gtk_widget_set_sensitive (GTK_WIDGET(m_mniGameRun),bEnabled);
    gtk_widget_set_sensitive (m_tbitems[i_tbiRun].widget,bEnabled);

    bEnabled =  m_pGameProcess->IsGameRunning();
    gtk_widget_set_sensitive (GTK_WIDGET(m_mniGameStop),bEnabled);
    gtk_widget_set_sensitive (m_tbitems[i_tbiStop].widget,bEnabled);

    bEnabled =  m_pGameProcess->IsGameRunning() && !GamePaused;
    gtk_widget_set_sensitive (GTK_WIDGET(m_mniGamePause),bEnabled);
    gtk_widget_set_sensitive (m_tbitems[i_tbiPause].widget,bEnabled);
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
    UpdateItems ();
}

static void OnGameStop(GtkWidget*, gpointer)
{
    if (m_pGameProcess->IsGameRunning())
        if (MB_YES != MsgBox ("Warning", "Game still running. Proceed?", MB_YESNO))
            return;

    GamePaused = false;
    m_pGameProcess->StopGame ();
    if (timer_installed)
        gtk_timeout_remove (timeout_tag);
    UpdateItems ();
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
        timeout_tag = g_timeout_add_full (timer_priority, 55 * (5 - m_Level), OnTimeout, NULL, NULL);
        timer_installed = true;
        m_pGameProcess->StartNewGame ();
    }
    UpdateItems ();
}

static void OnGamePause(GtkWidget*, gpointer) 
{
    GamePaused = true;
    UpdateItems ();
}


static void OnViewHighscores(GtkWidget*, gpointer) 
{
    m_pHighscoresManager->ShowDialog (m_pHighscoresManager->IsDialogVisible()?false:true);
}


static void GameEndNotify ()
{
    UpdateItems ();
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
            m_pHighscoresManager->LoadFromFile (m_HscFile.c_str());
        }
    }
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
