/*
 * File: i18n.cc
 * Built-in UI localization for Dillo, modeled after textedit.cxx.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>

#include "i18n.hh"

struct TranslationEntry {
   const char *key;
   const char *ar;
   const char *zh_CN;
   const char *nl;
   const char *en;
   const char *fr;
   const char *de;
   const char *it;
   const char *pl;
   const char *pt;
   const char *ru;
   const char *es;
   const char *tr;
};

static const char *app_lang = "en";
static char app_cjk_font_name[256] = "WenQuanYi Micro Hei";

static char *trim_space(char *s)
{
   while (*s && isspace((unsigned char)*s))
      ++s;
   char *end = s + strlen(s);
   while (end > s && isspace((unsigned char)end[-1]))
      --end;
   *end = 0;
   return s;
}

static const char *resource_name_tail(const char *key)
{
   const char *dot = strrchr(key, '.');
   return (dot && dot[1]) ? dot + 1 : key;
}

static void set_string_resource(const char *key, const char *value)
{
   const char *name = resource_name_tail(key);
   if (!strcmp(name, "cjkFont") && value && *value) {
      strncpy(app_cjk_font_name, value, sizeof(app_cjk_font_name) - 1);
      app_cjk_font_name[sizeof(app_cjk_font_name) - 1] = 0;
   }
}

static void load_string_resources_from_file(const char *path)
{
   FILE *fp = fopen(path, "r");
   if (!fp)
      return;

   char line[512];
   while (fgets(line, sizeof(line), fp)) {
      char *p = trim_space(line);
      if (!*p || *p == '!')
         continue;
      char *colon = strchr(p, ':');
      if (!colon)
         continue;
      *colon = 0;
      char *key = trim_space(p);
      char *value = trim_space(colon + 1);
      if (!strncmp(key, "Dillo.", 6) || !strncmp(key, "XMenu.", 6))
         set_string_resource(key, value);
   }
   fclose(fp);
}

static void load_string_resources(void)
{
   load_string_resources_from_file("/usr/share/X11/app-defaults/Dillo");
}

static int lang_matches(const char *value, const char *lang_code)
{
   if (!value || !*value || !lang_code || !*lang_code)
      return 0;
   size_t n = strlen(lang_code);
   return strncmp(value, lang_code, n) == 0 &&
          (value[n] == '\0' || value[n] == '_' || value[n] == '-' || value[n] == '.');
}

static const char *detect_app_language(void)
{
   const char *value = getenv("LANGUAGE");
   if (lang_matches(value, "ar")) return "ar";
   if (lang_matches(value, "zh_CN") || lang_matches(value, "zh")) return "zh_CN";
   if (lang_matches(value, "nl")) return "nl";
   if (lang_matches(value, "fr")) return "fr";
   if (lang_matches(value, "de")) return "de";
   if (lang_matches(value, "it")) return "it";
   if (lang_matches(value, "pl")) return "pl";
   if (lang_matches(value, "pt")) return "pt";
   if (lang_matches(value, "ru")) return "ru";
   if (lang_matches(value, "es")) return "es";
   if (lang_matches(value, "tr")) return "tr";
   if (lang_matches(value, "en")) return "en";
   return "en";
}

static const TranslationEntry translations[] = {
   {"Cut", "قص", "剪切", "Knippen", "Cut", "Couper", "Ausschneiden", "Taglia", "Wytnij", "Cortar", "Вырезать", "Cortar", "Kes"},
   {"Copy", "نسخ", "复制", "Kopiëren", "Copy", "Copier", "Kopieren", "Copia", "Kopiuj", "Copiar", "Копировать", "Copiar", "Kopyala"},
   {"Paste", "لصق", "粘贴", "Plakken", "Paste", "Coller", "Einfügen", "Incolla", "Wklej", "Colar", "Вставить", "Pegar", "Yapıştır"},
   {"OK", "موافق", "确定", "OK", "OK", "OK", "OK", "OK", "OK", "OK", "ОК", "Aceptar", "Tamam"},
   {"Cancel", "إلغاء", "取消", "Annuleren", "Cancel", "Annuler", "Abbrechen", "Annulla", "Anuluj", "Cancelar", "Отмена", "Cancelar", "İptal"},
   {"Close", "إغلاق", "关闭", "Sluiten", "Close", "Fermer", "Schließen", "Chiudi", "Zamknij", "Fechar", "Закрыть", "Cerrar", "Kapat"},
   {"Continue", "متابعة", "继续", "Doorgaan", "Continue", "Continuer", "Fortfahren", "Continua", "Kontynuuj", "Continuar", "Продолжить", "Continuar", "Devam et"},
   {"Abort", "إيقاف", "中止", "Afbreken", "Abort", "Abandonner", "Abbrechen", "Interrompi", "Przerwij", "Abortar", "Прервать", "Abortar", "Vazgeç"},
   {"Rename", "إعادة تسمية", "重命名", "Hernoemen", "Rename", "Renommer", "Umbenennen", "Rinomina", "Zmień nazwę", "Mudar nome", "Переименовать", "Renombrar", "Yeniden adlandır"},
   {"Quit", "إنهاء", "退出", "Afsluiten", "Quit", "Quitter", "Beenden", "Esci", "Zakończ", "Sair", "Выйти", "Salir", "Çık"},
   {"User", "المستخدم", "用户", "Gebruiker", "User", "Utilisateur", "Benutzer", "Utente", "Użytkownik", "Utilizador", "Пользователь", "Usuario", "Kullanıcı"},
   {"Password", "كلمة المرور", "密码", "Wachtwoord", "Password", "Mot de passe", "Passwort", "Password", "Hasło", "Palavra-passe", "Пароль", "Contraseña", "Parola"},
   {"Dillo: Message", "ديلو: رسالة", "Dillo：消息", "Dillo: Bericht", "Dillo: Message", "Dillo : message", "Dillo: Meldung", "Dillo: messaggio", "Dillo: Komunikat", "Dillo: Mensagem", "Dillo: сообщение", "Dillo: Mensaje", "Dillo: İleti"},
   {"Dillo: Input", "ديلو: إدخال", "Dillo：输入", "Dillo: Invoer", "Dillo: Input", "Dillo : saisie", "Dillo: Eingabe", "Dillo: input", "Dillo: Dane wejściowe", "Dillo: Entrada", "Dillo: ввод", "Dillo: Entrada", "Dillo: Giriş"},
   {"Dillo: Password", "ديلو: كلمة المرور", "Dillo：密码", "Dillo: Wachtwoord", "Dillo: Password", "Dillo : mot de passe", "Dillo: Passwort", "Dillo: password", "Dillo: Hasło", "Dillo: Palavra-passe", "Dillo: пароль", "Dillo: Contraseña", "Dillo: Parola"},
   {"Dillo: Text", "ديلو: نص", "Dillo：文本", "Dillo: Tekst", "Dillo: Text", "Dillo : texte", "Dillo: Text", "Dillo: testo", "Dillo: Tekst", "Dillo: Texto", "Dillo: текст", "Dillo: Texto", "Dillo: Metin"},
   {"Dillo: Choice", "ديلو: اختيار", "Dillo：选择", "Dillo: Keuze", "Dillo: Choice", "Dillo : choix", "Dillo: Auswahl", "Dillo: scelta", "Dillo: Wybór", "Dillo: Escolha", "Dillo: выбор", "Dillo: Elección", "Dillo: Seçim"},
   {"Dillo: User/Password", "ديلو: المستخدم/كلمة المرور", "Dillo：用户/密码", "Dillo: Gebruiker/wachtwoord", "Dillo: User/Password", "Dillo : utilisateur/mot de passe", "Dillo: Benutzer/Passwort", "Dillo: utente/password", "Dillo: Użytkownik/hasło", "Dillo: Utilizador/palavra-passe", "Dillo: пользователь/пароль", "Dillo: Usuario/contraseña", "Dillo: Kullanıcı/parola"},
   {"Preview", "معاينة", "预览", "Voorbeeld", "Preview", "Aperçu", "Vorschau", "Anteprima", "Podgląd", "Pré-visualizar", "Предпросмотр", "Vista previa", "Önizleme"},
   {"New Directory?", "دليل جديد؟", "新建目录？", "Nieuwe map?", "New Directory?", "Nouveau dossier ?", "Neues Verzeichnis?", "Nuova cartella?", "Nowy katalog?", "Novo diretório?", "Новый каталог?", "¿Nuevo directorio?", "Yeni dizin?"},
   {"Create a new directory.", "إنشاء دليل جديد.", "创建新目录。", "Een nieuwe map maken.", "Create a new directory.", "Créer un nouveau dossier.", "Neues Verzeichnis erstellen.", "Crea una nuova cartella.", "Utwórz nowy katalog.", "Criar um novo diretório.", "Создать новый каталог.", "Crear un nuevo directorio.", "Yeni bir dizin oluştur."},
   {"Show hidden files", "إظهار الملفات المخفية", "显示隐藏文件", "Verborgen bestanden tonen", "Show hidden files", "Afficher les fichiers cachés", "Versteckte Dateien anzeigen", "Mostra file nascosti", "Pokaż ukryte pliki", "Mostrar ficheiros ocultos", "Показывать скрытые файлы", "Mostrar archivos ocultos", "Gizli dosyaları göster"},
   {"Show:", "عرض:", "显示:", "Weergeven:", "Show:", "Afficher :", "Anzeigen:", "Mostra:", "Pokaż:", "Mostrar:", "Показать:", "Mostrar:", "Göster:"},
   {"Filename:", "اسم الملف:", "文件名:", "Bestandsnaam:", "Filename:", "Nom du fichier :", "Dateiname:", "Nome file:", "Nazwa pliku:", "Nome do ficheiro:", "Имя файла:", "Nombre del archivo:", "Dosya adı:"},
   {"All Files (*)", "كل الملفات (*)", "所有文件 (*)", "Alle bestanden (*)", "All Files (*)", "Tous les fichiers (*)", "Alle Dateien (*)", "Tutti i file (*)", "Wszystkie pliki (*)", "Todos os ficheiros (*)", "Все файлы (*)", "Todos los archivos (*)", "Tüm dosyalar (*)"},
   {"Custom Filter", "مرشح مخصص", "自定义筛选器", "Aangepast filter", "Custom Filter", "Filtre personnalisé", "Benutzerdefinierter Filter", "Filtro personalizzato", "Filtr niestandardowy", "Filtro personalizado", "Пользовательский фильтр", "Filtro personalizado", "Özel filtre"},
   {"Favorites", "المفضلة", "收藏夹", "Favorieten", "Favorites", "Favoris", "Favoriten", "Preferiti", "Ulubione", "Favoritos", "Избранное", "Favoritos", "Favoriler"},
   {"Add to Favorites", "أضف إلى المفضلة", "添加到收藏夹", "Toevoegen aan favorieten", "Add to Favorites", "Ajouter aux favoris", "Zu Favoriten hinzufügen", "Aggiungi ai preferiti", "Dodaj do ulubionych", "Adicionar aos favoritos", "Добавить в избранное", "Agregar a favoritos", "Favorilere ekle"},
   {"Manage Favorites", "إدارة المفضلة", "管理收藏夹", "Favorieten beheren", "Manage Favorites", "Gérer les favoris", "Favoriten verwalten", "Gestisci preferiti", "Zarządzaj ulubionymi", "Gerir favoritos", "Управление избранным", "Administrar favoritos", "Favorileri yönet"},
   {"File Systems", "أنظمة الملفات", "文件系统", "Bestandssystemen", "File Systems", "Systèmes de fichiers", "Dateisysteme", "File system", "Systemy plików", "Sistemas de ficheiros", "Файловые системы", "Sistemas de archivos", "Dosya sistemleri"},
   {"Please choose an existing file!", "يرجى اختيار ملف موجود!", "请选择一个现有文件！", "Kies een bestaand bestand!", "Please choose an existing file!", "Veuillez choisir un fichier existant !", "Bitte wählen Sie eine vorhandene Datei aus!", "Seleziona un file esistente!", "Wybierz istniejący plik!", "Escolha um ficheiro existente!", "Выберите существующий файл!", "¡Elija un archivo existente!", "Lütfen mevcut bir dosya seçin!"},
   {"Select search engine", "اختر محرك البحث", "选择搜索引擎", "Zoekmachine selecteren", "Select search engine", "Sélectionner le moteur de recherche", "Suchmaschine auswählen", "Seleziona motore di ricerca", "Wybierz wyszukiwarkę", "Selecionar motor de pesquisa", "Выберите поисковую систему", "Seleccionar motor de búsqueda", "Arama motoru seç"},
   {"Hide", "إخفاء", "隐藏", "Verbergen", "Hide", "Masquer", "Ausblenden", "Nascondi", "Ukryj", "Ocultar", "Скрыть", "Ocultar", "Gizle"},
   {"Next", "التالي", "下一个", "Volgende", "Next", "Suivant", "Weiter", "Avanti", "Następny", "Seguinte", "Далее", "Siguiente", "Sonraki"},
   {"Previous", "السابق", "上一个", "Vorige", "Previous", "Précédent", "Zurück", "Precedente", "Poprzedni", "Anterior", "Назад", "Anterior", "Önceki"},
   {"Case-sensitive", "حساس لحالة الأحرف", "区分大小写", "Hoofdlettergevoelig", "Case-sensitive", "Sensible à la casse", "Groß-/Kleinschreibung beachten", "Distingui maiuscole", "Uwzględniaj wielkość liter", "Diferenciar maiúsculas", "С учётом регистра", "Distinguir mayúsculas", "Büyük/küçük harfe duyarlı"},
   {"Find next occurrence of the search phrase\nshortcut: Enter", "البحث عن الظهور التالي لعبارة البحث\nالاختصار: Enter", "查找搜索短语的下一个匹配项\n快捷键：Enter", "Volgende overeenkomst zoeken\nsneltoets: Enter", "Find next occurrence of the search phrase\nshortcut: Enter", "Trouver l’occurrence suivante\nraccourci : Entrée", "Nächstes Vorkommen suchen\nTastenkürzel: Eingabe", "Trova occorrenza successiva\nscorciatoia: Invio", "Znajdź następne wystąpienie\nskrót: Enter", "Localizar próxima ocorrência\natalho: Enter", "Найти следующее вхождение\nклавиша: Enter", "Buscar la siguiente coincidencia\natajo: Intro", "Sonraki eşleşmeyi bul\nkısayol: Enter"},
   {"Find previous occurrence of the search phrase\nshortcut: Shift+Enter", "البحث عن الظهور السابق لعبارة البحث\nالاختصار: Shift+Enter", "查找搜索短语的上一个匹配项\n快捷键：Shift+Enter", "Vorige overeenkomst zoeken\nsneltoets: Shift+Enter", "Find previous occurrence of the search phrase\nshortcut: Shift+Enter", "Trouver l’occurrence précédente\nraccourci : Maj+Entrée", "Vorheriges Vorkommen suchen\nTastenkürzel: Umschalt+Eingabe", "Trova occorrenza precedente\nscorciatoia: Maiusc+Invio", "Znajdź poprzednie wystąpienie\nskrót: Shift+Enter", "Localizar ocorrência anterior\natalho: Shift+Enter", "Найти предыдущее вхождение\nклавиша: Shift+Enter", "Buscar la coincidencia anterior\natajo: Mayús+Intro", "Önceki eşleşmeyi bul\nkısayol: Shift+Enter"},
   {"Back", "رجوع", "后退", "Terug", "Back", "Retour", "Zurück", "Indietro", "Wstecz", "Voltar", "Назад", "Atrás", "Geri"},
   {"Forw", "أمام", "前进", "Vooruit", "Forw", "Suiv.", "Vor", "Avanti", "Dalej", "Avançar", "Вперёд", "Adel.", "İleri"},
   {"Home", "الرئيسية", "主页", "Start", "Home", "Accueil", "Start", "Home", "Start", "Início", "Домой", "Inicio", "Ana sayfa"},
   {"Reload", "إعادة تحميل", "重新加载", "Herladen", "Reload", "Recharger", "Neu laden", "Ricarica", "Odśwież", "Recarregar", "Обновить", "Recargar", "Yenile"},
   {"Save", "حفظ", "保存", "Opslaan", "Save", "Enregistrer", "Speichern", "Salva", "Zapisz", "Guardar", "Сохранить", "Guardar", "Kaydet"},
   {"Stop", "إيقاف", "停止", "Stoppen", "Stop", "Arrêter", "Stopp", "Ferma", "Stop", "Parar", "Стоп", "Detener", "Durdur"},
   {"Book", "علامات", "书签", "Bladw.", "Book", "Signets", "Lesez.", "Segnalibri", "Zakł.", "Marc.", "Закл.", "Marc.", "Yer imleri"},
   {"Tools", "أدوات", "工具", "Extra", "Tools", "Outils", "Werkzeuge", "Strumenti", "Narzędzia", "Ferramentas", "Инструменты", "Herram.", "Araçlar"},
   {"Previous page", "الصفحة السابقة", "上一页", "Vorige pagina", "Previous page", "Page précédente", "Vorherige Seite", "Pagina precedente", "Poprzednia strona", "Página anterior", "Предыдущая страница", "Página anterior", "Önceki sayfa"},
   {"Next page", "الصفحة التالية", "下一页", "Volgende pagina", "Next page", "Page suivante", "Nächste Seite", "Pagina successiva", "Następna strona", "Página seguinte", "Следующая страница", "Página siguiente", "Sonraki sayfa"},
   {"Save this page", "احفظ هذه الصفحة", "保存此页面", "Deze pagina opslaan", "Save this page", "Enregistrer cette page", "Diese Seite speichern", "Salva questa pagina", "Zapisz tę stronę", "Guardar esta página", "Сохранить эту страницу", "Guardar esta página", "Bu sayfayı kaydet"},
   {"Stop loading", "إيقاف التحميل", "停止加载", "Laden stoppen", "Stop loading", "Arrêter le chargement", "Laden stoppen", "Ferma caricamento", "Zatrzymaj ładowanie", "Parar carregamento", "Остановить загрузку", "Detener carga", "Yüklemeyi durdur"},
   {"Settings", "إعدادات", "设置", "Instellingen", "Settings", "Paramètres", "Einstellungen", "Impostazioni", "Ustawienia", "Definições", "Настройки", "Configuración", "Ayarlar"},
   {"Location", "الموقع", "位置", "Locatie", "Location", "Adresse", "Adresse", "Posizione", "Adres", "Localização", "Адрес", "Ubicación", "Konum"},
   {"Search the Web", "ابحث في الويب", "搜索网页", "Zoeken op internet", "Search the Web", "Rechercher sur le Web", "Im Web suchen", "Cerca nel Web", "Szukaj w sieci", "Pesquisar na Web", "Искать в сети", "Buscar en la Web", "Web’de ara"},
   {"Help", "مساعدة", "帮助", "Help", "Help", "Aide", "Hilfe", "Aiuto", "Pomoc", "Ajuda", "Справка", "Ayuda", "Yardım"},
   {"&File", "&ملف", "文件(&F)", "&Bestand", "&File", "&Fichier", "&Datei", "&File", "&Plik", "&Ficheiro", "&Файл", "&Archivo", "&Dosya"},
   {"File menu", "قائمة الملف", "文件菜单", "Bestandsmenu", "File menu", "Menu Fichier", "Dateimenü", "Menu File", "Menu pliku", "Menu Ficheiro", "Меню Файл", "Menú Archivo", "Dosya menüsü"},
   {"Welcome...", "مرحباً...", "欢迎...", "Welkom...", "Welcome...", "Bienvenue...", "Willkommen...", "Benvenuto...", "Witamy...", "Bem-vindo...", "Добро пожаловать...", "Bienvenido...", "Hoş geldiniz..."},
   {"Page", "صفحة", "页面", "Pagina", "Page", "Page", "Seite", "Pagina", "Strona", "Página", "Страница", "Página", "Sayfa"},
   {"Images", "صور", "图像", "Afbeeldingen", "Images", "Images", "Bilder", "Immagini", "Obrazy", "Imagens", "Изображения", "Imágenes", "Görseller"},
   {"View page source", "عرض مصدر الصفحة", "查看页面源代码", "Paginabron bekijken", "View page source", "Afficher le source de la page", "Seitenquelltext anzeigen", "Visualizza sorgente pagina", "Pokaż źródło strony", "Ver código da página", "Показать исходный код страницы", "Ver código fuente de la página", "Sayfa kaynağını göster"},
   {"View page bugs", "عرض أخطاء الصفحة", "查看页面错误", "Paginafouten bekijken", "View page bugs", "Afficher les erreurs de la page", "Seitenfehler anzeigen", "Visualizza errori pagina", "Pokaż błędy strony", "Ver erros da página", "Показать ошибки страницы", "Ver errores de la página", "Sayfa hatalarını göster"},
   {"View stylesheets", "عرض أوراق الأنماط", "查看样式表", "Stijlbladen bekijken", "View stylesheets", "Afficher les feuilles de style", "Stylesheets anzeigen", "Visualizza fogli di stile", "Pokaż arkusze stylów", "Ver folhas de estilo", "Показать таблицы стилей", "Ver hojas de estilo", "Stil sayfalarını göster"},
   {"Bookmark this page", "أضف هذه الصفحة إلى العلامات", "收藏此页面", "Deze pagina aan bladwijzers toevoegen", "Bookmark this page", "Ajouter cette page aux signets", "Diese Seite als Lesezeichen speichern", "Aggiungi questa pagina ai segnalibri", "Dodaj stronę do zakładek", "Adicionar esta página aos favoritos", "Добавить эту страницу в закладки", "Agregar esta página a marcadores", "Bu sayfayı yer imlerine ekle"},
   {"Find text", "ابحث عن نص", "查找文本", "Tekst zoeken", "Find text", "Rechercher du texte", "Text suchen", "Trova testo", "Znajdź tekst", "Localizar texto", "Найти текст", "Buscar texto", "Metin bul"},
   {"Save page as...", "احفظ الصفحة باسم...", "页面另存为...", "Pagina opslaan als...", "Save page as...", "Enregistrer la page sous...", "Seite speichern unter...", "Salva pagina con nome...", "Zapisz stronę jako...", "Guardar página como...", "Сохранить страницу как...", "Guardar página como...", "Sayfayı farklı kaydet..."},
   {"Page menu", "قائمة الصفحة", "页面菜单", "Paginamenu", "Page menu", "Menu Page", "Seitenmenü", "Menu pagina", "Menu strony", "Menu da página", "Меню страницы", "Menú de página", "Sayfa menüsü"},
   {"Open link in new tab", "افتح الرابط في لسان جديد", "在新标签页中打开链接", "Link openen in nieuw tabblad", "Open link in new tab", "Ouvrir le lien dans un nouvel onglet", "Link in neuem Tab öffnen", "Apri link in nuova scheda", "Otwórz link w nowej karcie", "Abrir ligação num novo separador", "Открыть ссылку в новой вкладке", "Abrir enlace en una pestaña nueva", "Bağlantıyı yeni sekmede aç"},
   {"Open link in new window", "افتح الرابط في نافذة جديدة", "在新窗口中打开链接", "Link openen in nieuw venster", "Open link in new window", "Ouvrir le lien dans une nouvelle fenêtre", "Link in neuem Fenster öffnen", "Apri link in nuova finestra", "Otwórz link w nowym oknie", "Abrir ligação numa nova janela", "Открыть ссылку в новом окне", "Abrir enlace en una ventana nueva", "Bağlantıyı yeni pencerede aç"},
   {"Bookmark this link", "أضف هذا الرابط إلى العلامات", "收藏此链接", "Deze link aan bladwijzers toevoegen", "Bookmark this link", "Ajouter ce lien aux signets", "Diesen Link als Lesezeichen speichern", "Aggiungi questo link ai segnalibri", "Dodaj link do zakładek", "Adicionar esta ligação aos favoritos", "Добавить эту ссылку в закладки", "Agregar este enlace a marcadores", "Bu bağlantıyı yer imlerine ekle"},
   {"Copy link location", "انسخ عنوان الرابط", "复制链接地址", "Linkadres kopiëren", "Copy link location", "Copier l’adresse du lien", "Linkadresse kopieren", "Copia indirizzo link", "Kopiuj adres linku", "Copiar endereço da ligação", "Копировать адрес ссылки", "Copiar dirección del enlace", "Bağlantı adresini kopyala"},
   {"Save link as...", "احفظ الرابط باسم...", "链接另存为...", "Link opslaan als...", "Save link as...", "Enregistrer le lien sous...", "Link speichern unter...", "Salva link con nome...", "Zapisz link jako...", "Guardar ligação como...", "Сохранить ссылку как...", "Guardar enlace como...", "Bağlantıyı farklı kaydet..."},
   {"Link menu", "قائمة الرابط", "链接菜单", "Linkmenu", "Link menu", "Menu Lien", "Linkmenü", "Menu link", "Menu linku", "Menu da ligação", "Меню ссылки", "Menú de enlace", "Bağlantı menüsü"},
   {"Isolate image", "اعزل الصورة", "单独显示图像", "Afbeelding isoleren", "Isolate image", "Isoler l’image", "Bild isolieren", "Isola immagine", "Wyizoluj obraz", "Isolar imagem", "Изолировать изображение", "Aislar imagen", "Görseli ayır"},
   {"Open image in new tab", "افتح الصورة في لسان جديد", "在新标签页中打开图像", "Afbeelding openen in nieuw tabblad", "Open image in new tab", "Ouvrir l’image dans un nouvel onglet", "Bild in neuem Tab öffnen", "Apri immagine in nuova scheda", "Otwórz obraz w nowej karcie", "Abrir imagem num novo separador", "Открыть изображение в новой вкладке", "Abrir imagen en una pestaña nueva", "Görseli yeni sekmede aç"},
   {"Open image in new window", "افتح الصورة في نافذة جديدة", "在新窗口中打开图像", "Afbeelding openen in nieuw venster", "Open image in new window", "Ouvrir l’image dans une nouvelle fenêtre", "Bild in neuem Fenster öffnen", "Apri immagine in nuova finestra", "Otwórz obraz w nowym oknie", "Abrir imagem numa nova janela", "Открыть изображение в новом окне", "Abrir imagen en una ventana nueva", "Görseli yeni pencerede aç"},
   {"Load image", "حمّل الصورة", "加载图像", "Afbeelding laden", "Load image", "Charger l’image", "Bild laden", "Carica immagine", "Załaduj obraz", "Carregar imagem", "Загрузить изображение", "Cargar imagen", "Görseli yükle"},
   {"Bookmark this image", "أضف هذه الصورة إلى العلامات", "收藏此图像", "Deze afbeelding aan bladwijzers toevoegen", "Bookmark this image", "Ajouter cette image aux signets", "Dieses Bild als Lesezeichen speichern", "Aggiungi questa immagine ai segnalibri", "Dodaj obraz do zakładek", "Adicionar esta imagem aos favoritos", "Добавить это изображение в закладки", "Agregar esta imagen a marcadores", "Bu görseli yer imlerine ekle"},
   {"Copy image location", "انسخ عنوان الصورة", "复制图像地址", "Afbeeldingsadres kopiëren", "Copy image location", "Copier l’adresse de l’image", "Bildadresse kopieren", "Copia indirizzo immagine", "Kopiuj adres obrazu", "Copiar endereço da imagem", "Копировать адрес изображения", "Copiar dirección de la imagen", "Görsel adresini kopyala"},
   {"Save image as...", "احفظ الصورة باسم...", "图像另存为...", "Afbeelding opslaan als...", "Save image as...", "Enregistrer l’image sous...", "Bild speichern unter...", "Salva immagine con nome...", "Zapisz obraz jako...", "Guardar imagem como...", "Сохранить изображение как...", "Guardar imagen como...", "Görseli farklı kaydet..."},
   {"Image menu", "قائمة الصورة", "图像菜单", "Afbeeldingsmenu", "Image menu", "Menu Image", "Bildmenü", "Menu immagine", "Menu obrazu", "Menu da imagem", "Меню изображения", "Menú de imagen", "Görsel menüsü"},
   {"Submit form", "أرسل النموذج", "提交表单", "Formulier verzenden", "Submit form", "Envoyer le formulaire", "Formular absenden", "Invia modulo", "Wyślij formularz", "Enviar formulário", "Отправить форму", "Enviar formulario", "Formu gönder"},
   {"Reset form", "إعادة تعيين النموذج", "重置表单", "Formulier herstellen", "Reset form", "Réinitialiser le formulaire", "Formular zurücksetzen", "Reimposta modulo", "Resetuj formularz", "Repor formulário", "Сбросить форму", "Restablecer formulario", "Formu sıfırla"},
   {"Form menu", "قائمة النموذج", "表单菜单", "Formuliermenu", "Form menu", "Menu Formulaire", "Formularmenü", "Menu modulo", "Menu formularza", "Menu do formulário", "Меню формы", "Menú de formulario", "Form menüsü"},
   {"Hide hiddens", "أخفِ المخفية", "隐藏隐藏项", "Verborgen items verbergen", "Hide hiddens", "Masquer les éléments cachés", "Versteckte ausblenden", "Nascondi nascosti", "Ukryj ukryte", "Ocultar ocultos", "Скрыть скрытые", "Ocultar ocultos", "Gizlileri sakla"},
   {"Show hiddens", "أظهر المخفية", "显示隐藏项", "Verborgen items tonen", "Show hiddens", "Afficher les éléments cachés", "Versteckte anzeigen", "Mostra nascosti", "Pokaż ukryte", "Mostrar ocultos", "Показать скрытые", "Mostrar ocultos", "Gizlileri göster"},
   {"New tab", "لسان جديد", "新建标签页", "Nieuw tabblad", "New tab", "Nouvel onglet", "Neuer Tab", "Nuova scheda", "Nowa karta", "Novo separador", "Новая вкладка", "Nueva pestaña", "Yeni sekme"},
   {"New window", "نافذة جديدة", "新建窗口", "Nieuw venster", "New window", "Nouvelle fenêtre", "Neues Fenster", "Nuova finestra", "Nowe okno", "Nova janela", "Новое окно", "Nueva ventana", "Yeni pencere"},
   {"Open file...", "افتح ملفاً...", "打开文件...", "Bestand openen...", "Open file...", "Ouvrir un fichier...", "Datei öffnen...", "Apri file...", "Otwórz plik...", "Abrir ficheiro...", "Открыть файл...", "Abrir archivo...", "Dosya aç..."},
   {"Exit Dillo", "اخرج من ديلو", "退出 Dillo", "Dillo afsluiten", "Exit Dillo", "Quitter Dillo", "Dillo beenden", "Esci da Dillo", "Zakończ Dillo", "Sair do Dillo", "Выйти из Dillo", "Salir de Dillo", "Dillo’dan çık"},
   {"About bug meter", "حول عداد الأخطاء", "关于错误计数器", "Over foutenmeter", "About bug meter", "À propos du compteur d’erreurs", "Über den Fehlermesser", "Informazioni sul misuratore errori", "O liczniku błędów", "Sobre o medidor de erros", "О счётчике ошибок", "Acerca del medidor de errores", "Hata göstergesi hakkında"},
   {"Use remote CSS", "استخدم CSS البعيد", "使用远程 CSS", "Externe CSS gebruiken", "Use remote CSS", "Utiliser le CSS distant", "Entferntes CSS verwenden", "Usa CSS remoto", "Używaj zdalnego CSS", "Usar CSS remoto", "Использовать удалённый CSS", "Usar CSS remoto", "Uzak CSS kullan"},
   {"Use embedded CSS", "استخدم CSS المضمّن", "使用嵌入式 CSS", "Ingebedde CSS gebruiken", "Use embedded CSS", "Utiliser le CSS intégré", "Eingebettetes CSS verwenden", "Usa CSS incorporato", "Używaj osadzonego CSS", "Usar CSS incorporado", "Использовать встроенный CSS", "Usar CSS incrustado", "Gömülü CSS kullan"},
   {"Load images", "حمّل الصور", "加载图像", "Afbeeldingen laden", "Load images", "Charger les images", "Bilder laden", "Carica immagini", "Ładuj obrazy", "Carregar imagens", "Загружать изображения", "Cargar imágenes", "Görselleri yükle"},
   {"Load background images", "حمّل صور الخلفية", "加载背景图像", "Achtergrondafbeeldingen laden", "Load background images", "Charger les images d’arrière-plan", "Hintergrundbilder laden", "Carica immagini di sfondo", "Ładuj obrazy tła", "Carregar imagens de fundo", "Загружать фоновые изображения", "Cargar imágenes de fondo", "Arka plan görsellerini yükle"},
   {"Force HTTPS", "افرض HTTPS", "强制 HTTPS", "HTTPS afdwingen", "Force HTTPS", "Forcer HTTPS", "HTTPS erzwingen", "Forza HTTPS", "Wymuś HTTPS", "Forçar HTTPS", "Принудительно использовать HTTPS", "Forzar HTTPS", "HTTPS zorla"},
   {"Panel size", "حجم اللوحة", "面板大小", "Paneelgrootte", "Panel size", "Taille du panneau", "Panelgröße", "Dimensione pannello", "Rozmiar panelu", "Tamanho do painel", "Размер панели", "Tamaño del panel", "Panel boyutu"},
   {"tiny", "دقيق", "极小", "zeer klein", "tiny", "minuscule", "winzig", "minuscolo", "bardzo mały", "minúsculo", "крошечный", "diminuto", "çok küçük"},
   {"small", "صغير", "小", "klein", "small", "petit", "klein", "piccolo", "mały", "pequeno", "маленький", "pequeño", "küçük"},
   {"medium", "متوسط", "中", "middelgroot", "medium", "moyen", "mittel", "medio", "średni", "médio", "средний", "mediano", "orta"},
   {"small icons", "أيقونات صغيرة", "小图标", "kleine pictogrammen", "small icons", "petites icônes", "kleine Symbole", "icone piccole", "małe ikony", "ícones pequenos", "маленькие значки", "iconos pequeños", "küçük simgeler"},
   {"Dillo: Close window?", "ديلو: إغلاق النافذة؟", "Dillo：关闭窗口？", "Dillo: Venster sluiten?", "Dillo: Close window?", "Dillo : fermer la fenêtre ?", "Dillo: Fenster schließen?", "Dillo: chiudere la finestra?", "Dillo: Zamknąć okno?", "Dillo: Fechar janela?", "Dillo: закрыть окно?", "Dillo: ¿Cerrar ventana?", "Dillo: Pencere kapatılsın mı?"},
   {"Window contains more than one tab.", "تحتوي النافذة على أكثر من لسان.", "窗口包含多个标签页。", "Venster bevat meer dan één tabblad.", "Window contains more than one tab.", "La fenêtre contient plus d’un onglet.", "Das Fenster enthält mehr als einen Tab.", "La finestra contiene più di una scheda.", "Okno zawiera więcej niż jedną kartę.", "A janela contém mais de um separador.", "Окно содержит больше одной вкладки.", "La ventana contiene más de una pestaña.", "Pencerede birden fazla sekme var."},
   {"Dillo: Quit?", "ديلو: إنهاء؟", "Dillo：退出？", "Dillo: Afsluiten?", "Dillo: Quit?", "Dillo : quitter ?", "Dillo: Beenden?", "Dillo: uscire?", "Dillo: Zakończyć?", "Dillo: Sair?", "Dillo: выйти?", "Dillo: ¿Salir?", "Dillo: Çıkılsın mı?"},
   {"More than one open tab or window.", "يوجد أكثر من لسان أو نافذة مفتوحة.", "打开了多个标签页或窗口。", "Meer dan één geopend tabblad of venster.", "More than one open tab or window.", "Plus d’un onglet ou d’une fenêtre est ouvert.", "Mehr als ein Tab oder Fenster ist geöffnet.", "Più di una scheda o finestra aperta.", "Otwarta jest więcej niż jedna karta lub okno.", "Há mais de um separador ou janela aberto.", "Открыто больше одной вкладки или окна.", "Hay más de una pestaña o ventana abierta.", "Birden fazla sekme veya pencere açık."},
   {"The file: %s (%d Bytes) already exists. What do we do?", "الملف: %s (%d بايت) موجود بالفعل. ماذا نفعل؟", "文件：%s（%d 字节）已存在。要怎么做？", "Het bestand: %s (%d bytes) bestaat al. Wat doen we?", "The file: %s (%d Bytes) already exists. What do we do?", "Le fichier : %s (%d octets) existe déjà. Que faire ?", "Die Datei %s (%d Bytes) existiert bereits. Was tun?", "Il file: %s (%d byte) esiste già. Cosa fare?", "Plik: %s (%d bajtów) już istnieje. Co zrobić?", "O ficheiro: %s (%d bytes) já existe. O que fazer?", "Файл %s (%d байт) уже существует. Что делать?", "El archivo: %s (%d bytes) ya existe. ¿Qué hacemos?", "Dosya: %s (%d bayt) zaten var. Ne yapalım?"},
   {"Dillo Save: File exists!", "حفظ ديلو: الملف موجود!", "Dillo 保存：文件已存在！", "Dillo opslaan: bestand bestaat!", "Dillo Save: File exists!", "Enregistrement Dillo : le fichier existe !", "Dillo Speichern: Datei existiert!", "Salvataggio Dillo: file esistente!", "Dillo zapis: plik istnieje!", "Guardar Dillo: ficheiro existe!", "Сохранение Dillo: файл существует!", "Guardar Dillo: ¡el archivo existe!", "Dillo Kaydet: dosya var!"},
   {"Save Page as File", "احفظ الصفحة كملف", "将页面保存为文件", "Pagina opslaan als bestand", "Save Page as File", "Enregistrer la page comme fichier", "Seite als Datei speichern", "Salva pagina come file", "Zapisz stronę jako plik", "Guardar página como ficheiro", "Сохранить страницу как файл", "Guardar página como archivo", "Sayfayı dosya olarak kaydet"},
   {"Dillo: Select a File", "ديلو: اختر ملفاً", "Dillo：选择文件", "Dillo: Bestand selecteren", "Dillo: Select a File", "Dillo : sélectionner un fichier", "Dillo: Datei auswählen", "Dillo: seleziona un file", "Dillo: Wybierz plik", "Dillo: Selecionar ficheiro", "Dillo: выбрать файл", "Dillo: Seleccionar un archivo", "Dillo: Dosya seç"},
   {"Dillo: Open File", "ديلو: افتح ملفاً", "Dillo：打开文件", "Dillo: Bestand openen", "Dillo: Open File", "Dillo : ouvrir un fichier", "Dillo: Datei öffnen", "Dillo: apri file", "Dillo: Otwórz plik", "Dillo: Abrir ficheiro", "Dillo: открыть файл", "Dillo: Abrir archivo", "Dillo: Dosya aç"},
   {"Dillo: Search", "ديلو: بحث", "Dillo：搜索", "Dillo: Zoeken", "Dillo: Search", "Dillo : recherche", "Dillo: Suche", "Dillo: cerca", "Dillo: Szukaj", "Dillo: Pesquisa", "Dillo: поиск", "Dillo: Buscar", "Dillo: Ara"},
   {"Search the Web:", "ابحث في الويب:", "搜索网页：", "Zoeken op internet:", "Search the Web:", "Rechercher sur le Web :", "Im Web suchen:", "Cerca nel Web:", "Szukaj w sieci:", "Pesquisar na Web:", "Искать в сети:", "Buscar en la Web:", "Web’de ara:"},
   {"Dillo: Save Link as File", "ديلو: احفظ الرابط كملف", "Dillo：将链接保存为文件", "Dillo: Link opslaan als bestand", "Dillo: Save Link as File", "Dillo : enregistrer le lien comme fichier", "Dillo: Link als Datei speichern", "Dillo: salva link come file", "Dillo: Zapisz link jako plik", "Dillo: Guardar ligação como ficheiro", "Dillo: сохранить ссылку как файл", "Dillo: Guardar enlace como archivo", "Dillo: Bağlantıyı dosya olarak kaydet"},
   {"Dillo: Detected HTML errors", "ديلو: تم اكتشاف أخطاء HTML", "Dillo：检测到 HTML 错误", "Dillo: HTML-fouten gevonden", "Dillo: Detected HTML errors", "Dillo : erreurs HTML détectées", "Dillo: HTML-Fehler erkannt", "Dillo: errori HTML rilevati", "Dillo: Wykryto błędy HTML", "Dillo: Erros HTML detetados", "Dillo: обнаружены ошибки HTML", "Dillo: Errores HTML detectados", "Dillo: HTML hataları algılandı"},
   {"Dillo: Good HTML!", "ديلو: HTML صحيح!", "Dillo：HTML 良好！", "Dillo: Goede HTML!", "Dillo: Good HTML!", "Dillo : HTML correct !", "Dillo: Gutes HTML!", "Dillo: HTML corretto!", "Dillo: Poprawny HTML!", "Dillo: HTML correto!", "Dillo: хороший HTML!", "Dillo: ¡HTML correcto!", "Dillo: HTML iyi!"},
   {"No HTML errors found while parsing!", "لم يتم العثور على أخطاء HTML أثناء التحليل!", "解析时未发现 HTML 错误！", "Geen HTML-fouten gevonden tijdens het verwerken!", "No HTML errors found while parsing!", "Aucune erreur HTML trouvée pendant l’analyse !", "Beim Parsen wurden keine HTML-Fehler gefunden!", "Nessun errore HTML trovato durante l’analisi!", "Nie znaleziono błędów HTML podczas parsowania!", "Não foram encontrados erros HTML durante a análise!", "При разборе ошибки HTML не найдены!", "¡No se encontraron errores HTML al analizar!", "Ayrıştırma sırasında HTML hatası bulunmadı!"},
   {"Top reached; restarting from the bottom.", "تم الوصول إلى الأعلى؛ إعادة البدء من الأسفل.", "已到顶部；从底部重新开始。", "Bovenkant bereikt; opnieuw beginnen vanaf onderen.", "Top reached; restarting from the bottom.", "Haut atteint ; reprise depuis le bas.", "Oben angekommen; Neustart von unten.", "Raggiunta la cima; riparto dal basso.", "Osiągnięto górę; zaczynam od dołu.", "Topo atingido; a recomeçar a partir de baixo.", "Достигнут верх; начинаю снизу.", "Se alcanzó el inicio; se reinicia desde abajo.", "Üste ulaşıldı; alttan yeniden başlanıyor."},
   {"Bottom reached; restarting from the top.", "تم الوصول إلى الأسفل؛ إعادة البدء من الأعلى.", "已到底部；从顶部重新开始。", "Onderkant bereikt; opnieuw beginnen vanaf boven.", "Bottom reached; restarting from the top.", "Bas atteint ; reprise depuis le haut.", "Unten angekommen; Neustart von oben.", "Raggiunto il fondo; riparto dall’alto.", "Osiągnięto dół; zaczynam od góry.", "Fundo atingido; a recomeçar a partir de cima.", "Достигнут низ; начинаю сверху.", "Se alcanzó el final; se reinicia desde arriba.", "Alta ulaşıldı; üstten yeniden başlanıyor."},
   {"Dillo: Insecure form submission", "ديلو: إرسال نموذج غير آمن", "Dillo：不安全的表单提交", "Dillo: Onveilige formulierverzending", "Dillo: Insecure form submission", "Dillo : envoi de formulaire non sécurisé", "Dillo: Unsicheres Formularsenden", "Dillo: invio modulo non sicuro", "Dillo: Niebezpieczne wysyłanie formularza", "Dillo: Submissão de formulário insegura", "Dillo: небезопасная отправка формы", "Dillo: Envío de formulario inseguro", "Dillo: Güvensiz form gönderimi"},
   {"INSECURE protocol to submit data.", "بروتوكول غير آمن لإرسال البيانات.", "用于提交数据的协议不安全。", "ONVEILIG protocol om gegevens te verzenden.", "INSECURE protocol to submit data.", "Protocole NON SÉCURISÉ pour envoyer les données.", "UNSICHERES Protokoll zum Senden von Daten.", "Protocollo NON SICURO per inviare dati.", "NIEBEZPIECZNY protokół przesyłania danych.", "Protocolo INSEGURO para enviar dados.", "НЕБЕЗОПАСНЫЙ протокол для отправки данных.", "Protocolo INSEGURO para enviar datos.", "Veri göndermek için GÜVENSİZ protokol."},
   {"Loading file...", "تحميل الملف...", "正在加载文件...", "Bestand laden...", "Loading file...", "Chargement du fichier...", "Datei wird geladen...", "Caricamento file...", "Ładowanie pliku...", "A carregar ficheiro...", "Загрузка файла...", "Cargando archivo...", "Dosya yükleniyor..."},
   {"File loaded.", "تم تحميل الملف.", "文件已加载。", "Bestand geladen.", "File loaded.", "Fichier chargé.", "Datei geladen.", "File caricato.", "Plik załadowany.", "Ficheiro carregado.", "Файл загружен.", "Archivo cargado.", "Dosya yüklendi."},
   {"ERROR: can't load: %s", "خطأ: لا يمكن التحميل: %s", "错误：无法加载：%s", "FOUT: kan niet laden: %s", "ERROR: can't load: %s", "ERREUR : impossible de charger : %s", "FEHLER: Laden nicht möglich: %s", "ERRORE: impossibile caricare: %s", "BŁĄD: nie można załadować: %s", "ERRO: não é possível carregar: %s", "ОШИБКА: не удалось загрузить: %s", "ERROR: no se puede cargar: %s", "HATA: yüklenemiyor: %s"},
   {"File selector", "محدد الملفات", "文件选择器", "Bestandskiezer", "File selector", "Sélecteur de fichier", "Dateiauswahl", "Selettore file", "Wybór pliku", "Seletor de ficheiros", "Выбор файла", "Selector de archivos", "Dosya seçici"},
   {"Dillo: %s", "ديلو: %s", "Dillo：%s", "Dillo: %s", "Dillo: %s", "Dillo : %s", "Dillo: %s", "Dillo: %s", "Dillo: %s", "Dillo: %s", "Dillo: %s", "Dillo: %s", "Dillo: %s"},
   {"Go to the Home page\nMiddle-click for new tab.", "اذهب إلى الصفحة الرئيسية\nالنقر بالزر الأوسط يفتح لساناً جديداً.", "转到主页\n中键点击打开新标签页。", "Ga naar de startpagina\nMiddelklik voor een nieuw tabblad.", "Go to the Home page\nMiddle-click for new tab.", "Aller à la page d’accueil\nClic milieu pour un nouvel onglet.", "Zur Startseite gehen\nMittelklick für neuen Tab.", "Vai alla pagina iniziale\nClic centrale per una nuova scheda.", "Przejdź do strony domowej\nŚrodkowy przycisk otwiera nową kartę.", "Ir para a página inicial\nClique do meio para novo separador.", "Перейти на домашнюю страницу\nСредняя кнопка откроет новую вкладку.", "Ir a la página de inicio\nClic central para nueva pestaña.", "Ana sayfaya git\nOrta tık yeni sekme açar."},
   {"View bookmarks\nMiddle-click for new tab.", "اعرض العلامات\nالنقر بالزر الأوسط يفتح لساناً جديداً.", "查看书签\n中键点击打开新标签页。", "Bladwijzers bekijken\nMiddelklik voor een nieuw tabblad.", "View bookmarks\nMiddle-click for new tab.", "Afficher les signets\nClic milieu pour un nouvel onglet.", "Lesezeichen anzeigen\nMittelklick für neuen Tab.", "Visualizza segnalibri\nClic centrale per una nuova scheda.", "Pokaż zakładki\nŚrodkowy przycisk otwiera nową kartę.", "Ver favoritos\nClique do meio para novo separador.", "Показать закладки\nСредняя кнопка откроет новую вкладку.", "Ver marcadores\nClic central para nueva pestaña.", "Yer imlerini göster\nOrta tık yeni sekme açar."},
   {"Clear the URL box.\nMiddle-click to paste a URL.", "امسح حقل العنوان.\nالنقر بالزر الأوسط يلصق عنوان URL.", "清空 URL 输入框。\n中键点击粘贴 URL。", "URL-veld wissen.\nMiddelklik om een URL te plakken.", "Clear the URL box.\nMiddle-click to paste a URL.", "Effacer le champ URL.\nClic milieu pour coller une URL.", "URL-Feld leeren.\nMittelklick fügt eine URL ein.", "Cancella il campo URL.\nClic centrale per incollare un URL.", "Wyczyść pole URL.\nŚrodkowy przycisk wkleja URL.", "Limpar a caixa de URL.\nClique do meio para colar um URL.", "Очистить поле URL.\nСредняя кнопка вставляет URL.", "Borrar el campo URL.\nClic central para pegar una URL.", "URL kutusunu temizle.\nOrta tık URL yapıştırır."},
   {"Page\n0.0 KB", "صفحة\n0.0 كيلوبايت", "页面\n0.0 KB", "Pagina\n0,0 KB", "Page\n0.0 KB", "Page\n0,0 Ko", "Seite\n0,0 KB", "Pagina\n0,0 KB", "Strona\n0,0 KB", "Página\n0,0 KB", "Страница\n0,0 КБ", "Página\n0,0 KB", "Sayfa\n0,0 KB"},
   {"Images\n0 of 0", "صور\n0 من 0", "图像\n0/0", "Afbeeldingen\n0 van 0", "Images\n0 of 0", "Images\n0 sur 0", "Bilder\n0 von 0", "Immagini\n0 di 0", "Obrazy\n0 z 0", "Imagens\n0 de 0", "Изображения\n0 из 0", "Imágenes\n0 de 0", "Görseller\n0 / 0"},
   {"A form on a SECURE page wants to use an INSECURE protocol to submit data.", "يريد نموذج في صفحة آمنة استخدام بروتوكول غير آمن لإرسال البيانات.", "安全页面上的表单想使用不安全的协议提交数据。", "Een formulier op een VEILIGE pagina wil een ONVEILIG protocol gebruiken om gegevens te verzenden.", "A form on a SECURE page wants to use an INSECURE protocol to submit data.", "Un formulaire sur une page SÉCURISÉE veut utiliser un protocole NON SÉCURISÉ pour envoyer des données.", "Ein Formular auf einer SICHEREN Seite will ein UNSICHERES Protokoll zum Senden von Daten verwenden.", "Un modulo su una pagina SICURA vuole usare un protocollo NON SICURO per inviare dati.", "Formularz na BEZPIECZNEJ stronie chce użyć NIEBEZPIECZNEGO protokołu do wysłania danych.", "Um formulário numa página SEGURA quer usar um protocolo INSEGURO para enviar dados.", "Форма на БЕЗОПАСНОЙ странице хочет использовать НЕБЕЗОПАСНЫЙ протокол для отправки данных.", "Un formulario en una página SEGURA quiere usar un protocolo INSEGURO para enviar datos.", "GÜVENLİ bir sayfadaki form veri göndermek için GÜVENSİZ bir protokol kullanmak istiyor."},
};

int a_I18n_use_cjk_font(void)
{
   return strcmp(app_lang, "zh_CN") == 0;
}

Fl_Font a_I18n_ui_font(Fl_Font fallback)
{
   return a_I18n_use_cjk_font() ? DILLO_CJK_FONT : fallback;
}

void a_I18n_apply_fonts(void)
{
   if (!a_I18n_use_cjk_font())
      return;

   Fl::set_fonts("*");
   Fl::set_font(DILLO_CJK_FONT, app_cjk_font_name);
}

const char *a_I18n_tr(const char *key)
{
   if (!key)
      return key;
   for (size_t i = 0; i < sizeof(translations) / sizeof(translations[0]); ++i) {
      if (strcmp(translations[i].key, key) == 0) {
         if (strcmp(app_lang, "ar") == 0) return translations[i].ar;
         if (strcmp(app_lang, "zh_CN") == 0) return translations[i].zh_CN;
         if (strcmp(app_lang, "nl") == 0) return translations[i].nl;
         if (strcmp(app_lang, "fr") == 0) return translations[i].fr;
         if (strcmp(app_lang, "de") == 0) return translations[i].de;
         if (strcmp(app_lang, "it") == 0) return translations[i].it;
         if (strcmp(app_lang, "pl") == 0) return translations[i].pl;
         if (strcmp(app_lang, "pt") == 0) return translations[i].pt;
         if (strcmp(app_lang, "ru") == 0) return translations[i].ru;
         if (strcmp(app_lang, "es") == 0) return translations[i].es;
         if (strcmp(app_lang, "tr") == 0) return translations[i].tr;
         return translations[i].en;
      }
   }
   return key;
}

void a_I18n_init(void)
{
   app_lang = detect_app_language();
   load_string_resources();
   fl_ok = a_I18n_tr("OK");
   fl_cancel = a_I18n_tr("Cancel");
   fl_close = a_I18n_tr("Close");
   Fl_File_Chooser::preview_label = a_I18n_tr("Preview");
   Fl_File_Chooser::new_directory_label = a_I18n_tr("New Directory?");
   Fl_File_Chooser::new_directory_tooltip = a_I18n_tr("Create a new directory.");
   Fl_File_Chooser::hidden_label = a_I18n_tr("Show hidden files");
   Fl_File_Chooser::show_label = a_I18n_tr("Show:");
   Fl_File_Chooser::filename_label = a_I18n_tr("Filename:");
   Fl_File_Chooser::all_files_label = a_I18n_tr("All Files (*)");
   Fl_File_Chooser::custom_filter_label = a_I18n_tr("Custom Filter");
   Fl_File_Chooser::favorites_label = a_I18n_tr("Favorites");
   Fl_File_Chooser::add_favorites_label = a_I18n_tr("Add to Favorites");
   Fl_File_Chooser::manage_favorites_label = a_I18n_tr("Manage Favorites");
   Fl_File_Chooser::filesystems_label = a_I18n_tr("File Systems");
   Fl_File_Chooser::existing_file_label = a_I18n_tr("Please choose an existing file!");
}
