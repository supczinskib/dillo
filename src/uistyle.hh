#ifndef __UISTYLE_HH__
#define __UISTYLE_HH__

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Browser_.H>
#include <FL/Fl_Menu_.H>
#include <FL/fl_ask.H>

#include "prefs.h"

static inline Fl_Font a_UI_font(Fl_Font fallback = FL_HELVETICA)
{
   return (prefs.ui_font && prefs.ui_font[0]) ? FL_FREE_FONT : fallback;
}

static inline Fl_Fontsize a_UI_font_size(Fl_Fontsize fallback = 14)
{
   return prefs.ui_font_size > 0 ? (Fl_Fontsize)prefs.ui_font_size : fallback;
}

static inline void a_UI_apply_label_font(Fl_Widget *w, Fl_Font fallback_font = FL_HELVETICA,
                                         Fl_Fontsize fallback_size = 14)
{
   if (!w)
      return;
   if (prefs.ui_font && prefs.ui_font[0])
      w->labelfont(a_UI_font(fallback_font));
   if (prefs.ui_font_size > 0)
      w->labelsize(a_UI_font_size(fallback_size));
}

static inline void a_UI_apply_input_font(Fl_Input *w, Fl_Font fallback_font = FL_HELVETICA,
                                         Fl_Fontsize fallback_size = 14)
{
   if (!w)
      return;
   if (prefs.ui_font && prefs.ui_font[0]) {
      w->labelfont(a_UI_font(fallback_font));
      w->textfont(a_UI_font(fallback_font));
   }
   if (prefs.ui_font_size > 0) {
      w->labelsize(a_UI_font_size(fallback_size));
      w->textsize(a_UI_font_size(fallback_size));
   }
}

static inline void a_UI_apply_output_font(Fl_Output *w, Fl_Font fallback_font = FL_HELVETICA,
                                          Fl_Fontsize fallback_size = 14)
{
   if (!w)
      return;
   if (prefs.ui_font && prefs.ui_font[0]) {
      w->labelfont(a_UI_font(fallback_font));
      w->textfont(a_UI_font(fallback_font));
   }
   if (prefs.ui_font_size > 0) {
      w->labelsize(a_UI_font_size(fallback_size));
      w->textsize(a_UI_font_size(fallback_size));
   }
}

static inline void a_UI_apply_text_display_font(Fl_Text_Display *w,
                                                Fl_Font fallback_font = FL_HELVETICA,
                                                Fl_Fontsize fallback_size = 14)
{
   if (!w)
      return;
   if (prefs.ui_font && prefs.ui_font[0])
      w->textfont(a_UI_font(fallback_font));
   if (prefs.ui_font_size > 0)
      w->textsize(a_UI_font_size(fallback_size));
}

static inline void a_UI_apply_menu_font(Fl_Menu_Item *menu)
{
   if (!menu)
      return;

   for (Fl_Menu_Item *item = menu; item->text; ++item) {
      if (prefs.ui_font && prefs.ui_font[0])
         item->labelfont(a_UI_font());
      if (prefs.ui_font_size > 0)
         item->labelsize(a_UI_font_size());

      if ((item->flags & FL_SUBMENU_POINTER) && item->user_data())
         a_UI_apply_menu_font((Fl_Menu_Item *)item->user_data());
   }
}

static inline void a_UI_apply_browser_font(Fl_Browser_ *w, Fl_Font fallback_font = FL_HELVETICA,
                                           Fl_Fontsize fallback_size = 14)
{
   if (!w)
      return;
   if (prefs.ui_font && prefs.ui_font[0])
      w->textfont(a_UI_font(fallback_font));
   if (prefs.ui_font_size > 0)
      w->textsize(a_UI_font_size(fallback_size));
}

static inline void a_UI_apply_menu_widget_font(Fl_Menu_ *w)
{
   if (!w)
      return;
   if (prefs.ui_font && prefs.ui_font[0]) {
      w->labelfont(a_UI_font());
      w->textfont(a_UI_font());
   }
   if (prefs.ui_font_size > 0) {
      w->labelsize(a_UI_font_size());
      w->textsize(a_UI_font_size());
   }
   a_UI_apply_menu_font((Fl_Menu_Item *)w->menu());
}

static inline void a_UI_apply_message_font()
{
   if (prefs.ui_font && prefs.ui_font[0]) {
      fl_message_font(a_UI_font(), a_UI_font_size());
   } else if (prefs.ui_font_size > 0) {
      fl_message_font(FL_HELVETICA, a_UI_font_size());
   }
}

static inline void a_UI_apply_widget_tree_font(Fl_Widget *w)
{
   if (!w)
      return;

   /* Dillo is built with -fno-rtti, so this function must not use
    * dynamic_cast.  It is intentionally conservative: apply label font
    * recursively to every widget.  Widgets that need textfont/textsize
    * (inputs, browsers, menus, text displays) are styled explicitly at
    * their construction sites, where their concrete type is known.
    */
   a_UI_apply_label_font(w);

   if (Fl_Group *group = w->as_group()) {
      for (int i = 0; i < group->children(); ++i)
         a_UI_apply_widget_tree_font(group->child(i));
   }
}

#endif /* __UISTYLE_HH__ */
