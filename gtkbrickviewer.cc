#include "gtkbrickviewer.h"
#include "types.h"


static GtkWidgetClass* parent_class = (GtkWidgetClass*)NULL;

static void gtk_brick_viewer_class_init (GtkBrickViewerClass *klass);
static void gtk_brick_viewer_init (GtkBrickViewer* bv);
static void gtk_brick_viewer_destroy (GtkObject *object);

static void gtk_brick_viewer_realize (GtkWidget* widget);
static void gtk_brick_viewer_size_request (GtkWidget* widget,GtkRequisition *requisition);
static void gtk_brick_viewer_size_allocate (GtkWidget* widget,GtkAllocation *allocation);
static gint gtk_brick_viewer_expose (GtkWidget      *widget, GdkEventExpose *event);



guint gtk_brick_viewer_get_type ()
{
    static guint type = 0;

    if (!type)
    {
        GtkTypeInfo ti =
        {
            "GtkBrickViewer",
            sizeof (GtkBrickViewer),
            sizeof (GtkBrickViewerClass),
            (GtkClassInitFunc) gtk_brick_viewer_class_init,
            (GtkObjectInitFunc) gtk_brick_viewer_init,
            (GtkArgSetFunc) NULL,
            (GtkArgGetFunc) NULL,
        };

        type = gtk_type_unique (gtk_widget_get_type (), &ti);
    }

    return type;
}


static void gtk_brick_viewer_class_init (GtkBrickViewerClass *klass)
{
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;

    object_class = (GtkObjectClass*) klass;
    widget_class = (GtkWidgetClass*) klass;

    parent_class = (GtkWidgetClass*) gtk_type_class(gtk_widget_get_type());

    object_class->destroy = gtk_brick_viewer_destroy;

    widget_class->realize = gtk_brick_viewer_realize;
    widget_class->expose_event = gtk_brick_viewer_expose;
    widget_class->size_request = gtk_brick_viewer_size_request;
    widget_class->size_allocate = gtk_brick_viewer_size_allocate;
}


static void gtk_brick_viewer_init (GtkBrickViewer* bv)
{
}


GtkWidget* gtk_brick_viewer_new (int cols, int rows, int BrickSize)
{
    GtkBrickViewer* bv = (GtkBrickViewer*) gtk_type_new(gtk_brick_viewer_get_type ());

    bv->m_Cols = cols;
    bv->m_Rows = rows;
    bv->m_BrickSize = BrickSize;

    bv->m_Contents = new GdkColor* [bv->m_Cols];
    GdkColor clBlack = RGB(0,0,0);
    for (int col = 0; col < bv->m_Cols; col++)
    {
        bv->m_Contents[col] = new GdkColor[bv->m_Rows];
        for (int row = 0; row < bv->m_Rows; row++)
        {
            bv->m_Contents[col][row] = clBlack;
        }
    }

    return GTK_WIDGET (bv);
}


static void gtk_brick_viewer_destroy (GtkObject *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (GTK_IS_BRICK_VIEWER (object));

    GtkBrickViewer* bv = GTK_BRICK_VIEWER (object);

    for (int col = 0; col < bv->m_Cols; col++)
    {
        delete[] bv->m_Contents[col];
    }
    delete[] bv->m_Contents;
}



int gtk_brick_viewer_GetRows (GtkBrickViewer* bv)
{ return bv->m_Rows; }

int gtk_brick_viewer_GetCols (GtkBrickViewer* bv)
{ return bv->m_Cols; }

int gtk_brick_viewer_GetBrickSize (GtkBrickViewer* bv)
{ return bv->m_BrickSize; }

GdkColor gtk_brick_viewer_GetBrickColor (GtkBrickViewer* bv, int col, int row)
{ return bv->m_Contents[col][row]; }


//TODO: asserts einbauen



GdkColor** gtk_brick_viewer_AllocateRect (int width, int height)
{
    GdkColor** result = new GdkColor* [height];

    for (int row = 0; row < height; row++)
        result[row] = new GdkColor [width];

    return result;
}


void gtk_brick_viewer_FreeRect (GdkColor** rect, int height)
{
    for (int row = 0; row < height; row++)
        delete[] rect[row];
    delete[] rect;
}


void gtk_brick_viewer_SetBrickSize (GtkBrickViewer* bv, int s)
{
    bv->m_BrickSize = s;

    GtkWidget* widget = GTK_WIDGET(bv);

    if (GTK_WIDGET_REALIZED (widget))
    {
        gdk_window_resize (widget->window, s * bv->m_Cols, s * bv->m_Rows); 
//        Invalidate ();
    }
}


void gtk_brick_viewer_SetBrickColor (GtkBrickViewer* bv, GdkColor color, int col, int row)
{
    bv->m_Contents[col][row] = color;
    gtk_brick_viewer_InvalidateBrickArea (bv,col,row,col,row);
}

void gtk_brick_viewer_FillRect (GtkBrickViewer* bv, int col1, int row1, int col2, int row2, GdkColor color)
{
    for (int col = col1; col <= col2; col++)
        for (int row = row1; row <= row2; row++)
        {
            bv->m_Contents[col][row] = color;
        }
    
    gtk_brick_viewer_InvalidateBrickArea (bv,col1,row1,col2,row2);
}

void gtk_brick_viewer_FillAll (GtkBrickViewer* bv, GdkColor color)
{
    gtk_brick_viewer_FillRect (bv, 0, 0, bv->m_Cols - 1, bv->m_Rows - 1, color);
}

