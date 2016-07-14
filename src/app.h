#ifndef APP_H_
#define APP_H_

extern int fbwidth, fbheight;
extern unsigned char *fbpixels;
extern unsigned long time_msec;

int app_init(int argc, char **argv);
void app_cleanup(void);

void app_draw(void);

void app_keyboard(int key, int state);

/* defined in main_*.c */
void app_quit(void);
void set_palette(int idx, int r, int g, int b);

#endif	/* APP_H_ */
