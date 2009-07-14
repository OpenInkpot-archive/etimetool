#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
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



void
hwclock(int year, int month, int day, int h, int m)
{
    char *s;
	asprintf(&s, "date %02d%02d%02d%02d20%02d; hwclock --systohc",
            month, day, h, m, year);
	if(s)
    {
    	//system(s);
        printf("system(\"%s\");\n",s);
	    free(s);
	}
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