void gtk_brick_viewer_PasteShape (GtkBrickViewer* bv, stone_shape& shape, int col, int row, GdkColor color)
{
    int minCol=100000, minRow=100000,
        maxCol=-100000, maxRow=-100000;
    for (GdkPoint* p = shape.relpos;
         p - shape.relpos < shape.no_points;
         p++)
    {
        int x = col + p->x, y = row + p->y;
        if (x < minCol) minCol = x;
        if (x > maxCol) maxCol = x;
        if (y < minRow) minRow = y;
        if (y > maxRow) maxRow = y;
        bv->m_Contents[x][y] = color;
    }

    gtk_brick_viewer_InvalidateBrickArea (bv, minCol,minRow,maxCol,maxRow);
}


void gtk_brick_viewer_PasteRect (GtkBrickViewer* bv, GdkColor** rect, int width, int height,
                              int colOrig, int rowOrig)
{
    for (int row = 0; row < height; row++)
        for (int col = 0; col < width; col++)
            bv->m_Contents[colOrig + col][rowOrig + row] = rect[row][col];

    gtk_widget_draw (GTK_WIDGET(bv),(GdkRectangle*)NULL);
}


GdkColor** gtk_brick_viewer_GetRect (GtkBrickViewer* bv, int colOrig, int rowOrig, int width, int height)
{
    GdkColor** result = gtk_brick_viewer_AllocateRect (width,height);

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
            result[row][col] = bv->m_Contents[colOrig+col][rowOrig+row];
    }

    return result;
}


GdkColor** gtk_brick_viewer_GetContents (GtkBrickViewer* bv)
{
    return gtk_brick_viewer_GetRect (bv,0,0,bv->m_Cols,bv->m_Rows);
}


void gtk_brick_viewer_SetContents (GtkBrickViewer* bv, GdkColor** rect)
{
    gtk_brick_viewer_PasteRect (bv,rect,bv->m_Cols,bv->m_Rows,0,0);
}



static void gtk_brick_viewer_realize (GtkWidget *widget)
{
    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_BRICK_VIEWER (widget));

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    GtkBrickViewer* bv = GTK_BRICK_VIEWER (widget);

    GdkWindowAttr attributes;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);

    gint attributes_mask;
    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gdk_window_new (widget->parent->window, &attributes, attributes_mask);

    widget->style = gtk_style_attach (widget->style, widget->window);

    gdk_window_set_user_data (widget->window, widget);

    gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);
}


static void gtk_brick_viewer_size_request (GtkWidget      *widget,
                       GtkRequisition *requisition)
{
    GtkBrickViewer* bv = GTK_BRICK_VIEWER(widget);
    requisition->width  = bv->m_BrickSize * bv->m_Cols;
    requisition->height = bv->m_BrickSize * bv->m_Rows;
}




static void gtk_brick_viewer_size_allocate (GtkWidget     *widget,
                        GtkAllocation *allocation)
{
    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_BRICK_VIEWER (widget));
    g_return_if_fail (allocation != NULL);

    widget->allocation = *allocation;
    if (GTK_WIDGET_REALIZED (widget))
    {
        GtkBrickViewer* bv = GTK_BRICK_VIEWER(widget);

        gdk_window_move_resize (widget->window,
                                allocation->x, allocation->y,
                                allocation->width, allocation->height);
    }
}


static gint gtk_brick_viewer_expose (GtkWidget* widget, GdkEventExpose* event)
{
    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_BRICK_VIEWER (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

    if (event->count > 0)
        return FALSE;

    GtkBrickViewer* bv = GTK_BRICK_VIEWER(widget);

    /*    gdk_window_clear_area (widget->window,
                           0, 0,
                           widget->allocation.width,
                           widget->allocation.height);
    */

    GdkRectangle& rect = event->area;

    int m_BrickSize = bv->m_BrickSize;

    int minCol = rect.x / m_BrickSize, minRow = rect.y / m_BrickSize,
        maxCol = (rect.x+rect.width) / m_BrickSize, maxRow = (rect.y+rect.height) / m_BrickSize;
    
    if (maxCol >= bv->m_Cols) maxCol = bv->m_Cols-1;
    if (maxRow >= bv->m_Rows) maxRow = bv->m_Rows-1;

    for (int col = minCol; col <= maxCol; col++)
        for (int row = minRow; row <= maxRow; row++)
        {
            GdkColor cl = bv->m_Contents[col][row];
            bool isBlack = (cl.red==0 && cl.green==0 && cl.blue==0);
            gdk_draw_rectangle (widget->window,
                                isBlack? widget->style->bg_gc[widget->state] : widget->style->fg_gc[widget->state],
                                TRUE,
                                col * m_BrickSize, row * m_BrickSize,
                                m_BrickSize, m_BrickSize);
        }

    return FALSE;
}


void gtk_brick_viewer_InvalidateBrickArea (GtkBrickViewer* bv, int col1, int row1, int col2, int row2)
{
    GdkRectangle r;
    r.x = bv->m_BrickSize * col1;
    r.y = bv->m_BrickSize * row1;
    r.width = bv->m_BrickSize * (col2-col1+1);
    r.height = bv->m_BrickSize * (row2-row1+1);

    gtk_widget_draw (GTK_WIDGET(bv),&r);
}


/*
int gtk_brick_viewer_OnCreate(GtkBrickViewer* bv, LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

    SetBrickSize (m_BrickSize);

	return 0;
}
*/
