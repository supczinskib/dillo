/*
 * File: dialog.cc
 *
 * Copyright (C) 2005-2007 Jorge Arellano Cid <jcid@dillo.org>
 * Copyright (C) 2026 Rodrigo Arias Mallo <rodarima@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

/** @file
 * UI dialogs
 */

#include <math.h> // for rint()
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#define private public
#include <FL/Fl_File_Chooser.H>
#undef private
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_draw.H>

#include "msg.h"
#include "dialog.hh"
#include "misc.h"
#include "prefs.h"
#include "uistyle.hh"
#include "dlib/dlib.h"

/*
 * Local Data
 */

/* Target device visible work area: 480x320 screen with 24 px used by
 * the system bar. Leave additional room for the window-manager titlebar
 * so the FLTK file chooser titlebar stays visible on the LCD.
 */
#define DILLO_FILE_CHOOSER_MAX_W 480
#define DILLO_FILE_CHOOSER_WORK_H 296
#define DILLO_FILE_CHOOSER_TITLEBAR_ROOM 30
#define DILLO_FILE_CHOOSER_MAX_H \
   (DILLO_FILE_CHOOSER_WORK_H - DILLO_FILE_CHOOSER_TITLEBAR_ROOM)

static int input_answer;
static char *input_str = NULL;
static int choice_answer;

/* Dillo globally replaces FL_NORMAL_LABEL with a label drawer that disables
 * '@' symbol interpretation. Fl_File_Chooser's preview pane relies on the
 * symbolic label "@fileopen" for directory preview, so give this one widget
 * a local label type that keeps symbols enabled. Do not use it for text
 * previews because file contents may legitimately contain '@'.
 */
#define DILLO_SYMBOL_LABEL ((Fl_Labeltype)(FL_FREE_LABELTYPE + 1))

static void Dialog_symbol_label_draw(const Fl_Label *o, int X, int Y,
                                     int W, int H, Fl_Align align)
{
   fl_draw_shortcut = 0;
   fl_font(o->font, o->size);
   fl_color((Fl_Color)o->color);
   fl_draw(o->value, X, Y, W, H, align, o->image, 1);
}

static void Dialog_symbol_label_measure(const Fl_Label *o, int &W, int &H)
{
   fl_draw_shortcut = 0;
   fl_font(o->font, o->size);
   fl_measure(o->value, W, H, 1);
}

static void Dialog_init_symbol_label_type()
{
   static int initialized = 0;
   if (!initialized) {
      Fl::set_labeltype(DILLO_SYMBOL_LABEL,
                        Dialog_symbol_label_draw,
                        Dialog_symbol_label_measure);
      initialized = 1;
   }
}

struct DialogNewDirData {
   Fl_Window *window;
   Fl_Input *input;
   int answer;
};

static void Dialog_new_dir_ok_cb(Fl_Widget *, void *data)
{
   DialogNewDirData *d = (DialogNewDirData *)data;
   d->answer = 1;
   d->window->hide();
}

static void Dialog_new_dir_cancel_cb(Fl_Widget *, void *data)
{
   DialogNewDirData *d = (DialogNewDirData *)data;
   d->answer = 0;
   d->window->hide();
}


/*
 * Local sub classes
 */

//----------------------------------------------------------------------------
/**
 * Used to enable CTRL+{a,e,d,k} in search dialog (for start,end,del,cut).
 * TODO: bind down arrow to a search engine selection list.
 */
class CustInput3 : public Fl_Input {
public:
   CustInput3 (int x, int y, int w, int h, const char* l=0) :
      Fl_Input(x,y,w,h,l) {};
   int handle(int e);
   int d_position();
   void d_position(int p);
};

/* FLTK 1.4 deprecated "position()" for "insert_position()", so we make
 * a backward compatible wrapper. */
int CustInput3::d_position()
{
#if FL_API_VERSION < 10400
   return CustInput3::position();
#else
   return CustInput3::insert_position();
#endif
}

void CustInput3::d_position(int p)
{
#if FL_API_VERSION < 10400
   CustInput3::position(p);
#else
   CustInput3::insert_position(p);
#endif
}

