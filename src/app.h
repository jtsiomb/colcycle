#ifndef APP_H_
#define APP_H_

int fbwidth, fbheight;
unsigned char *fbpixels;
unsigned long time_msec;

int app_init(int argc, char **argv);
void app_cleanup(void);

void app_draw(void);

void app_keyboard(int key, int state);

/* defined in main_*.c */
void app_quit(void);
void set_palentry(int idx, unsigned char r, unsigned char g, unsigned char b);

#endif	/* APP_H_ */
