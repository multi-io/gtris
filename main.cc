/*  $Id: main.cc,v 1.6.2.2 1999/10/24 10:40:22 olaf Exp $ */

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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdio.h>
#include <string>
#include "gtkbrickviewer.h"
#include "utils.h"
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

static void CreateStatusbar (GtkWidget** statusbar);
static void UpdateScoreDisplay();
static void UpdateLinesDisplay();
static void UpdateLevelDisplay();

static void OnKeyPressed(GtkWidget*, GdkEventKey* event, gpointer);
static void OnGameStop();

static void CreateMenuAndToolbar( GtkWidget  *window, GtkWidget **menubar, GtkWidget **toolbar );


static GtkWidget* m_mainwnd;

static gint delete_wnd_event
    ( GtkWidget*, GdkEvent, gpointer )
{
    OnGameStop();
    if (m_pGameProcess->IsGameRunning())
        return TRUE;

    if (!m_pHighscoresManager->SaveToFile (m_HscFile.c_str()))
    {
        if (MB_YES != MsgBox
            ("Warning",
"Error while saving the highscores file.\n\
Path of this file can be set via Options/Game Options.\n\
Quit Game?", MB_YESNO))
            return TRUE;
    }

    int x=100,y=100;
    gdk_window_get_root_origin (m_mainwnd->window,&x,&y);
    registry.SetValue ("WndPosX",x);
    registry.SetValue ("WndPosY",y);

    return FALSE;
}

static void destroy_wnd_event (GtkWidget*, gpointer)
{
    gtk_main_quit ();
}

static void OnGameExit() 
{
    if (!delete_wnd_event(NULL,GdkEvent(),NULL))
        destroy_wnd_event (NULL,NULL);
}


int main (int argc, char* argv[])
{
    gtk_init (&argc,&argv);

    m_pHighscoresManager = new HighscoresManager;

    GamePaused = false;
    m_Level =  m_NewLevel = 0;

    unsigned regbuffer;

    if ( registry.QueryValue ("Level", regbuffer) )
        if (regbuffer <= 4)
            m_Level = m_NewLevel = regbuffer;

    unsigned bs = 15;
    if ( registry.QueryValue ("BrickSize", regbuffer) )
        if (regbuffer <= 100)
            bs = regbuffer;

    m_bvPlayField = 
        GTK_BRICK_VIEWER(gtk_brick_viewer_new (PlayFieldSize.x,PlayFieldSize.y,bs));
    m_bvNextField =
        GTK_BRICK_VIEWER(gtk_brick_viewer_new (NextFieldSize.x,NextFieldSize.y,bs));

    m_pGameProcess = new CTetrisGameProcess (m_bvPlayField, m_bvNextField);

    m_pGameProcess->m_StoneColorRange = CTetrisGameProcess::scrBasic;
    if ( registry.QueryValue ("StoneColorRange", regbuffer) )
    {
        //einfaches Casten von unsigned nach CTetrisGameProcess::StoneColorRange produziert
        //ab und zu Muell
        //obwohl IMO lt. Standard enum - und int - Typen eineindeutig ineinander umwandelbar sein sollten
        //==> egcs - Bug??
        m_pGameProcess->m_StoneColorRange =
            regbuffer==0? CTetrisGameProcess::scrBlackWhite :
                         (regbuffer==1? CTetrisGameProcess::scrBasic : CTetrisGameProcess::scrWide);
    }

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

    m_mainwnd = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title (GTK_WINDOW(m_mainwnd),"GTris");

    gtk_signal_connect (GTK_OBJECT (m_mainwnd),
                        "delete_event",
                        GTK_SIGNAL_FUNC (delete_wnd_event),
                        NULL);
    gtk_signal_connect (GTK_OBJECT(m_mainwnd),
                        "destroy",
                        GTK_SIGNAL_FUNC(destroy_wnd_event),
                        NULL);

    GtkVBox* main_vbox = GTK_VBOX(gtk_vbox_new (FALSE, 1));
    gtk_container_border_width (GTK_CONTAINER(main_vbox), 1);
    gtk_container_add (GTK_CONTAINER(m_mainwnd), GTK_WIDGET(main_vbox));

    GtkWidget* menubar;
    GtkWidget* toolbar;
    CreateMenuAndToolbar (m_mainwnd, &menubar, &toolbar);
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

    GtkWidget* statusbar;
    CreateStatusbar (&statusbar);
    gtk_box_pack_start (GTK_BOX(main_vbox), GTK_WIDGET(statusbar), FALSE, TRUE, 0);
    gtk_widget_show (GTK_WIDGET(statusbar));

    GtkWidget* sep = gtk_vseparator_new();
    gtk_box_pack_start (GTK_BOX(centerBox),GTK_WIDGET(m_bvPlayField),TRUE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(centerBox),sep,TRUE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(centerBox),GTK_WIDGET(m_bvNextField),TRUE,TRUE,0);

    gtk_signal_connect (GTK_OBJECT(m_mainwnd),
                        "key_press_event",
                        GTK_SIGNAL_FUNC(OnKeyPressed),
                        NULL);

    gtk_widget_show (GTK_WIDGET(m_bvPlayField));
    gtk_widget_show (GTK_WIDGET(m_bvNextField));
    gtk_widget_show (GTK_WIDGET(sep));
    gtk_widget_show (GTK_WIDGET(centerBox));
    gtk_widget_show (GTK_WIDGET(menubar));
    gtk_widget_show (GTK_WIDGET(main_vbox));

    gtk_widget_show (m_mainwnd);

    int x=100, y=100;
    if ( registry.QueryValue ("WndPosX", regbuffer) )
        x = regbuffer;
    if ( registry.QueryValue ("WndPosY", regbuffer) )
        y = regbuffer;
    gdk_window_move (m_mainwnd->window,x,y);

    UpdateScoreDisplay();
    UpdateLinesDisplay();
    UpdateLevelDisplay();

    gtk_main ();

    registry.SetValue ("Level", m_Level);
    registry.SetValue ("StoneColorRange",
                       m_pGameProcess->m_StoneColorRange==CTetrisGameProcess::scrBlackWhite?
                              0 : (m_pGameProcess->m_StoneColorRange==CTetrisGameProcess::scrBasic? 1:2 ) );
    registry.SetValue ("HscFile", m_HscFile);
    registry.SetValue ("HscUserName", m_HscUserName);
    registry.SetValue ("BrickSize", gtk_brick_viewer_GetBrickSize (m_bvPlayField));

    delete m_pGameProcess;
    delete m_pHighscoresManager;

    return 0;
}