int CustInput3::handle(int e)
{
   int k = Fl::event_key();

   _MSG("CustInput3::handle event=%d\n", e);

   // We're only interested in some flags
   unsigned modifier = Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT);

   if (e == FL_KEYBOARD && modifier == FL_CTRL) {
      if (k == 'a' || k == 'e') {
         d_position(k == 'a' ? 0 : size());
         return 1;
      } else if (k == 'k') {
         cut(d_position(), size());
         return 1;
      } else if (k == 'd') {
         cut(d_position(), d_position()+1);
         return 1;
      }
   }
   return Fl_Input::handle(e);
}

/**
 * Used to make the ENTER key activate the CustChoice
 */
class CustChoice2 : public Fl_Choice {
public:
   CustChoice2 (int x, int y, int w, int h, const char* l=0) :
      Fl_Choice(x,y,w,h,l) {};
   int handle(int e) {
      if (e == FL_KEYBOARD &&
          (Fl::event_key() == FL_Enter || Fl::event_key() == FL_Down) &&
          (Fl::event_state() & (FL_SHIFT|FL_CTRL|FL_ALT|FL_META)) == 0) {
         return Fl_Choice::handle(FL_PUSH);
      }
      return Fl_Choice::handle(e);
   };
};

class EnterButton : public Fl_Button {
public:
   EnterButton (int x,int y,int w,int h, const char* label = 0) :
      Fl_Button (x,y,w,h,label) {};
   int handle(int e);
};

int EnterButton::handle(int e)
{
   if (e == FL_KEYBOARD && Fl::focus() == this && Fl::event_key() == FL_Enter){
      set_changed();
      simulate_key_action();
      do_callback();
      return 1;
   }
   return Fl_Button::handle(e);
}

//----------------------------------------------------------------------------


/**
 * Display a message in a popup window.
 */
static void msg_close_cb(Fl_Widget *button, void *)
{
   button->window()->hide();
}

void a_Dialog_msg(const char *title, const char *msg)
{
   int ww = 410, wh = 105, gap = 10, ih = 58, bw = 88, bh = 28;

   if (!(title && *title))
      title = "Dillo: Message";

   Fl_Window *window = new Fl_Window(ww, wh, title);
   window->set_modal();
   window->begin();

    Fl_Box *icon = new Fl_Box(gap, gap, ih, ih);
    icon->box(FL_THIN_UP_BOX);
    icon->color(FL_WHITE);
    icon->labelfont(FL_TIMES_BOLD);
    icon->labelsize(34);
    icon->labelcolor(FL_BLUE);
    icon->label("i");

    Fl_Box *box = new Fl_Box(ih + 2*gap, gap,
                             ww - ih - 3*gap, ih, msg ? msg : "");
    box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP | FL_ALIGN_CLIP);
    a_UI_apply_label_font(box);

    Fl_Return_Button *b = new Fl_Return_Button(ww - gap - bw, wh - gap - bh,
                                               bw, bh, "Close");
    b->align(FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    b->box(FL_UP_BOX);
    a_UI_apply_label_font(b);
    b->callback(msg_close_cb, NULL);

   window->end();
   window->show();
   while (window->shown())
      Fl::wait();
   delete window;
}


/**
 * Callback for a_Dialog_input()
 */
static void input_cb(Fl_Widget *button, void *number)
{
  input_answer = VOIDP2INT(number);
  button->window()->hide();
}

/**
 * Dialog for one line of Input with a message.
 * avoids the sound bell in fl_input(), and allows customization
 *
 * @return string on success, NULL upon Cancel or Close window
 */
