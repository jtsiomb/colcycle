/*
colcycle - color cycling image viewer
Copyright (C) 2016  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
unsigned long get_msec(void);
void set_palette(int idx, int r, int g, int b);

#endif	/* APP_H_ */