static gint OnTimeout (gpointer);


static void OnKeyPressed(GtkWidget*, GdkEventKey* event, gpointer) 
{
    if (!GamePaused)
    {
        m_pGameProcess->ProcessKey (event->keyval);
        UpdateScoreDisplay();
        UpdateLinesDisplay();
    }
}


static void OnGameNew();
static void OnGameRun();
static void OnGamePause();
static void OnViewHighscores();
static void OnOptionsGameOptions();
static void OnHelpAbout();
static void OnOptionsLevel (GtkWidget*, unsigned* pLevel);

static GtkMenuItem* m_mniGameNew;
static GtkMenuItem* m_mniGameRun;
static GtkMenuItem* m_mniGameStop;
static GtkMenuItem* m_mniGamePause;
static GtkMenuItem* m_mniViewHighscores;

struct ToolbarItem
{
    char label[20];
    char tooltip[30];
    char** xpmdata;
    GtkSignalFunc callback;
    GtkWidget* widget;
};

/* TODO: Die Casts nach GtkSignalFunc sind noetig, da egcs (1.0.2) sonst Warnungen ausspuckt,
 * obwohl die Signaturen der Funktionen exakt mit GtkSignalFunc uebereinstimmen => ??
 */

static ToolbarItem m_tbitems[] =
{
    {"New","new game",new_xpm,(GtkSignalFunc)OnGameNew,NULL},
    {"Run","run game",run_xpm,(GtkSignalFunc)OnGameRun,NULL},
    {"Stop","stop game",stop_xpm,(GtkSignalFunc)OnGameStop,NULL},
    {"Pause","pause game",pause_xpm,(GtkSignalFunc)OnGamePause,NULL},
    {"Options","game options",options_xpm,(GtkSignalFunc)OnOptionsGameOptions,NULL},
    {"Highscores","show/hide highscores",highscores_xpm,(GtkSignalFunc)OnViewHighscores,NULL}
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

/* TODO: s.o. */

static GtkItemFactoryEntry menu_items[] = {
  { "/_Game",        (char*)NULL,  NULL, 0, "<Branch>" },
  { "/Game/_New",    "<control>N", (GtkItemFactoryCallback)OnGameNew,   0, NULL },
  { "/Game/_Run",    "F6", (GtkItemFactoryCallback)OnGameRun,   0, NULL },
  { "/Game/_Stop",   "F8", (GtkItemFactoryCallback)OnGameStop,  0, NULL },
  { "/Game/_Pause",  "F5",  (GtkItemFactoryCallback)OnGamePause, 0, NULL },
  { "/Game/",  (char*)NULL,  NULL, 0, "<Separator>" },
  { "/Game/E_xit",  "<alt>F4",  (GtkItemFactoryCallback)OnGameExit, 0, NULL },
  { "/_View",        (char*)NULL,  NULL, 0, "<Branch>" },
  { "/View/_Highscores",  (char*)NULL,  (GtkItemFactoryCallback)OnViewHighscores, 0, NULL },
  { "/_Options",        (char*)NULL,  NULL, 0, "<Branch>" },
  { "/Options/_Game Options",  (char*)NULL,  (GtkItemFactoryCallback)OnOptionsGameOptions, 0, NULL },
  { "/_Help",        (char*)NULL,  NULL, 0, "<LastBranch>" },
  { "/Help/_About",  (char*)NULL,  (GtkItemFactoryCallback)OnHelpAbout, 0, NULL }
};

static void UpdateItems ();

static void CreateMenuAndToolbar( GtkWidget  *window, GtkWidget **menubar, GtkWidget **toolbar )
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
        GTK_MENU_ITEM( gtk_item_factory_get_widget (item_factory, "/View/Highscores") );


    static unsigned levels[nLevels];
    GtkMenu* om = GTK_MENU( gtk_item_factory_get_widget (item_factory,"/Options") );
    GtkWidget* sep = gtk_menu_item_new();
    gtk_widget_show (sep);
    gtk_menu_append (om,sep);
    for (int i=0; i<nLevels; i++)
    {
        levels[i] = i;

        GtkWidget* mni = gtk_menu_item_new ();

        GtkWidget* lbl = gtk_label_new ("");
        gtk_misc_set_alignment (GTK_MISC(lbl),0,0);
        gtk_container_add (GTK_CONTAINER(mni),lbl);
        gtk_widget_show (lbl);
        char buf[10];
        sprintf (buf,"Level _%i",i);
        guint accel_key = gtk_label_parse_uline (GTK_LABEL(lbl), buf);
        gtk_widget_add_accelerator
            (mni, "activate_item",
             (GtkAccelGroup*)(gtk_accel_groups_from_object (GTK_OBJECT(om)) ->data),
             accel_key,
             0, GTK_ACCEL_LOCKED);

        gtk_widget_show (mni);
        gtk_menu_append (om,mni);
        gtk_signal_connect (GTK_OBJECT (mni),
                            "activate",
                            GTK_SIGNAL_FUNC (OnOptionsLevel),
                            (gpointer) (levels+i));
    }

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

//TODO: der ganze Menue-/Toolbar-Kram saugt.
//==> was Cooleres ausdenken (Action-Klassen wie in Java/Swing)


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


static void OnGameNew(void)
{
    if (m_pGameProcess->IsGameRunning())
        if (MB_YES != MsgBox ("Warning", "Game still running. Proceed?", MB_YESNO))
            return;

    GamePaused = false;
    if (timer_installed)
        gtk_timeout_remove (timeout_tag);
    m_Level = m_NewLevel;
    UpdateLevelDisplay();
    timeout_tag = gtk_timeout_add (55 * (nLevels+1 - m_Level), OnTimeout, NULL);
    timer_installed = true;
    m_pGameProcess->StartNewGame ();
    UpdateItems ();
}

static void OnGameStop()
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

static void OnGameRun() 
{
    if (GamePaused)
        GamePaused = false;
    else
    {
        GamePaused = false;
        if (timer_installed)
            gtk_timeout_remove (timeout_tag);
        m_Level = m_NewLevel;
        UpdateLevelDisplay();
        timeout_tag = gtk_timeout_add (55 * (nLevels+1 - m_Level), OnTimeout, NULL);
        timer_installed = true;
        m_pGameProcess->StartNewGame ();
    }
    UpdateItems ();
}

static void OnGamePause() 
{
    GamePaused = true;
    UpdateItems ();
}


static void OnViewHighscores() 
{
    m_pHighscoresManager->ShowDialog (m_pHighscoresManager->IsDialogVisible()?false:true);
}


static void OnOptionsGameOptions() 
{
    unsigned level = m_Level;
    CTetrisGameProcess::StoneColorRange scr = m_pGameProcess->m_StoneColorRange;
    string hscfile = m_HscFile;
    unsigned bs = gtk_brick_viewer_GetBrickSize (m_bvPlayField);

    if (GetOptions (&level, &hscfile, &scr, &bs))
    {
        m_NewLevel = level;
        if (!m_pGameProcess->IsGameRunning ())
            m_Level = m_NewLevel;
        UpdateLevelDisplay();
        m_pGameProcess->m_StoneColorRange = scr;
        if (m_HscFile != hscfile)
        {
            m_HscFile = hscfile;
            m_pHighscoresManager->LoadFromFile (m_HscFile.c_str());
        }
        gtk_brick_viewer_SetBrickSize (m_bvPlayField,bs);
        gtk_brick_viewer_SetBrickSize (m_bvNextField,bs);
        GtkRequisition r;
        gtk_signal_emit_by_name (GTK_OBJECT (m_mainwnd), "size_request",&r);
        gtk_widget_set_usize (m_mainwnd,r.width,r.height);
        gdk_window_resize (m_mainwnd->window,r.width,r.height);
    }
}


static void OnOptionsLevel (GtkWidget*, unsigned* pLevel)
{
    m_NewLevel = *pLevel;
    if (!m_pGameProcess->IsGameRunning ())
        m_Level = m_NewLevel;
    UpdateLevelDisplay();
}


static GtkDialog* aboutbox;
static void on_aboutbox_close_click (GtkWidget*, gpointer p);
static gint on_aboutbox_delete ( GtkWidget*, GdkEvent, gpointer );

static void OnHelpAbout()
{
    aboutbox = GTK_DIALOG( gtk_dialog_new() );
    gtk_window_set_title ( GTK_WINDOW(aboutbox), "About GTris");
    gtk_widget_realize (GTK_WIDGET(aboutbox));

    GtkAccelGroup* accel_group = gtk_accel_group_new ();
    gtk_accel_group_attach (accel_group, GTK_OBJECT (aboutbox));

    GdkBitmap* mask;
    GdkPixmap* pm = gdk_pixmap_create_from_xpm
        (GTK_WIDGET(aboutbox)->window,
         &mask,
         &(gtk_widget_get_style (GTK_WIDGET(aboutbox))->bg[GTK_STATE_NORMAL]),
         "title.xpm");
    GtkPixmap* pmwidget = GTK_PIXMAP( gtk_pixmap_new (pm,mask) );

    //TODO: Der Separator in einem GtkDialog gehoert mit zur dessen vbox
    //(siehe gtkdialog.c, Fkt. gtk_dialog_init). Deshalb fuehrt folgender Befehl dazu, dass
    //nicht nur die Pixmap, sondern auch der Separator 5 Pixel vom Rand eingerueckt wird.
    //gtk_container_set_border_width (GTK_CONTAINER(aboutbox->vbox),5);
    //==> Ist das nun ein Bug oder ein Feature von GtkDialog?

    gtk_box_pack_start (GTK_BOX(aboutbox->vbox), GTK_WIDGET(pmwidget), FALSE, TRUE, 0);
    gtk_widget_show (GTK_WIDGET(pmwidget));

    GtkWidget* btn = gtk_button_new();
    connect_button_accelerator (GTK_BUTTON(btn),"_Close",accel_group);
    gtk_box_pack_start (GTK_BOX(aboutbox->action_area), btn, FALSE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT(btn), "clicked",
                        GTK_SIGNAL_FUNC(on_aboutbox_close_click), NULL);
    gtk_widget_show (btn);

    gtk_widget_add_accelerator
            (btn, "clicked",
             accel_group,
             GDK_Return,
             0,
             GTK_ACCEL_LOCKED);
    gtk_widget_add_accelerator
            (btn, "clicked",
             accel_group,
             GDK_Escape,
             0,
             GTK_ACCEL_LOCKED);

    gtk_signal_connect (GTK_OBJECT (aboutbox), "delete_event",
                        GTK_SIGNAL_FUNC (on_aboutbox_delete), NULL);

    gtk_widget_show (GTK_WIDGET(aboutbox));

    gtk_grab_add (GTK_WIDGET(aboutbox));
    gtk_main ();
    gtk_grab_remove (GTK_WIDGET(aboutbox));

    gtk_widget_destroy (GTK_WIDGET(aboutbox));
}

static void on_aboutbox_close_click ( GtkWidget*, gpointer )
{
    gtk_widget_hide (GTK_WIDGET(aboutbox));
    gtk_main_quit();
}

static gint on_aboutbox_delete ( GtkWidget*, GdkEvent, gpointer )
{
    gtk_main_quit();
}


static GtkLabel
     *m_lblScore, *m_lblLines, *m_lblLevel;

static void CreateStatusbar (GtkWidget** statusbar)
{
    *statusbar = gtk_hbox_new(FALSE,0);

    m_lblScore = GTK_LABEL( gtk_label_new ("") );
    m_lblLines = GTK_LABEL( gtk_label_new ("") );
    m_lblLevel = GTK_LABEL( gtk_label_new ("") );

    GdkFont* fnt = GTK_WIDGET(m_lblScore)->style->font;

    GtkWidget* frm = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type( GTK_FRAME(frm), GTK_SHADOW_IN);
    gtk_container_add (GTK_CONTAINER(frm),GTK_WIDGET(m_lblScore));
    gtk_box_pack_start (GTK_BOX(*statusbar),frm,FALSE,FALSE,0);
    gtk_widget_set_usize(frm,
                         gdk_string_measure(fnt,"Score: 0000000000000"+3),
                         20); //gdk_string_height(fnt,"Aq"+3)); //TODO: wie besser machen?
    gtk_widget_show(frm);

    frm = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type( GTK_FRAME(frm), GTK_SHADOW_IN);
    gtk_container_add (GTK_CONTAINER(frm),GTK_WIDGET(m_lblLines));
    gtk_box_pack_start (GTK_BOX(*statusbar),frm,FALSE,FALSE,0);
    gtk_widget_set_usize(frm,
                         gdk_string_measure(fnt,"Lines: 0000000000"+3),
                         20);
    gtk_widget_show(frm);

    frm = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type( GTK_FRAME(frm), GTK_SHADOW_IN);
    gtk_container_add (GTK_CONTAINER(frm),GTK_WIDGET(m_lblLevel));
    gtk_box_pack_end (GTK_BOX(*statusbar),frm,FALSE,FALSE,0);
    gtk_widget_set_usize(frm,
                         gdk_string_measure(fnt,"Level 000000"+3),
                         20);
    gtk_widget_show(frm);

    gtk_widget_show (GTK_WIDGET(m_lblLevel));
    gtk_widget_show (GTK_WIDGET(m_lblScore));
    gtk_widget_show (GTK_WIDGET(m_lblLines));
}


static void UpdateScoreDisplay()
{
    static int prevVal = -1;
    if (m_pGameProcess->GetScore() != prevVal)
    {
        char buf[20];
        sprintf (buf,"Score: %8i",m_pGameProcess->GetScore());
        gtk_label_set_text (m_lblScore,buf);
        prevVal = m_pGameProcess->GetScore();
    }
}

static void UpdateLinesDisplay()
{
    static int prevVal = -1;
    if (m_pGameProcess->GetLines() != prevVal)
    {
        char buf[20];
        sprintf (buf,"Lines: %6i",m_pGameProcess->GetLines());
        gtk_label_set_text (m_lblLines,buf);
        prevVal = m_pGameProcess->GetLines();
    }
}

static void UpdateLevelDisplay()
{
    static int prevVal = -1;
    if (m_Level != prevVal)
    {
        char buf[20];
        sprintf (buf,"Level %i",m_Level);
        gtk_label_set_text (m_lblLevel,buf);
        prevVal = m_Level;
    }
}


static gint OnTimeout (gpointer)
{
    if (!GamePaused)
    {
        gdk_threads_enter ();
        bool bGameEnded = ! m_pGameProcess->StepForth ();
        UpdateScoreDisplay();
        UpdateLinesDisplay();
        gdk_threads_leave ();

        if (bGameEnded)
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
    }
    return TRUE;
}
