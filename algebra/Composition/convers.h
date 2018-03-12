/*
#ifdef	XVIEW

#define	P_SET	xv_set
#define	P_GET	xv_get
#define	P_SHOW_ITEM	XV_SHOW
#define	WINDOW_SET	xv_set

#define	XV_xy(m,x,y)	XV_X, xv_col(m, x), XV_Y, xv_row(m, y)

#undef	mpr_static
#define	mpr_static(a,b,c,d,e)

#define	CREA_SRV_I_48(n)	\
xv_create(NULL,SERVER_IMAGE,XV_WIDTH,48,XV_HEIGHT,48,SERVER_IMAGE_BITS,n,\
                      SERVER_IMAGE_DEPTH,1,NULL)

#define	CREA_CURSORE(n,d,x,y,w,h)						\
			{						\
			    Display * display;  			\
			    Window drawable;   				\
			    display=XtDisplay(drawwin);    		\
			    drawable=XtWindow(drawwin);			\
			    Pixmap si_xx ;				\
			    si_xx = XCreatePixmap(display, drawable,w,h,1)	\
			    if( ! si_xx ) {				\
				fprintf(stderr,				\
				    "can'create cursor server image\n"); \
				exit(1);				\
			    }						\
			    n = XCreatePixmapCursor(display, si_xx,si_xx \
					,&XBlack,&XWhite,x,y); \
			    if( ! n ) {					\
				fprintf(stderr,				\
				    "can'create cursor\n");		\
				exit(1);				\
			    }						\
			}

#else

#define	P_SET	panel_set
#define	P_GET	panel_get
#define	P_SHOW_ITEM	PANEL_SHOW_ITEM
#define	WINDOW_SET	window_set

#endif

*/
