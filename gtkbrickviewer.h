#ifndef GTK_BRICK_VIEWER_H
#define GTK_BRICK_VIEWER_H

#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>


#ifdef __cplusplus
extern "C" {
#endif



#define GTK_BRICK_VIEWER(obj)          GTK_CHECK_CAST (obj, gtk_brick_viewer_get_type (), GtkBrickViewer)
#define GTK_BRICK_VIEWER_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, gtk_brick_viewer_get_type (), GtkBrickViewerClass)
#define GTK_IS_BRICK_VIEWER(obj)       GTK_CHECK_TYPE (obj, gtk_brick_viewer_get_type ())


typedef struct _GtkBrickViewer        GtkBrickViewer;
typedef struct _GtkBrickViewerClass   GtkBrickViewerClass;

struct _GtkBrickViewer
{
    GtkWidget widget;
    unsigned m_Cols, m_Rows, m_BrickSize;

    GdkColor** m_Contents;
};

struct _GtkBrickViewerClass
{
    GtkWidgetClass parent_class;
};


GtkWidget* gtk_brick_viewer_new (unsigned cols, unsigned rows, unsigned BrickSize);

guint gtk_brick_viewer_get_type ();

unsigned gtk_brick_viewer_GetRows (GtkBrickViewer* bv);
unsigned gtk_brick_viewer_GetCols (GtkBrickViewer* bv);
unsigned gtk_brick_viewer_GetBrickSize (GtkBrickViewer* bv);
void gtk_brick_viewer_SetBrickSize (GtkBrickViewer* bv, unsigned s);

GdkColor gtk_brick_viewer_GetBrickColor (GtkBrickViewer* bv, int col, int row);
void gtk_brick_viewer_SetBrickColor (GtkBrickViewer* bv, GdkColor color, int col, int row);

void gtk_brick_viewer_FillRect (GtkBrickViewer* bv, int col1, int row1, int col2, int row2, GdkColor color);
void gtk_brick_viewer_FillAll (GtkBrickViewer* bv, GdkColor color);

void gtk_brick_viewer_PasteRect (GtkBrickViewer* bv, GdkColor** rect, int width, int height, int colOrig, int rowOrig);


struct stone_shape    /* Struktur für die Form eines Tetris-Steins */
  {
    int no_points;
    GdkPoint		     /* Posit. der einzelnen Punkte relativ */
       relpos [4]; 	     /* zum 'Bezugspunkt' */
    enum {on_point,on_dr_cross} rot_point; /* Drehpunkt auf dem Bezugspunkt */
  };                                       /* oder auf dem Kreuz rechts darunter */

/* Kopf einer Schleife, die ueber alle Punkte einer Form iteriert */
#define FOR_EACH_SHAPE_POINT(shape, point)           \
    for (GdkPoint* point = (shape).relpos;              \
             point - (shape).relpos < (shape).no_points;  \
         point++)


void gtk_brick_viewer_PasteShape (GtkBrickViewer* bv, stone_shape& shape, int col, int row, GdkColor color);

GdkColor** gtk_brick_viewer_GetRect (GtkBrickViewer* bv, int colOrig, int rowOrig, int width, int height);
GdkColor** gtk_brick_viewer_GetContents (GtkBrickViewer* bv);
void gtk_brick_viewer_SetContents (GtkBrickViewer* bv, GdkColor** rect);

GdkColor** gtk_brick_viewer_AllocateRect (int width, int height);
void gtk_brick_viewer_FreeRect (GdkColor** rect, int height);


#ifdef __cplusplus
}
#endif


#endif
