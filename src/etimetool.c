#define _GNU_SOURCE 1

#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <libintl.h>
#include <linux/rtc.h>
#include <stdbool.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include <Evas.h>

static bool update_clock;

enum {
    E_YEAR = 0,
    E_MONTH,
    E_DAY,
    E_HOUR,
    E_MIN,
} positions;

static int rtc_open()
{
    int rtc = open("/dev/rtc", O_WRONLY);
    if(rtc)
        return rtc;
    rtc = open("/dev/rtc0", O_WRONLY);
    if(rtc)
        return rtc;
    rtc = open("/dev/misc/rtc", O_WRONLY);
    return rtc;
}

static void set_clock(int year, int month, int day, int h, int m)
{
    if(!update_clock)
    {
        fprintf(stderr, "set_clock(%d-%d-%d %d:%d) dry run\n",
                year, month, day, h, m);
        return;
    }

    struct tm tm = {
        .tm_sec = 0,
        .tm_min = m,
        .tm_hour = h,
        .tm_mday = day,
        .tm_mon = month,
        .tm_year = year
    };

    struct timeval tv = {
        .tv_sec = mktime(&tm),
        .tv_usec = 0
    };

    if(tv.tv_sec == -1)
        err(1, "failed to convert %d-%d-%d %d:%d time to Epoch-based timestamp",
            year, month, day, h, m);

    struct timezone tz = {
        .tz_minuteswest = 0,
        .tz_dsttime = 0
    };

    if(-1 == settimeofday(&tv, &tz))
        err(1, "settimeofday failed to set timestamp %ld", tv.tv_sec);

    int rtc = rtc_open();
    if(rtc == -1)
        err(1, "Unable to open RTC clock");

    if(ioctl(rtc, RTC_SET_TIME, &tm) == -1)
        err(1, "Unable to set RTC clock to date %d-%d-%d %d:%d",
            year, month, day, h, m);

    if(-1 == close(rtc))
        err(1, "Unable to close RTC clock descriptior");
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
    if(index == E_YEAR)
        _append("20");
    if(cursor == index)
        _append("<cursor>");
    if(cursor == index && half_mode)
        snprintf(s, 16, "%d ", half);
    else
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
    _append(" - ");
    _append_value(E_MONTH);
    _append(" - ");
    _append_value(E_DAY);
    _append(" ");
    printf("etimetool/date=\"%s\"\n", buf);
    edje_object_part_text_set(edje, "etimetool/date", buf);
    limit=1024;
    buf[0]='\0';
    _append_value(E_HOUR);
    _append(" : ");
    _append_value(E_MIN);
    _append(" ");
    printf("etimetool/time=\"%s\"\n", buf);
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
    values[E_MONTH] = loctime->tm_mon + 1;
    values[E_DAY] = loctime->tm_mday;
    values[E_HOUR] = loctime->tm_hour;
    values[E_MIN] = loctime->tm_min;
    values[E_YEAR] = loctime->tm_year - 100;
    if(values[E_YEAR] < 9)
        values[E_YEAR] = 0;
}

static void save_and_exit()
{
    set_clock(values[E_YEAR], values[E_MONTH], values[E_DAY],
              values[E_HOUR], values[E_MIN]);
    ecore_main_loop_quit();
}

static void main_win_key_handler(void* param __attribute__((unused)),
        Evas* e __attribute__((unused)),
        Evas_Object *r, void* event_info)
{
    Evas_Event_Key_Down* ev = (Evas_Event_Key_Down*)event_info;
    const char* k = ev->keyname;

    if(!strncmp(k, "KP_", 3) && (isdigit(k[3])) && !k[4])
    {
        int code = k[3] - '0';
        int res;
        printf("code %s %d\n", k, code);
        if(half_mode)
        {
            printf("In half mode: %d %d\n", half, code);
            res = half * 10 + code;
            if(res > limits_up[cursor] || res < limits_down[cursor] ||
                (res < 9 && cursor == E_YEAR))
                reset_input();
            else
            {
                values[cursor] = res;
                half_mode = 0;
                cursor_fwd();
            }
        }
        else
        {
            if(code * 10 > limits_up[cursor])
                return;
            rollback = values[cursor];
            half_mode = 1;
            half = code;
        }
        draw(r);
        return;

    }

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
    draw(r);
}

static void main_win_resize_handler(Ecore_Evas* main_win)
{
   ecore_evas_hide(main_win);
   int w, h;
   Evas* canvas = ecore_evas_get(main_win);
   evas_output_size_get(canvas, &w, &h);

   Evas_Object* edje = evas_object_name_find(canvas, "edje");
   evas_object_resize(edje, w, h);
   ecore_evas_show(main_win);
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
    evas_object_move(edje, 0, 0);
    evas_object_resize(edje, 600, 800);
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
    if(argc == 2 && !strcmp(argv[1], "--update-clock"))
        update_clock = true;

    if(!evas_init())
        err(1, "Unable to initialize Evas");
    if(!ecore_init())
        err(1, "Unable to initialize Ecore");
    if(!ecore_evas_init())
        err(1, "Unable to initialize Ecore_Evas");
    if(!edje_init())
        err(1, "Unable to initialize Edje");

    run();

    edje_shutdown();
    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();

	return 0;
}
