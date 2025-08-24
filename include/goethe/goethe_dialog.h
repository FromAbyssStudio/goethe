#ifndef GOETHE_DIALOG_H
#define GOETHE_DIALOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GoetheDialog GoetheDialog;
typedef struct GoetheDialogLine GoetheDialogLine;

// Dialog line structure (C-compatible)
typedef struct GoetheDialogLine {
    const char* character;
    const char* phrase;
    const char* direction;
    const char* expression;
    const char* mood;
    float time;
} GoetheDialogLine;

// Dialog structure (C-compatible)
typedef struct GoetheDialog {
    const char* dialogue_id;
    const char* title;
    const char* mode;
    float default_time;
    GoetheDialogLine* lines;
    int line_count;
} GoetheDialog;

// Dialog API functions
GoetheDialog* goethe_dialog_create(void);
void goethe_dialog_destroy(GoetheDialog* dialog);

// Load dialog from YAML file/stream
int goethe_dialog_load_from_file(GoetheDialog* dialog, const char* filepath);
int goethe_dialog_load_from_yaml(GoetheDialog* dialog, const char* yaml_string);

// Save dialog to YAML file/stream
int goethe_dialog_save_to_file(const GoetheDialog* dialog, const char* filepath);
char* goethe_dialog_save_to_yaml(const GoetheDialog* dialog);

// Dialog manipulation
int goethe_dialog_add_line(GoetheDialog* dialog, const GoetheDialogLine* line);
int goethe_dialog_remove_line(GoetheDialog* dialog, int line_index);
GoetheDialogLine* goethe_dialog_get_line(const GoetheDialog* dialog, int line_index);

// Utility functions
int goethe_dialog_get_line_count(const GoetheDialog* dialog);
const char* goethe_dialog_get_id(const GoetheDialog* dialog);
const char* goethe_dialog_get_title(const GoetheDialog* dialog);
const char* goethe_dialog_get_mode(const GoetheDialog* dialog);
float goethe_dialog_get_default_time(const GoetheDialog* dialog);

#ifdef __cplusplus
}
#endif

#endif // GOETHE_DIALOG_H
