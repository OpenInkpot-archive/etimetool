#define _GNU_SOURCE 1
#include <locale.h>
#include <libintl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>


static void die(const char* fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);
   vfprintf(stderr, fmt, ap);
   va_end(ap);
   exit(EXIT_FAILURE);
}

enum {
    E_YEAR = 0,
    E_MONTH,
    E_DAY,
    E_HOUR,
    E_MIN,
    } positions;

void
hwclock(int year, int month, int day, int h, int m)
{
    char *s;
	asprintf(&s, "date %02d%02d%02d%02d20%02d; hwclock --systohc",
            month, day, h, m, year);
	if(s)
    {
    	system(s);
        //printf("system(\"%s\");\n",s);
	    free(s);
	}
}

int values[5];
int limits_up[5] = { 99, 12, 31, 23, 59 };
int limits_down[5] = { 0, 1, 1, 0, 0 };

static char buf[1024];
static int limit = 1024;
int cursor = 0;

int half = 0;
int half_mode = 0;
int rollback = 0;

static void
_append(const char *s)
{
    limit-=strlen(s);
    if(limit > 0) // Just protect from segfault, do nothing if string not fit.
        strncat(buf, s, limit);
}

static void
_append_value(int index)
{
    char s[16];
    if(cursor == index)
        _append("<cursor>");
    if(index == E_YEAR)
        _append("20");
    snprintf(s, 16, "%02d", values[index]);
    _append(s);
    if(cursor == index)
        _append("</cursor>");
}



static void
cursor_fwd() {
    if(cursor == E_MIN)
        cursor = E_YEAR;
    else
        cursor++;
}

static void
cursor_back() {
    if(cursor == E_YEAR)
        cursor = E_MIN;
    else
        cursor--;
}

static void
draw(Evas_Object *edje)
{
    limit=1024;
    buf[0]='\0';
    _append_value(E_YEAR);
    _append(" / ");
    _append_value(E_MONTH);
    _append(" / ");
    _append_value(E_DAY);
    edje_object_part_text_set(edje, "etimetool/date", buf);
    limit=1024;
    buf[0]='\0';
    _append_value(E_HOUR);
    _append(" : ");
    _append_value(E_MIN);
    edje_object_part_text_set(edje, "etimetool/time", buf);
}

static void
reset_input()
{
    half_mode = 0;
    values[cursor] = rollback;
}

static void
increment() {
    values[cursor]++;
    if(values[cursor] > limits_up[cursor])
        values[cursor] = limits_down[cursor];
}

static void
decrement() {
    values[cursor]--;
    if(values[cursor] < limits_down[cursor])
        values[cursor] = limits_up[cursor];
}

static void
prepare()
{
    time_t curtime;
    struct tm *loctime;
    curtime = time (NULL);
    loctime = localtime (&curtime);
    values[E_MONTH] = loctime->tm_mon;
    values[E_DAY] = loctime->tm_mday;
    values[E_HOUR] = loctime->tm_hour;
    values[E_MIN] = loctime->tm_min;
    values[E_YEAR] = loctime->tm_year - 100;
    if(values[E_YEAR] < 9)
        values[E_YEAR] = 0;
}

static void
save_and_exit()
{
    printf("save and exit\n");
}

static void main_win_key_handler(void* param __attribute__((unused)),
        Evas* e __attribute__((unused)),
        Evas_Object *r, void* event_info)
{
    Evas_Event_Key_Down* ev = (Evas_Event_Key_Down*)event_info;
    const char* k = ev->keyname;

    if(!strcmp(k, "Escape"))
        ecore_main_loop_quit();

    if(half_mode)
    {
        reset_input();
        draw(r);
        return;
    }

    else if(!strcmp(k, "Return") || !strcmp(k, "KP_Return"))
        save_and_exit();
    else if(!strcmp(k, "Up") || !strcmp(k, "Prior"))
        increment();
    else if(!strcmp(k, "Down") || !strcmp(k, "Next"))
        decrement();
    else if(!strcmp(k, "Left"))
        cursor_back();
    else if(!strcmp(k, "Right"))
        cursor_fwd();
#if 0
    else if(!strcmp(k, "KP_8"))
        increment();
    else if(!strcmp(k, "KP_2"))
        decrement();
    else if(!strcmp(k, "KP_4"))
        cursor_back();
    else if(!strcmp(k, "KP_6"))
        cursor_fwd();
#endif
    draw(r);
}

static void main_win_resize_handler(Ecore_Evas* main_win)
{
   int w, h;
   Evas* canvas = ecore_evas_get(main_win);
   evas_output_size_get(canvas, &w, &h);

   Evas_Object* edje = evas_object_name_find(canvas, "edje");
   evas_object_resize(edje, w, h);
}

void exit_all(void* param) { ecore_main_loop_quit(); }

static void main_win_close_handler(Ecore_Evas* main_win)
{
    ecore_main_loop_quit();
}

static int exit_handler(void* param, int ev_type, void* event)
{
    ecore_main_loop_quit();
    return 1;
}

static void
run()
{
    ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_handler, NULL);
    Ecore_Evas* main_win = ecore_evas_software_x11_new(0, 0, 0, 0, 600, 800);
    ecore_evas_title_set(main_win, "Date/time settings");
    ecore_evas_name_class_set(main_win, "etimetool", "etimetool");
    Evas* main_canvas = ecore_evas_get(main_win);
    ecore_evas_callback_delete_request_set(main_win, main_win_close_handler);
    ecore_evas_callback_resize_set(main_win, main_win_resize_handler);
    Evas_Object *edje = edje_object_add(main_canvas);
    evas_object_name_set(edje, "edje");
    edje_object_file_set(edje, DATADIR "/etimetool/themes/etimetool.edj", "etimetool");
    evas_object_event_callback_add(edje, EVAS_CALLBACK_KEY_UP,
            &main_win_key_handler, edje);
    evas_object_focus_set(edje, 1);
    edje_object_part_text_set(edje, "etimetool/help",
        gettext("\"C\" - Cancel \"OK\" - Apply"));
    prepare();
    draw(edje);
    evas_object_show(edje);
    ecore_evas_show(main_win);
    ecore_main_loop_begin();
}

int main(int argc, char **argv)
{
   if(!evas_init())
      die("Unable to initialize Evas\n");
   if(!ecore_init())
      die("Unable to initialize Ecore\n");
   if(!ecore_evas_init())
      die("Unable to initialize Ecore_Evas\n");
   if(!edje_init())
      die("Unable to initialize Edje\n");

   run();

   edje_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   evas_shutdown();

	return 0;
}

