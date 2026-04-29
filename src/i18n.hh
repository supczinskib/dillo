#ifndef __I18N_HH__
#define __I18N_HH__

#include <FL/Fl.H>

#define DILLO_CJK_FONT ((Fl_Font)(FL_FREE_FONT + 1))

#ifdef __cplusplus
extern "C" {
#endif

void a_I18n_init(void);
void a_I18n_apply_fonts(void);
int a_I18n_use_cjk_font(void);
Fl_Font a_I18n_ui_font(Fl_Font fallback);
const char *a_I18n_tr(const char *key);

#ifdef __cplusplus
}
#endif

#define DTR(key) a_I18n_tr(key)

#endif