const char *a_Dialog_input(const char *title, const char *msg)
{
   static Fl_Menu_Item *pm = 0;
   int ww = 450, wh = 130, gap = 10, ih = 60, bw = 80, bh = 30;

   input_answer = 0;

   if (!(title && *title))
      title = "Dillo: Input";

   Fl_Window *window = new Fl_Window(ww,wh,title);
   window->set_modal();
   window->begin();
    Fl_Group* ib = new Fl_Group(0,0,window->w(),window->h());
    ib->begin();
    window->resizable(ib);

    /* '?' Icon */
    Fl_Box* o = new Fl_Box(gap, gap, ih, ih);
    o->box(FL_THIN_UP_BOX);
    o->labelfont(FL_TIMES_BOLD);
    o->labelsize(34);
    o->label("?");
    o->show();

    Fl_Box *box = new Fl_Box(ih+2*gap,gap,ww-(ih+3*gap),ih/2, msg);
    box->labelfont(FL_HELVETICA);
    box->labelsize(14);
    a_UI_apply_label_font(box);
    box->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_CLIP|FL_ALIGN_WRAP);

    CustInput3 *c_inp = new CustInput3(ih+2*gap,gap+ih/2+gap,ww-(ih+3*gap),24);
    c_inp->labelsize(14);
    c_inp->textsize(14);
    a_UI_apply_input_font(c_inp);

    CustChoice2 *ch = new CustChoice2(1*gap,ih+3*gap,180,24);
    if (!pm) {
       int n_it = dList_length(prefs.search_urls);
       pm = (Fl_Menu_Item *) calloc(n_it+1, sizeof(Fl_Menu_Item));
       if (pm == NULL) {
          MSG("calloc failed: %s\n", strerror(errno));
          exit(1);
       }
       for (int i = 0, j = 0; i < n_it; i++) {
          char *label, *url, *source;
          source = (char *)dList_nth_data(prefs.search_urls, i);
          if (!source || a_Misc_parse_search_url(source, &label, &url) < 0)
             continue;
          pm[j++].label(FL_NORMAL_LABEL, dStrdup(label));
       }
    }
    ch->tooltip("Select search engine");
    a_UI_apply_menu_font(pm);
    ch->menu(pm);
    ch->value(prefs.search_url_idx);

    int xpos = ww-2*(gap+bw), ypos = ih+3*gap;
    Fl_Return_Button *rb = new Fl_Return_Button(xpos, ypos, bw, bh, "OK");
    a_UI_apply_label_font(rb);
    rb->align(FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
    rb->box(FL_UP_BOX);
    rb->callback(input_cb, INT2VOIDP(1));

    xpos = ww-(gap+bw);
    Fl_Button *b = new Fl_Button(xpos, ypos, bw, bh, "Cancel");
    a_UI_apply_label_font(b);
    b->align(FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
    b->box(FL_UP_BOX);
    b->callback(input_cb, INT2VOIDP(2));

   window->end();

   window->show();
   while (window->shown())
      Fl::wait();
   if (input_answer == 1) {
      /* we have a string, save it */
      dFree(input_str);
      input_str = dStrdup(c_inp->value());
      prefs.search_url_idx = ch->value();
   }
   delete window;

   return (input_answer == 1) ? input_str : NULL;
}

/**
 * Dialog for password
 */
const char *a_Dialog_passwd(const char *title, const char *msg)
{
   if (!(title && *title))
      title = "Dillo: Password";
   fl_message_title(title);
   return fl_password("%s", "", msg);
}

static void Dialog_center_window_on_screen(Fl_Window *w)
{
   if (!w)
      return;

   int x = (Fl::w() - w->w()) / 2;
   int y = (Fl::h() - w->h()) / 2;

   /* IceWM's titlebar is outside the FLTK client area.  With a small LCD,
    * centering the client rectangle alone makes the decorated window look
    * too high by roughly half of the titlebar.
    */
   y += DILLO_FILE_CHOOSER_TITLEBAR_ROOM / 2;

   if (x < 0) x = 0;
   if (y < 0) y = 0;
   w->position(x, y);
}

static void Dialog_fit_file_window(Fl_Window *w)
{
   if (!w)
      return;

   int max_w = Fl::w();
   int max_h = Fl::h();

   if (max_w <= 0 || max_w > DILLO_FILE_CHOOSER_MAX_W)
      max_w = DILLO_FILE_CHOOSER_MAX_W;
   if (max_h <= 0 || max_h > DILLO_FILE_CHOOSER_MAX_H)
      max_h = DILLO_FILE_CHOOSER_MAX_H;

   int new_w = w->w();
   int new_h = w->h();

   if (new_w > max_w) new_w = max_w;
   if (new_h > max_h) new_h = max_h;

   if (new_w != w->w() || new_h != w->h())
      w->size(new_w, new_h);

   Dialog_center_window_on_screen(w);

   if (w->x() < 0 || w->y() < 0) {
      int x = w->x() < 0 ? 0 : w->x();
      int y = w->y() < 0 ? 0 : w->y();
      w->position(x, y);
   }

   w->redraw();
}

static void Dialog_apply_file_chooser_label_font(Fl_Widget *w, Fl_Widget *skip)
{
   if (!w || w == skip)
      return;

   a_UI_apply_label_font(w);

   if (Fl_Group *group = w->as_group()) {
      for (int i = 0; i < group->children(); ++i)
         Dialog_apply_file_chooser_label_font(group->child(i), skip);
   }
}

static void Dialog_fix_file_chooser_preview(Fl_File_Chooser *fc);

static void Dialog_center_window_on_parent(Fl_Window *w, Fl_Window *parent)
{
   if (!w)
      return;

   if (parent) {
      int x = parent->x() + (parent->w() - w->w()) / 2;
      int y = parent->y() + (parent->h() - w->h()) / 2;
      if (x < 0) x = 0;
      if (y < 0) y = 0;
      w->position(x, y);
   } else {
      Dialog_center_window_on_screen(w);
   }
}

static const char *Dialog_new_directory_input(Fl_Window *parent)
{
   static char ret[FL_PATH_MAX];
   DialogNewDirData data;
   const int ww = 420;
   const int wh = 110;

   Fl_Window *win = new Fl_Window(ww, wh, Fl_File_Chooser::new_directory_tooltip);
   data.window = win;
   data.input = 0;
   data.answer = 0;

   Fl_Box *icon = new Fl_Box(10, 10, 50, 50, "?");
   icon->box(FL_THIN_UP_BOX);
   icon->labelfont(FL_TIMES_BOLD);
   icon->labelsize(34);
   icon->color(FL_WHITE);
   icon->labelcolor(FL_BLUE);

   Fl_Box *msg = new Fl_Box(70, 10, ww - 80, 25, Fl_File_Chooser::new_directory_label);
   msg->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
   a_UI_apply_label_font(msg);

   Fl_Input *input = new Fl_Input(70, 40, ww - 80, 25);
   a_UI_apply_input_font(input);
   data.input = input;

   Fl_Button *cancel = new Fl_Button(220, 76, 85, 24, fl_cancel);
   cancel->callback(Dialog_new_dir_cancel_cb, &data);
   a_UI_apply_label_font(cancel);

   Fl_Return_Button *ok = new Fl_Return_Button(315, 76, 85, 24, fl_ok);
   ok->callback(Dialog_new_dir_ok_cb, &data);
   a_UI_apply_label_font(ok);

   win->callback(Dialog_new_dir_cancel_cb, &data);
   win->set_modal();
   win->end();
   win->size_range(ww, wh, ww, wh);
   Dialog_center_window_on_parent(win, parent);
   input->take_focus();

   Fl_Window *grab = Fl::grab();
   if (grab)
      Fl::grab(0);

   win->show();
   while (win->shown())
      Fl::wait();

   if (grab)
      Fl::grab(grab);

   ret[0] = 0;
   if (data.answer && input->value() && input->value()[0])
      snprintf(ret, sizeof(ret), "%s", input->value());

   delete win;
   return ret[0] ? ret : NULL;
}

static void Dialog_file_chooser_newdir_cb(Fl_Widget *, void *data)
{
   Fl_File_Chooser *fc = (Fl_File_Chooser *)data;
   char pathname[FL_PATH_MAX + 4];
   const char *dir;

   if (!fc)
      return;

   dir = Dialog_new_directory_input(fc->window);
   if (!dir)
      return;

   if (dir[0] != '/')
      snprintf(pathname, sizeof(pathname), "%s/%s", fc->directory_, dir);
   else
      snprintf(pathname, sizeof(pathname), "%s", dir);

   if (mkdir(pathname, 0777) && errno != EEXIST) {
      a_Dialog_msg("Dillo", strerror(errno));
      return;
   }

   fc->directory(pathname);
   Dialog_fix_file_chooser_preview(fc);
}

static void Dialog_apply_file_chooser_style(Fl_File_Chooser *fc)
{
   if (!fc)
      return;

   if (prefs.ui_font && prefs.ui_font[0])
      fc->textfont(a_UI_font());
   if (prefs.ui_font_size > 0)
      fc->textsize(a_UI_font_size());

   /* These members are private in FLTK, but this file intentionally exposes
    * them with '#define private public' before including Fl_File_Chooser.H.
    * Styling the known widgets directly avoids RTTI/dynamic_cast, because
    * Dillo is compiled with -fno-rtti.
    */
   /* Do not apply a blind recursive label font to the whole chooser.
    * In particular, previewBox uses FLTK symbolic labels such as @fileopen;
    * changing that widget blindly can make the symbol appear as text.
    * Style only the known controls below.
    */

   Dialog_apply_file_chooser_label_font(fc->window, fc->previewBox);
   Dialog_apply_file_chooser_label_font(fc->favWindow, 0);

   a_UI_apply_menu_widget_font(fc->showChoice);
   a_UI_apply_menu_widget_font(fc->favoritesButton);
   a_UI_apply_browser_font(fc->fileList);
   a_UI_apply_browser_font(fc->favList);
   a_UI_apply_input_font(fc->fileName);

   Fl_Tooltip::font(a_UI_font(FL_HELVETICA));
   Fl_Tooltip::size(a_UI_font_size(14));

   a_UI_apply_label_font(fc->newButton);
   if (fc->newButton)
      fc->newButton->callback(Dialog_file_chooser_newdir_cb, fc);
   a_UI_apply_label_font(fc->previewButton);
   a_UI_apply_label_font(fc->showHiddenButton);
   a_UI_apply_label_font(fc->okButton);
   a_UI_apply_label_font(fc->cancelButton);
   a_UI_apply_label_font(fc->favUpButton);
   a_UI_apply_label_font(fc->favDeleteButton);
   a_UI_apply_label_font(fc->favDownButton);
   a_UI_apply_label_font(fc->favCancelButton);
   a_UI_apply_label_font(fc->favOkButton);
   a_UI_apply_label_font(fc->errorBox);
   if (fc->okButton)
      fc->ok_label(fc->ok_label());
}

static const char *Dialog_home_dir()
{
   const char *home = getenv("HOME");

   if (home && home[0])
      return home;

   return "/root";
}

static void Dialog_file_chooser_paths(const char *fname,
                                      char *start_dir, size_t start_dir_size,
                                      char *preset_file, size_t preset_size)
{
   const char *home = Dialog_home_dir();

   snprintf(start_dir, start_dir_size, "%s", home);
   preset_file[0] = 0;

   if (!(fname && fname[0]))
      return;

   if (fname[0] == '/') {
      const char *slash = strrchr(fname, '/');

      if (slash && slash != fname) {
         size_t len = slash - fname;
         if (len >= start_dir_size)
            len = start_dir_size - 1;
         memcpy(start_dir, fname, len);
         start_dir[len] = 0;
      } else {
         snprintf(start_dir, start_dir_size, "/");
      }

      snprintf(preset_file, preset_size, "%s", fname);
      return;
   }

   /* Save Page as File can pass just a suggested file name.  In that case
    * open the chooser in HOME and only pre-fill the file name there.
    */
   snprintf(preset_file, preset_size, "%s/%s", home, fname);
}

static void Dialog_fix_file_chooser_preview(Fl_File_Chooser *fc)
{
   if (!fc || !fc->previewBox)
      return;

   /* Textedit leaves Fl_File_Chooser's preview pane native. Do the same.
    * The only extra step needed in Dillo is for directories: Dillo globally
    * disables '@' symbol interpretation in FL_NORMAL_LABEL, so the native
    * "@fileopen" preview must use our symbol-capable label type. For all
    * other previews restore FL_NORMAL_LABEL and leave FLTK's own font/size
    * choices intact, including the small FL_COURIER text-file preview.
    */
   const char *label = fc->previewBox->label();

   if (label && strcmp(label, "@fileopen") == 0) {
      Dialog_init_symbol_label_type();
      fc->previewBox->labeltype(DILLO_SYMBOL_LABEL);
      fc->previewBox->image(0);
      fc->previewBox->align(FL_ALIGN_CLIP);
      fc->previewBox->labelfont(FL_HELVETICA);
      fc->previewBox->labelsize(75);
   } else {
      fc->previewBox->labeltype(FL_NORMAL_LABEL);
   }

   fc->previewBox->redraw();
}

static const char *Dialog_file_chooser(const char *title, const char *pattern,
                                       const char *fname, int type)
{
   static char retname[FL_PATH_MAX];
   char start_dir[FL_PATH_MAX];
   char preset_file[FL_PATH_MAX];

   Dialog_file_chooser_paths(fname, start_dir, sizeof(start_dir),
                             preset_file, sizeof(preset_file));

   Fl_File_Chooser *fc = new Fl_File_Chooser(start_dir, pattern, type, title);

   if (preset_file[0])
      fc->value(preset_file);

   Dialog_apply_file_chooser_style(fc);

   /* Force the preview pane on for Dillo.  Fl_File_Chooser stores the
    * preview checkbox in FLTK user preferences, so an earlier build/test
    * that disabled preview can persist and make the file list consume the
    * whole chooser width.  Dillo wants the right-side preview pane visible.
    */
   fc->preview(1);

   fc->rescan();
   Dialog_fix_file_chooser_preview(fc);
   fc->show();
   Dialog_fit_file_window(fc->window);

   Fl_Window *grab = Fl::grab();
   if (grab)
      Fl::grab(0);

   int main_last_w = fc->window ? fc->window->w() : -1;
   int main_last_h = fc->window ? fc->window->h() : -1;
   int fav_last_w = -1;
   int fav_last_h = -1;

   while (fc->shown()) {
      Fl::wait();

      /* FLTK can restore/resize the chooser after show().  Enforce the small
       * LCD geometry again, but only when the size actually changed.
       */
      if (fc->window &&
          (fc->window->w() != main_last_w || fc->window->h() != main_last_h)) {
         Dialog_fit_file_window(fc->window);
         main_last_w = fc->window->w();
         main_last_h = fc->window->h();
      }

      /* update_preview() can reset previewBox font/size after every file
       * selection.  Restyle only real text previews, never FLTK's symbolic
       * icon labels such as @fileopen.
       */
      Dialog_fix_file_chooser_preview(fc);

      /* The Manage Favorites dialog is created internally by
       * Fl_File_Chooser. Fit it only when it first appears or if FLTK
       * changes its size, to avoid unnecessary redraws on Xfbdev.
       */
      if (fc->favWindow && fc->favWindow->shown()) {
         Dialog_apply_file_chooser_style(fc);
         if (fc->favWindow->w() != fav_last_w ||
             fc->favWindow->h() != fav_last_h) {
            Dialog_fit_file_window(fc->favWindow);
            fav_last_w = fc->favWindow->w();
            fav_last_h = fc->favWindow->h();
         }
      } else {
         fav_last_w = -1;
         fav_last_h = -1;
      }
   }

   if (grab)
      Fl::grab(grab);

   const char *value = fc->value();
   if (value)
      snprintf(retname, sizeof(retname), "%s", value);
   delete fc;

   return value ? retname : NULL;
}

/**
 * Show the save file dialog.
 *
 * @return pointer to chosen filename, or NULL on Cancel.
 */
const char *a_Dialog_save_file(const char *title,
                               const char *pattern, const char *fname)
{
   return Dialog_file_chooser(title, pattern, fname, Fl_File_Chooser::CREATE);
}

/**
 * Show the select file dialog.
 *
 * @return pointer to chosen filename, or NULL on Cancel.
 */
const char *a_Dialog_select_file(const char *title,
                                 const char *pattern, const char *fname)
{
   /*
    * FileChooser::type(MULTI) appears to allow multiple files to be selected,
    * but just follow save_file's path for now.
    */
   return Dialog_file_chooser(title, pattern, fname, Fl_File_Chooser::CREATE);
}

/**
 * Show the open file dialog.
 *
 * @return pointer to chosen filename, or NULL on Cancel.
 */
char *a_Dialog_open_file(const char *title,
                         const char *pattern, const char *fname)
{
   const char *fc_name;

   fc_name = Dialog_file_chooser(title, pattern, fname, Fl_File_Chooser::CREATE);
   return (fc_name) ? a_Misc_escape_chars(fc_name, "% #") : NULL;
}

/**
 * Close text window.
 */
static void text_window_close_cb(Fl_Widget *, void *vtd)
{
   Fl_Text_Display *td = (Fl_Text_Display *)vtd;
   Fl_Text_Buffer *buf = td->buffer();

   delete (Fl_Window*)td->window();
   delete buf;
}

/**
 * Show a new window with the provided text
 */
void a_Dialog_text_window(const char *title, const char *txt)
{
   int wh = prefs.height, ww = prefs.width, bh = 30;

   if (!(title && *title))
      title = "Dillo: Text";

   Fl_Window *window = new Fl_Window(ww, wh, title);
   Fl_Group::current(0);


    Fl_Text_Buffer *buf = new Fl_Text_Buffer();
    buf->text(txt);
    Fl_Text_Display *td = new Fl_Text_Display(0,0,ww, wh-bh);
    td->buffer(buf);
    a_UI_apply_text_display_font(td, FL_HELVETICA,
                                 (Fl_Fontsize) rint(14.0 * prefs.font_factor));

    /* enable wrapping lines; text uses entire width of window */
    td->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
   window->add(td);

    Fl_Return_Button *b = new Fl_Return_Button (0, wh-bh, ww, bh, "Close");
    a_UI_apply_label_font(b);
    b->callback(text_window_close_cb, td);
   window->add(b);

   window->callback(text_window_close_cb, td);
   window->resizable(td);
   window->show();
}

/*--------------------------------------------------------------------------*/

static void choice_cb(Fl_Widget *button, void *number)
{
  choice_answer = VOIDP2INT(number);
  _MSG("choice_cb: %d\n", choice_answer);

  button->window()->hide();
}

/**
 * Make a question-dialog with a question and alternatives.
 * Last parameter must be NULL.
 *
 * @return 0 = dialog was cancelled, >0 = selected alternative.
 */
int a_Dialog_choice(const char *title, const char *msg, ...)
{
   va_list ap;
   int i, n;

   if (title == NULL || *title == '\0')
      title = "Dillo: Choice";

   va_start(ap, msg);
   for (n = 0; va_arg(ap, char *) != NULL; n++);
   va_end(ap);

   if (n == 0) {
      MSG_ERR("Dialog_choice: no alternatives.\n");
      return 0;
   }

   int gap = 8;
   int ww = 140 + n * 60, wh = 120;
   int bw = (ww - gap) / n - gap, bh = 45;

   Fl_Window *window = new Fl_Window(ww, wh, title);
   window->set_modal();
   window->begin();

    Fl_Text_Buffer *buf = new Fl_Text_Buffer();
    buf->text(msg);
    Fl_Text_Display *td = new Fl_Text_Display(0, 0, ww, wh - bh);
    td->buffer(buf);
    a_UI_apply_text_display_font(td, FL_HELVETICA,
                                 (Fl_Fontsize) rint(14.0 * prefs.font_factor));
    td->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
    
    window->resizable(td);

    int xpos = gap;
    va_start(ap, msg);
    for (i = 1; i <= n; i++) {
       Fl_Button *b = new EnterButton(xpos, wh-bh, bw, bh, va_arg(ap, char *));
       a_UI_apply_label_font(b);
       b->align(FL_ALIGN_WRAP | FL_ALIGN_CLIP);
       b->box(FL_UP_BOX);
       b->callback(choice_cb, INT2VOIDP(i));
       xpos += bw + gap;
       /* TODO: set focus to the *-prefixed alternative */
    }
    va_end(ap);
   window->end();

   choice_answer = 0;

   window->show();
   while (window->shown())
      Fl::wait();
   _MSG("Dialog_choice answer = %d\n", answer);
   td->buffer(NULL);
   delete buf;
   delete window;

   return choice_answer;
}

/*--------------------------------------------------------------------------*/
static void Dialog_user_password_cb(Fl_Widget *button, void *)
{
   button->window()->user_data(button);
   button->window()->hide();
}

/**
 * Make a user/password dialog.
 * Call the callback with the result (OK or not) and the given user and
 *   password if OK.
 */
int a_Dialog_user_password(const char *title, const char *msg,
                           UserPasswordCB cb, void *vp)
{
   int ok = 0, window_h = 280, y, msg_w, msg_h;
   const int window_w = 300, input_x = 80, input_w = 200, input_h = 30,
      button_h = 30;

   /* window is resized below */
   if (!(title && *title))
      title = "Dillo: User/Password";
   Fl_Window *window = new Fl_Window(window_w,window_h,title);
   Fl_Group::current(0);
   window->user_data(NULL);

   /* message */
   y = 20;
   msg_w = window_w - 40;
   Fl_Box *msg_box = new Fl_Box(20, y, msg_w, 100); /* resized below */
   msg_box->label(msg);
   msg_box->labelfont(FL_HELVETICA);
   msg_box->labelsize(14);
   a_UI_apply_label_font(msg_box);
   msg_box->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP_LEFT | FL_ALIGN_WRAP);

   fl_font(msg_box->labelfont(), msg_box->labelsize());
   msg_w -= 6; /* The label doesn't fill the entire box. */
   fl_measure(msg_box->label(), msg_w, msg_h, 0); // fl_measure wraps at msg_w
   msg_box->size(msg_box->w(), msg_h);
   window->add(msg_box);

   /* inputs */
   y += msg_h + 20;
   Fl_Input *user_input = new Fl_Input(input_x, y, input_w, input_h, "User");
   user_input->labelsize(14);
   user_input->textsize(14);
   a_UI_apply_input_font(user_input);
   window->add(user_input);
   y += input_h + 10;
   Fl_Secret_Input *password_input =
      new Fl_Secret_Input(input_x, y, input_w, input_h, "Password");
   password_input->labelsize(14);
   password_input->textsize(14);
   a_UI_apply_input_font(password_input);
   window->add(password_input);

   /* "OK" button */
   y += input_h + 20;
   Fl_Button *ok_button = new EnterButton(200, y, 50, button_h, "OK");
   ok_button->labelsize(14);
   a_UI_apply_label_font(ok_button);
   ok_button->callback(Dialog_user_password_cb);
   window->add(ok_button);

   /* "Cancel" button */
   Fl_Button *cancel_button =
      new EnterButton(50, y, 100, button_h, "Cancel");
   cancel_button->labelsize(14);
   a_UI_apply_label_font(cancel_button);
   cancel_button->callback(Dialog_user_password_cb);
   window->add(cancel_button);

   y += button_h + 20;
   window_h = y;
   window->size(window_w, window_h);
   window->size_range(window_w, window_h, window_w, window_h);
   window->resizable(window);

   window->show();
   while (window->shown())
      Fl::wait();

   ok = ((Fl_Widget *)window->user_data()) == ok_button ? 1 : 0;

   if (ok) {
      /* call the callback */
      const char *user, *password;
      user = user_input->value();
      password = password_input->value();
      _MSG("a_Dialog_user_passwd: ok = %d\n", ok);
      (*cb)(user, password, vp);
   }
   delete window;

   return ok;
}

