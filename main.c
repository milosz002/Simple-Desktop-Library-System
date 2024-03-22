#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <gtk/gtk.h>

#define ZA_MALO_PAMIECI 0


typedef struct wezel {
  char *tytul;
  char *autor;
  char *ilosc;
  char *cena;

  int pozycja;

  //int ilosc;
  //double cena;

  struct wezel *nastepny;
  struct wezel *wczesniejszy;
} ksiazka;


struct save_edit_data{
    ksiazka *element;
    GtkWidget *EditWindow;
    GtkWidget *tytul;
    GtkWidget *autor;
    GtkWidget *ilosc;
    GtkWidget *cena;
};

struct add_row_data{
    GtkWidget *AddWindow;
    GtkWidget *tytul;
    GtkWidget *autor;
    GtkWidget *ilosc;
    GtkWidget *cena;
};

struct delete_yes_data{
    ksiazka *element;
    GtkWidget *DeleteWindow;
};

// globalne
ksiazka *magazyn = NULL;
GtkWidget *main_window; // glowne okno
GtkWidget *main_stack; // glowny stack
GtkWidget *listitems_grid; // grid od magazynu
int last_row = 1;
char *columns_names[4];
bool delete_checkbox = 0;
char *stack_names[] = {"load_file", "magazyn", "save", "exit"};

GtkListBoxRow *last_selected_list_box_row = NULL;


void my_split(const char *linia, char *tytul, char *autor, char *ilosc, char *cena, char sep){
	// tytul
	while(*linia != sep){
		*tytul++ = *linia++;
	}
	*linia++; // pomijanie srednika
	*tytul++ = '\0';
	// autor
	while(*linia != sep){
		*autor++ = *linia++;
	}
	*linia++; // pomijanie srednika
	*autor++ = '\0';

	// ilosc
	while(*linia != sep){
		*ilosc++ = *linia++;
	}
	*linia++; // pomijanie srednika
	*ilosc++ = '\0';

	// cena
	while(*linia != sep){
		*cena++ = *linia++;
	}
	*cena++ = '\0';
	// koniec
}

bool check_line(const char *linia){
    int count = 0;
    while(*linia != '\0'){
        if(*linia == ';'){
            count++;
        }
        *linia++;
    }
    if (count == 4){
        return 1;
    }
    else{
        return 0;
    }
}


void push_back(ksiazka **pierwszy, ksiazka *element) {//wstaw element na koniec listy
  // konstruujemy nowy wezel
  ksiazka* iterator = *pierwszy;

    if (iterator == NULL){ //gdy pusta lista
        *pierwszy = element;
        return;
    }

    while (iterator->nastepny != NULL){
        iterator = iterator->nastepny;
    }

    element->wczesniejszy = iterator;
    iterator->nastepny = element;

}


bool wypelnijMagazyn(const char *nazwaPliku, ksiazka **pierwszy){
    FILE *fp = fopen (nazwaPliku, "r");
    if (fp == NULL) {
       printf("Brak pliku z danymi\n");
       return 0;
    }
    const int max_n= 500;
    char linia[500], *result;
    char tytul[200];
    char autor[100];
    char cena[20];
    char ilosc[10];
    char *frag;

    while(!feof(fp)){ // czytanie do konca pliku

       ksiazka *element;
       if ((element = (ksiazka*)malloc(sizeof(ksiazka))) == NULL ) {
          fprintf(stderr, "Za malo pamieci!\n");
          exit(ZA_MALO_PAMIECI);
       }
       result = fgets (linia, max_n, fp);

       tytul[0] = '\0';
       autor[0] = '\0';
       cena[0] = '\0';
       ilosc[0] = '\0';

       if(check_line(linia) == 0){
            return 0;
       }

       my_split(linia, tytul, autor, ilosc, cena, ';');
       if (tytul[0] == '\0' || autor[0] == '\0' || cena[0] == '\0' || ilosc[0] == '\0'){
            return 0;
       }
       else{
            element->tytul = (char*) malloc((strlen(tytul) + 1) * sizeof(char));
            strcpy(element->tytul, tytul);

            element->autor = (char*) malloc((strlen(autor) + 1) * sizeof(char));
            strcpy(element->autor, autor);


            element->ilosc = (char*) malloc((strlen(ilosc) + 1) * sizeof(char));
            strcpy(element->ilosc, ilosc);

            element->cena = (char*) malloc((strlen(cena) + 1) * sizeof(char));
            strcpy(element->cena, cena);

            element->nastepny = NULL;

            element->wczesniejszy = NULL;

            element->pozycja = NULL;

            push_back(pierwszy, element);
       }

    }
    fclose(fp);
    return 1;
}


// ---------------------------------------------------------------------------------------------------

void message_window(char* message){
    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void set_columns_names(){
    columns_names[0] = "Tytul";
    columns_names[1] = "Autor";
    columns_names[2] = "Ilosc";
    columns_names[3] = "Cena";
}

void myCSS(void){
    GtkCssProvider *provider;
    GdkDisplay *display;
    GdkScreen *screen;

    provider = gtk_css_provider_new ();
    display = gdk_display_get_default ();
    screen = gdk_display_get_default_screen (display);
    gtk_style_context_add_provider_for_screen (screen, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    const gchar *myCssFile = "mystyle.css";
    GError *error = 0;

    gtk_css_provider_load_from_file(provider, g_file_new_for_path(myCssFile), &error);
    g_object_unref (provider);
}

static void load_file (GtkWidget *widget, gpointer data){
  GtkWidget *loadfile_label = GTK_WIDGET(data);
  GtkWidget *dialog = gtk_file_chooser_dialog_new("Wybierz plik",
                                                    NULL,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        gtk_widget_destroy(dialog); // zamykanie okna wyboru pliku

        char* f1 = "Wybrano plik: ";
        char* newlabeltext = filename;
        newlabeltext = malloc(strlen(f1)+1+4 + strlen(filename));
        strcpy(newlabeltext, f1);
        strcat(newlabeltext, filename);

        gtk_label_set_label(GTK_LABEL(loadfile_label), newlabeltext); // update label
        free(newlabeltext);

        // ------------------- wypelnianie magazynu
        magazyn = NULL;
        bool sukces = wypelnijMagazyn(filename, &magazyn);
        if(sukces){
            message_window("Pomyslnie wczytano plik.");
        }
        else{
            message_window("Nie udalo sie wczytac pliku.");
        }

        g_free(filename);
    }
    else{
        gtk_label_set_label(GTK_LABEL(loadfile_label), "Nie wybrano pliku");
        gtk_widget_destroy(dialog);
    }
}

static void save_file (GtkWidget *widget, gpointer data){

    GtkWidget *save_label = GTK_WIDGET(data);
    ksiazka* iterator = magazyn;

    if(iterator == NULL){
        gtk_label_set_label(GTK_LABEL(save_label), "Magazyn jest pusty, brak danych do zapisania.");
        message_window("Nie mozna zapisac pustego magazynu.");
    }
    else{
        GtkWidget *dialog;
        //dialog = gtk_file_chooser_dialog_new("Wybierz katalog", GTK_WINDOW(data), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "Anuluj", GTK_RESPONSE_CANCEL, "Wybierz", GTK_RESPONSE_ACCEPT, NULL);

         dialog = gtk_file_chooser_dialog_new("Zapisz plik jako", GTK_WINDOW(data),
                                             GTK_FILE_CHOOSER_ACTION_SAVE,
                                             "Anuluj", GTK_RESPONSE_CANCEL,
                                             "Zapisz", GTK_RESPONSE_ACCEPT,
                                             NULL);

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            char *filename;
            GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
            filename = gtk_file_chooser_get_filename(chooser);


            FILE *f = fopen(filename, "w");
            if (f == NULL)
            {
                gtk_label_set_label(GTK_LABEL(save_label), "Nie udalo sie zapisac pliku.");
            }
            else{
                // zapisywanie do pliku
                while(iterator->nastepny != NULL){
                    fprintf(f, "%s%s%s%s%s%s%s%s\n", iterator->tytul, ";", iterator->autor, ";", iterator->ilosc, ";", iterator->cena, ";");
                    iterator = iterator->nastepny;
                }
                fprintf(f, "%s%s%s%s%s%s%s%s", iterator->tytul, ";", iterator->autor, ";", iterator->ilosc, ";", iterator->cena, ";"); // ostatni element bez '\n'

                char* f1 = "Pomyslnie zapisano plik: ";
                char* newlabeltext = filename;
                newlabeltext = malloc(strlen(f1)+1+4 + strlen(filename));
                strcpy(newlabeltext, f1);
                strcat(newlabeltext, filename);
                gtk_label_set_label(GTK_LABEL(save_label), newlabeltext); // update label
                free(newlabeltext);
            }

            fclose(f);
            g_free(filename);
        }
        else{
            gtk_label_set_label(GTK_LABEL(save_label), "Nie wybrano lokalizacji");
        }

        gtk_widget_destroy(dialog); // zamykanie okna wyboru pliku
    }



}
void edit_save(GtkWidget *widget, gpointer user_data){
    struct save_edit_data *data = user_data;
    ksiazka* element = data->element;

    const gchar *edited_tytul = gtk_entry_get_text(GTK_ENTRY(data->tytul));
    const gchar *edited_autor = gtk_entry_get_text(GTK_ENTRY(data->autor));
    const gchar *edited_ilosc = gtk_entry_get_text(GTK_ENTRY(data->ilosc));
    const gchar *edited_cena = gtk_entry_get_text(GTK_ENTRY(data->cena));

    element->tytul = (gchar*) malloc((strlen(edited_tytul) + 1) * sizeof(gchar));
    strcpy(element->tytul, edited_tytul);

    element->autor = (gchar*) malloc((strlen(edited_autor) + 1) * sizeof(gchar));
    strcpy(element->autor, edited_autor);

    element->ilosc = (gchar*) malloc((strlen(edited_ilosc) + 1) * sizeof(gchar));
    strcpy(element->ilosc, edited_ilosc);

    element->cena = (gchar*) malloc((strlen(edited_cena) + 1) * sizeof(gchar));
    strcpy(element->cena, edited_cena);


    int grid_row = element->pozycja;
    gtk_label_set_text(GTK_LABEL(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 0, grid_row)), edited_tytul);
    gtk_label_set_text(GTK_LABEL(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 1, grid_row)), edited_autor);
    gtk_label_set_text(GTK_LABEL(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 2, grid_row)), edited_ilosc);
    gtk_label_set_text(GTK_LABEL(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 3, grid_row)), edited_cena);


    gtk_widget_destroy(GTK_WIDGET(data->EditWindow));

    free(user_data);
}

void on_grid_button_edit(GtkWidget *widget, ksiazka *data){
    ksiazka* element = data;


    GtkWidget *edit_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(edit_window), "Edytuj");
    gtk_window_set_default_size(GTK_WINDOW(edit_window), 20, 20);
    gtk_window_set_resizable (GTK_WINDOW(edit_window), FALSE);
    gtk_window_set_modal(GTK_WINDOW(edit_window), TRUE);

    // ------------------------- GRID

    GtkWidget *edit_grid = gtk_grid_new();
    GtkStyleContext *context_grid = gtk_widget_get_style_context(edit_grid);
    gtk_style_context_add_class(context_grid, "editgrid");
    gtk_grid_set_row_spacing(GTK_GRID(edit_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(edit_grid), 10);


    // dodawanie nazw kolumn
    for (int i = 0; i < 4; i++){
        GtkWidget *columnname = gtk_label_new(columns_names[i]);
        GtkStyleContext *context_grid_label_title = gtk_widget_get_style_context(columnname);
        gtk_style_context_add_class(context_grid_label_title, "grid_label_title");
        gtk_grid_attach(GTK_GRID(edit_grid), columnname, i, 0, 1, 1);
    }

    GtkWidget *entry_tytul = gtk_entry_new();
    GtkStyleContext *context_entry_tytul = gtk_widget_get_style_context(entry_tytul);
    gtk_style_context_add_class(context_entry_tytul, "add_edit_entry");
    gint tytul_len = g_utf8_strlen(element->tytul, -1);
    gtk_entry_set_width_chars(GTK_ENTRY(entry_tytul), tytul_len);
    gtk_entry_set_text(GTK_ENTRY(entry_tytul), element->tytul);
    gtk_entry_set_max_length(GTK_ENTRY(entry_tytul), 100);

    gtk_grid_attach(GTK_GRID(edit_grid), entry_tytul, 0, 1, 1, 1);


    GtkWidget *entry_autor = gtk_entry_new();
    GtkStyleContext *context_entry_autor = gtk_widget_get_style_context(entry_autor);
    gtk_style_context_add_class(context_entry_autor, "add_edit_entry");
    gint autor_len = g_utf8_strlen(element->autor, -1);
    gtk_entry_set_width_chars(GTK_ENTRY(entry_autor), autor_len);
    gtk_entry_set_text(GTK_ENTRY(entry_autor), element->autor);
    gtk_entry_set_max_length(GTK_ENTRY(entry_autor), 50);

    gtk_grid_attach(GTK_GRID(edit_grid), entry_autor, 1, 1, 1, 1);


    GtkWidget *entry_ilosc = gtk_entry_new();
    GtkStyleContext *context_entry_ilosc = gtk_widget_get_style_context(entry_ilosc);
    gtk_style_context_add_class(context_entry_ilosc, "add_edit_entry");
    gint ilosc_len = g_utf8_strlen(element->ilosc, -1);
    gtk_entry_set_width_chars(GTK_ENTRY(entry_ilosc), ilosc_len);
    gtk_entry_set_text(GTK_ENTRY(entry_ilosc), element->ilosc);
    gtk_entry_set_max_length(GTK_ENTRY(entry_ilosc), 10);

    gtk_grid_attach(GTK_GRID(edit_grid), entry_ilosc, 2, 1, 1, 1);


    GtkWidget *entry_cena = gtk_entry_new();
    GtkStyleContext *context_entry_cena= gtk_widget_get_style_context(entry_cena);
    gtk_style_context_add_class(context_entry_cena, "add_edit_entry");
    gint cena_len = g_utf8_strlen(element->cena, -1);
    gtk_entry_set_width_chars(GTK_ENTRY(entry_cena), cena_len);
    gtk_entry_set_text(GTK_ENTRY(entry_cena), element->cena);
    gtk_entry_set_max_length(GTK_ENTRY(entry_cena), 10);

    gtk_grid_attach(GTK_GRID(edit_grid), entry_cena, 3, 1, 1, 1);


    GtkWidget *grid_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(grid_box), edit_grid, FALSE, FALSE, 0);
    gtk_box_set_center_widget(GTK_BOX(grid_box), edit_grid);

    // ---------------------------------------- BUTTONS
    GtkWidget *save_button = gtk_button_new_with_label("Zapisz");
    gtk_widget_set_size_request(save_button, 10, 10);
    GtkStyleContext *context_button = gtk_widget_get_style_context(save_button);
    gtk_style_context_add_class(context_button, "edit_green_button");

    struct save_edit_data *data_send = (struct save_edit_data*)malloc(sizeof(struct save_edit_data));
    data_send->element = element;
    data_send->tytul = entry_tytul;
    data_send->autor = entry_autor;
    data_send->ilosc = entry_ilosc;
    data_send->cena = entry_cena;
    data_send->EditWindow = edit_window;

    g_signal_connect (save_button, "clicked", G_CALLBACK (edit_save), data_send);



    GtkWidget *buttons_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(buttons_box), save_button, FALSE, FALSE, 0);
    gtk_box_set_center_widget(GTK_BOX(buttons_box), save_button);



    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkStyleContext *context_main_box = gtk_widget_get_style_context(main_box);
    gtk_style_context_add_class(context_main_box, "black_background");
    gtk_box_pack_start(GTK_BOX(main_box), grid_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), buttons_box, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(edit_window), main_box);


    gtk_widget_show_all(edit_window);
}

void on_checkbox_toggled(){
    delete_checkbox = !delete_checkbox;
}

void on_delete_no_button(GtkButton *button, gpointer data){
    GtkWidget *delete_window = data;
    gtk_widget_destroy(GTK_WIDGET(delete_window));
}

void hide_grid_row(int row_number){
    gtk_widget_set_visible(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 0, row_number), FALSE); // tytul
    gtk_widget_set_visible(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 1, row_number), FALSE); // autor
    gtk_widget_set_visible(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 2, row_number), FALSE); // ilosc
    gtk_widget_set_visible(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 3, row_number), FALSE); // cena
    gtk_widget_set_visible(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 4, row_number), FALSE); // edit button
    gtk_widget_set_visible(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 5, row_number), FALSE); // delete button
}

void show_grid_row(int row_number){
    gtk_widget_set_visible(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 0, row_number), TRUE); // tytul
    gtk_widget_set_visible(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 1, row_number), TRUE); // autor
    gtk_widget_set_visible(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 2, row_number), TRUE); // ilosc
    gtk_widget_set_visible(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 3, row_number), TRUE); // cena
    gtk_widget_set_visible(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 4, row_number), TRUE); // edit button
    gtk_widget_set_visible(gtk_grid_get_child_at(GTK_GRID(listitems_grid), 5, row_number), TRUE); // delete button
}

void on_delete_yes_button(GtkButton *button, gpointer data){
    struct delete_yes_data *user_data = data;
    ksiazka* element = user_data->element;


    if(element->nastepny == NULL && element->wczesniejszy == NULL){
        // jedyny element w magazynie
        magazyn = NULL;

    }
    else if(element->wczesniejszy == NULL){
        // pierwszy element
        element->nastepny->wczesniejszy = NULL;
        magazyn = element->nastepny;
    }
    else if (element->nastepny == NULL){
        // ostatni element
        element->wczesniejszy->nastepny=NULL;
    }
    else{
        // jakis element posrodku
        element->nastepny->wczesniejszy = element->wczesniejszy;
        element->wczesniejszy->nastepny = element->nastepny;
    }

    hide_grid_row(element->pozycja); // ukrywanie tego wiersza

    if(user_data->DeleteWindow != NULL){
        gtk_widget_destroy(GTK_WIDGET(user_data->DeleteWindow)); // niszczenie okna z usuwaniem
    }

    free(data);

}

void on_grid_button_delete(GtkWidget *widget, ksiazka *data){
    ksiazka* element = data;

    if(delete_checkbox==0){
        GtkWidget *delete_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(delete_window), "Usun");
        gtk_window_set_default_size(GTK_WINDOW(delete_window), 20, 20);
        gtk_window_set_resizable (GTK_WINDOW(delete_window), FALSE);
        gtk_window_set_modal(GTK_WINDOW(delete_window), TRUE);

        // ----------------------------------- question_label

        GtkWidget *question_label = gtk_label_new("Czy na pewno chcesz usunac ten element?");
        GtkStyleContext *context_label = gtk_widget_get_style_context(question_label);
        gtk_style_context_add_class(context_label, "delete_question_label");

        GtkWidget *checkbox = gtk_check_button_new_with_label("Nie pytaj ponownie");
        GtkStyleContext *context_checkbox = gtk_widget_get_style_context(checkbox);
        gtk_style_context_add_class(context_checkbox, "delete_checkbox");
        g_signal_connect(checkbox, "toggled", G_CALLBACK(on_checkbox_toggled), NULL);


        GtkWidget *label_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_pack_start(GTK_BOX(label_box), question_label, FALSE, FALSE, 0);
        gtk_box_set_center_widget(GTK_BOX(label_box), question_label);

        gtk_box_pack_start(GTK_BOX(label_box), checkbox, FALSE, FALSE, 0);
        gtk_box_set_center_widget(GTK_BOX(label_box), checkbox);

        // ----------------------------------- buttons

        GtkWidget *yes_button = gtk_button_new_with_label("Tak");
        gtk_widget_set_size_request(yes_button, 200, 30);
        GtkStyleContext *context_yes_button = gtk_widget_get_style_context(yes_button);
        gtk_style_context_add_class(context_yes_button, "green_button");
        struct delete_yes_data *data_send = (struct delete_yes_data*)malloc(sizeof(struct delete_yes_data));
        data_send->element = element;
        data_send->DeleteWindow = delete_window;

        g_signal_connect(yes_button, "clicked", G_CALLBACK(on_delete_yes_button), data_send);


        GtkWidget *no_button = gtk_button_new_with_label("Nie");
        gtk_widget_set_size_request(no_button, 200, 30);
        GtkStyleContext *context_no_button = gtk_widget_get_style_context(no_button);
        gtk_style_context_add_class(context_no_button, "red_button");
        g_signal_connect(no_button, "clicked", G_CALLBACK(on_delete_no_button), delete_window);



        GtkWidget *buttons_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

        gtk_box_pack_start(GTK_BOX(buttons_box), yes_button, FALSE, FALSE, 0);
        gtk_box_set_center_widget(GTK_BOX(buttons_box), yes_button);

        gtk_box_pack_start(GTK_BOX(buttons_box), no_button, FALSE, FALSE, 0);
        gtk_box_set_center_widget(GTK_BOX(buttons_box), no_button);




        GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        GtkStyleContext *context_main_box = gtk_widget_get_style_context(main_box);
        gtk_style_context_add_class(context_main_box, "black_background");

        gtk_container_add(GTK_BOX(main_box), label_box);
        gtk_container_add(GTK_BOX(main_box), buttons_box);

        gtk_container_add(GTK_CONTAINER(delete_window), main_box);
        gtk_widget_show_all(delete_window);
    }
    else{
        // zaznaczone zeby nie pytac ponownie "Czy na pewno chcesz usunac ten element?"
        struct delete_yes_data *data_send = (struct delete_yes_data*)malloc(sizeof(struct delete_yes_data));
        data_send->element = element;
        data_send->DeleteWindow = NULL;

        on_delete_yes_button(NULL, data_send);

    }

}


void add_title_row(){
    for (int i = 0; i < 4; i++){
        GtkWidget *columnname = gtk_label_new(columns_names[i]);
        GtkStyleContext *context_grid_label_title = gtk_widget_get_style_context(columnname);
        gtk_style_context_add_class(context_grid_label_title, "grid_label_title");

        gtk_grid_attach(GTK_GRID(listitems_grid), columnname, i, 0, 1, 1);
    }
}

static void load_magazyn (GtkWidget *widget, gpointer data){
    if (magazyn != NULL){
        // czyszczenie grida
        gtk_container_foreach(GTK_CONTAINER(listitems_grid), (GtkCallback) gtk_widget_destroy, NULL);

        // ladowanie danych z magazynu do grida
        ksiazka* iterator = magazyn;

        // dodawanie nazw kolumn
        add_title_row();

        // dodawanie do grida danych z magazynu
        int row = 1;
        while (iterator != NULL){
            GtkWidget *new_labels[4];
        	new_labels[0] = gtk_label_new(iterator->tytul);
        	new_labels[1] = gtk_label_new(iterator->autor);
        	new_labels[2] = gtk_label_new(iterator->ilosc);
        	new_labels[3] = gtk_label_new(iterator->cena);
            for(int i = 0; i < 4; i++){
                GtkStyleContext *context_grid_label = gtk_widget_get_style_context(new_labels[i]);
                gtk_style_context_add_class(context_grid_label, "grid_label");

                gtk_grid_attach(GTK_GRID(listitems_grid), new_labels[i], i, row, 1, 1);
            }

            iterator->pozycja = row;

            GtkWidget *button_edit =  gtk_button_new_from_icon_name("edit-paste", GTK_ICON_SIZE_BUTTON);
            GtkStyleContext *context_edit_button = gtk_widget_get_style_context(button_edit);
            gtk_style_context_add_class(context_edit_button, "magazyn_grid_button");
            g_signal_connect (button_edit, "clicked", G_CALLBACK (on_grid_button_edit), iterator);
            gtk_grid_attach(GTK_GRID(listitems_grid), button_edit, 4, row, 1, 1);

            GtkWidget *button_delete =  gtk_button_new_from_icon_name("edit-delete", GTK_ICON_SIZE_BUTTON);
            GtkStyleContext *context_delete_button = gtk_widget_get_style_context(button_delete);
            gtk_style_context_add_class(context_delete_button, "magazyn_grid_button");
            g_signal_connect (button_delete, "clicked", G_CALLBACK (on_grid_button_delete), iterator);
            gtk_grid_attach(GTK_GRID(listitems_grid), button_delete, 5, row, 1, 1);


        	row++;
        	iterator = iterator->nastepny;
    	}
    	gtk_widget_show_all(listitems_grid); // zeby pokazalo zaktualizowanego grida
    	last_row = row;

    }
    else{
        // komunikat
        message_window("Magazyn jest pusty! Najpierw zaladuj plik.");
    }
}

void add_new_row(GtkWidget *widget, gpointer user_data){
    struct add_row_data *data = user_data;

    // tworzenie nowego elementu
    ksiazka *element;

    if ((element = (ksiazka*)malloc(sizeof(ksiazka))) == NULL ) {
          fprintf(stderr, "Za malo pamieci!\n");
          exit(ZA_MALO_PAMIECI);
    }

    const gchar *added_tytul = gtk_entry_get_text(GTK_ENTRY(data->tytul));
    const gchar *added_autor = gtk_entry_get_text(GTK_ENTRY(data->autor));
    const gchar *added_ilosc = gtk_entry_get_text(GTK_ENTRY(data->ilosc));
    const gchar *added_cena = gtk_entry_get_text(GTK_ENTRY(data->cena));

    element->tytul = (gchar*) malloc((strlen(added_tytul) + 1) * sizeof(gchar));
    strcpy(element->tytul, added_tytul);

    element->autor = (gchar*) malloc((strlen(added_autor) + 1) * sizeof(gchar));
    strcpy(element->autor, added_autor);

    element->ilosc = (gchar*) malloc((strlen(added_ilosc) + 1) * sizeof(gchar));
    strcpy(element->ilosc, added_ilosc);

    element->cena = (gchar*) malloc((strlen(added_cena) + 1) * sizeof(gchar));
    strcpy(element->cena, added_cena);

    element->nastepny = NULL;

    element->wczesniejszy = NULL;

    element->pozycja = NULL;


    // dodawanie nowego elementu do magazynu
    ksiazka *iterator = magazyn;

    if(iterator == NULL){ //gdy magazyn jest pusty
        element->pozycja = last_row;
        magazyn = element;
    }
    else{
        while (iterator->nastepny != NULL){
            iterator = iterator->nastepny;
        }

        element->pozycja = last_row;
        element->wczesniejszy = iterator;
        iterator->nastepny = element;
    }

    load_magazyn(NULL, NULL); // przeładowanie magazynu zeby pokazalo dodany element


    last_row++; // wyznaczanie ostatniego pustego wiersza

    gtk_widget_destroy(GTK_WIDGET(data->AddWindow));

    free(user_data);
}

void add_row_cancel(GtkButton *button, gpointer data){
    GtkWidget *add_window = data;
    gtk_widget_destroy(GTK_WIDGET(add_window));
}

void on_add_row(){
    // dodawanie nowego elementu/wiersza do magazynu
    GtkWidget *add_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(add_window), "Dodaj wiersz");
    gtk_window_set_default_size(GTK_WINDOW(add_window), 20, 20);
    gtk_window_set_resizable (GTK_WINDOW(add_window), FALSE);
    gtk_window_set_modal(GTK_WINDOW(add_window), TRUE);

    // ------------------------- GRID

    GtkWidget *add_grid = gtk_grid_new();
    GtkStyleContext *context_grid = gtk_widget_get_style_context(add_grid);
    gtk_style_context_add_class(context_grid, "editgrid");
    gtk_grid_set_row_spacing(GTK_GRID(add_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(add_grid), 10);


    // dodawanie nazw kolumn
    for (int i = 0; i < 4; i++){
        GtkWidget *columnname = gtk_label_new(columns_names[i]);
        GtkStyleContext *context_grid_label_title = gtk_widget_get_style_context(columnname);
        gtk_style_context_add_class(context_grid_label_title, "grid_label_title");

        gtk_grid_attach(GTK_GRID(add_grid), columnname, i, 0, 1, 1);
    }

    GtkWidget *entry_tytul = gtk_entry_new();
    GtkStyleContext *context_entry_tytul = gtk_widget_get_style_context(entry_tytul);
    gtk_style_context_add_class(context_entry_tytul, "add_edit_entry");
    gtk_widget_set_size_request(GTK_ENTRY(entry_tytul), 150, 30);
    gtk_entry_set_text(GTK_ENTRY(entry_tytul), "");
    gtk_entry_set_max_length(GTK_ENTRY(entry_tytul), 100);

    gtk_grid_attach(GTK_GRID(add_grid), entry_tytul, 0, 1, 1, 1);


    GtkWidget *entry_autor = gtk_entry_new();
    GtkStyleContext *context_entry_autor = gtk_widget_get_style_context(entry_autor);
    gtk_style_context_add_class(context_entry_autor, "add_edit_entry");
    gtk_widget_set_size_request(GTK_ENTRY(entry_autor), 150, 30);
    gtk_entry_set_text(GTK_ENTRY(entry_autor), "");
    gtk_entry_set_max_length(GTK_ENTRY(entry_autor), 50);

    gtk_grid_attach(GTK_GRID(add_grid), entry_autor, 1, 1, 1, 1);


    GtkWidget *entry_ilosc = gtk_entry_new();
    GtkStyleContext *context_entry_ilosc = gtk_widget_get_style_context(entry_ilosc);
    gtk_style_context_add_class(context_entry_ilosc, "add_edit_entry");
    gtk_widget_set_size_request(GTK_ENTRY(entry_ilosc), 150, 30);
    gtk_entry_set_text(GTK_ENTRY(entry_ilosc), "");
    gtk_entry_set_max_length(GTK_ENTRY(entry_ilosc), 10);

    gtk_grid_attach(GTK_GRID(add_grid), entry_ilosc, 2, 1, 1, 1);


    GtkWidget *entry_cena = gtk_entry_new();
    GtkStyleContext *context_entry_cena= gtk_widget_get_style_context(entry_cena);
    gtk_style_context_add_class(context_entry_cena, "add_edit_entry");
    gtk_widget_set_size_request(GTK_ENTRY(entry_cena), 150, 30);
    gtk_entry_set_text(GTK_ENTRY(entry_cena), "");
    gtk_entry_set_max_length(GTK_ENTRY(entry_cena), 10);

    gtk_grid_attach(GTK_GRID(add_grid), entry_cena, 3, 1, 1, 1);


    GtkWidget *grid_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    //gtk_container_add(GTK_CONTAINER(grid_box), edit_grid);
    gtk_box_pack_start(GTK_BOX(grid_box), add_grid, FALSE, FALSE, 0);
    gtk_box_set_center_widget(GTK_BOX(grid_box), add_grid);

    // ---------------------------------------- BUTTONS
    GtkWidget *add_button = gtk_button_new_with_label("Dodaj");
    GtkStyleContext *context_button = gtk_widget_get_style_context(add_button);
    gtk_style_context_add_class(context_button, "add_row_button_add");


    struct add_row_data *data_send = (struct save_edit_data*)malloc(sizeof(struct save_edit_data));
    data_send->tytul = entry_tytul;
    data_send->autor = entry_autor;
    data_send->ilosc = entry_ilosc;
    data_send->cena = entry_cena;
    data_send->AddWindow = add_window;

    g_signal_connect (add_button, "clicked", G_CALLBACK (add_new_row), data_send);

    GtkWidget *cancel_button = gtk_button_new_with_label("Anuluj");
    GtkStyleContext *context_button_cancel = gtk_widget_get_style_context(cancel_button);
    gtk_style_context_add_class(context_button_cancel, "add_row_button_cancel");

    g_signal_connect (cancel_button, "clicked", G_CALLBACK (add_row_cancel), add_window);


    GtkWidget *buttons_grid = gtk_grid_new();

    gtk_grid_attach(GTK_GRID(buttons_grid), add_button, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(buttons_grid), cancel_button, 1, 0, 1, 1);



    GtkWidget *buttons_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(buttons_box), buttons_grid);



    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkStyleContext *context_main_box = gtk_widget_get_style_context(main_box);
    gtk_style_context_add_class(context_main_box, "black_background");
    gtk_box_pack_start(GTK_BOX(main_box), grid_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), buttons_box, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(add_window), main_box);


    gtk_widget_show_all(add_window);
}

void on_search_button(GtkButton *button, gpointer data){
    GtkWidget *entry_search = data;
    const gchar *search_text = gtk_entry_get_text(GTK_ENTRY(entry_search));

    ksiazka* iterator = magazyn;

    if(iterator != NULL){
        if(entry_search == ""){
            // pokazywanie wszystkich wierszy
            while(iterator != NULL){
                show_grid_row(iterator->pozycja);
                iterator = iterator->nastepny;
            }
        }
        else{
            bool contains;
            char *ret;
            while(iterator != NULL){
                contains = 0;
                ret = strstr(iterator->tytul, search_text);
                if(ret){
                    contains = 1;
                }
                ret = strstr(iterator->autor, search_text);
                if(ret && !contains){
                    contains = 1;
                }
                ret = strstr(iterator->ilosc, search_text);
                if(ret && !contains){
                    contains = 1;
                }
                ret = strstr(iterator->cena, search_text);
                if(ret && !contains){
                    contains = 1;
                }
                if(contains){
                    show_grid_row(iterator->pozycja);
                }
                else{
                    hide_grid_row(iterator->pozycja);
                }
                iterator = iterator->nastepny;
            }
        }
    }

}

void on_sidebar_listbox_row_activated(GtkWidget *widget, gpointer data) {

    if(last_selected_list_box_row != NULL){
        // usuwanie stylu dla wczesniej aktywnej zakladki sidebar
        GtkStyleContext *context_row = gtk_widget_get_style_context(last_selected_list_box_row);
        gtk_style_context_remove_class(context_row, "sidebar_list_box_row_active");
    }

    GtkListBoxRow *selected_list_box_row = gtk_list_box_get_selected_row(GTK_LIST_BOX(widget)); // pobieranie zaznaczonego list box row
    last_selected_list_box_row = selected_list_box_row; // ustawianie last_selected_list_box_row na aktualnie zaznaczony row

    // zmiana stylu dla aktualnie zaznaczonego wiersza
    GtkStyleContext *context_selected_row = gtk_widget_get_style_context(selected_list_box_row);
    gtk_style_context_add_class(context_selected_row, "sidebar_list_box_row_active");

    gpointer *gpoi = g_object_get_data(G_OBJECT(selected_list_box_row), "page_num"); // pobieranie numeru zaznaczonego wiersza

    int num = GPOINTER_TO_INT(gpoi); // zmiana na int
    const gchar* name = stack_names[num]; // pobieranie nazwy zeby wyszukac stack

    if(name == "exit"){
        // wyjscie z programu
        gtk_widget_destroy(main_window);
        return;
    }

    GtkWidget *stack_child = gtk_stack_get_child_by_name(GTK_STACK(main_stack), name); // pobieranie stack child z ta nazwa
    gtk_stack_set_visible_child(GTK_STACK(main_stack), GTK_WIDGET(stack_child)); // ustawianie tego stack na widoczny
}

static void activate (GtkApplication *app, gpointer user_data){

    set_columns_names(); // ustawianie nazw kolumn
    myCSS();

    main_window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (main_window), "System Biblioteki");
    gtk_window_set_default_size (GTK_WINDOW (main_window), 1200, 800);
    gtk_window_set_resizable (GTK_WINDOW(main_window), FALSE); // wylaczenie mozliwosci zmieniania rozmiaru okna



    main_stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(main_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(main_stack), 500);


    // Wczytywanie pliku @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    GtkWidget *lodafile_grid = gtk_grid_new();
    GtkStyleContext *context_lodafile_grid = gtk_widget_get_style_context(lodafile_grid);
    gtk_style_context_add_class(context_lodafile_grid, "load_file_grid");

    GtkWidget *loadfile_button = gtk_button_new_with_label("Wczytaj plik");
    GtkStyleContext *context_loadfile_button = gtk_widget_get_style_context(loadfile_button);
    gtk_style_context_add_class(context_loadfile_button, "menu_blue_button");

    GtkWidget *loadfile_label = gtk_label_new("Nie wybrano pliku");
    GtkStyleContext *context_loadfile_label = gtk_widget_get_style_context(loadfile_label);
    gtk_style_context_add_class(context_loadfile_label, "load_file_label");

    g_signal_connect (loadfile_button, "clicked", G_CALLBACK (load_file), loadfile_label);


    gtk_grid_set_column_spacing (GTK_GRID (lodafile_grid), 10);
    gtk_grid_set_row_spacing (GTK_GRID (lodafile_grid), 10);

    gtk_grid_attach(GTK_GRID(lodafile_grid), loadfile_button, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(lodafile_grid), loadfile_label, 1, 0, 1, 1);


    gtk_stack_add_named(GTK_STACK(main_stack), lodafile_grid, "load_file");



    // Wyświetlanie magazynu @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    GtkWidget *box_showitems = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); // główny box na całą zakładke


    GtkWidget *loadmagazyn_button = gtk_button_new_with_label("Zaladuj magazyn");
    GtkStyleContext *context_button_loadgrid = gtk_widget_get_style_context(loadmagazyn_button);
    gtk_style_context_add_class(context_button_loadgrid, "menu_blue_button");
    g_signal_connect (loadmagazyn_button, "clicked", G_CALLBACK (load_magazyn), NULL);

    GtkWidget *add_button = gtk_button_new_with_label("Dodaj wiersz");
    GtkStyleContext *context_add_button = gtk_widget_get_style_context(add_button);
    gtk_style_context_add_class(context_add_button, "menu_green_button");
    g_signal_connect (add_button, "clicked", G_CALLBACK (on_add_row), NULL);

    GtkWidget *entry_search = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry_search), 20);
    gtk_entry_set_max_length(GTK_ENTRY(entry_search), 100);
    GtkStyleContext *context_entry_search = gtk_widget_get_style_context(entry_search);
    gtk_style_context_add_class(context_entry_search, "menu_search_entry");
    g_signal_connect (entry_search, "activate", G_CALLBACK (on_search_button), entry_search);

    GtkWidget *button_search =  gtk_button_new_from_icon_name("edit-find", GTK_ICON_SIZE_BUTTON);
    GtkStyleContext *context_search_button = gtk_widget_get_style_context(button_search);
    gtk_style_context_add_class(context_search_button, "menu_search_button");
    g_signal_connect (button_search, "clicked", G_CALLBACK (on_search_button), entry_search);


    GtkWidget *menu_items_grid = gtk_grid_new(); // menu grid z przyciskami

    GtkStyleContext *context_menu_items_grid = gtk_widget_get_style_context(menu_items_grid);
    gtk_style_context_add_class(context_menu_items_grid, "menu_items_grid");

    gtk_grid_set_column_spacing (GTK_GRID (menu_items_grid), 5);


    gtk_grid_attach(GTK_GRID(menu_items_grid), loadmagazyn_button, 0, 0, 1, 1); // dodajemy przycisk "Zaladuj magazyn" do grida
    gtk_grid_attach(GTK_GRID(menu_items_grid), add_button, 1, 0, 1, 1); // dodajemy przycisk "Dodaj wiersz" do grida
    gtk_grid_attach(GTK_GRID(menu_items_grid), entry_search, 2, 0, 1, 1); // dodajemy entry z do wyszukiwania do grida
    gtk_grid_attach(GTK_GRID(menu_items_grid), button_search, 3, 0, 1, 1); // dodajemy przycisk wyszukania do grida



    listitems_grid = gtk_grid_new();

    GtkStyleContext *context_grid = gtk_widget_get_style_context(listitems_grid);
    gtk_style_context_add_class(context_grid, "grid_main");

    gtk_grid_set_row_spacing(GTK_GRID(listitems_grid), 10);

    add_title_row();

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_window, 1200, 800);

    GtkStyleContext *context_scrolled_win = gtk_widget_get_style_context(scrolled_window);
    gtk_style_context_add_class(context_scrolled_win, "scrolled_window");

    gtk_container_add(GTK_CONTAINER(scrolled_window), listitems_grid); // do scrolled_window dajemy listitems_grid
    gtk_widget_set_halign(listitems_grid, GTK_ALIGN_CENTER);


    gtk_box_pack_start(GTK_BOX(box_showitems), menu_items_grid, FALSE, FALSE, 0); // najpierw menu
    gtk_box_pack_start(GTK_BOX(box_showitems), scrolled_window, FALSE, FALSE, 0); // potem scrolled_window, które zawiera listitems_grid



    gtk_stack_add_named(GTK_STACK(main_stack), box_showitems, "magazyn");


    // zapisywanie aktualnego magazynu @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    GtkWidget *save_grid = gtk_grid_new();
    GtkStyleContext *context_save_grid = gtk_widget_get_style_context(save_grid);
    gtk_style_context_add_class(context_save_grid, "load_file_grid");

    GtkWidget *save_button = gtk_button_new_with_label("Zapisz magazyn");
    GtkStyleContext *context_save_button = gtk_widget_get_style_context(save_button);
    gtk_style_context_add_class(context_save_button, "menu_blue_button");

    GtkWidget *save_label = gtk_label_new("");
    GtkStyleContext *context_save_label = gtk_widget_get_style_context(save_label);
    gtk_style_context_add_class(context_save_label, "load_file_label");

    g_signal_connect (save_button, "clicked", G_CALLBACK (save_file), save_label);


    gtk_grid_set_column_spacing (GTK_GRID (save_grid), 10);
    gtk_grid_set_row_spacing (GTK_GRID (save_grid), 10);

    gtk_grid_attach(GTK_GRID(save_grid), save_button, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(save_grid), save_label, 1, 0, 1, 1);


    gtk_stack_add_named(GTK_STACK(main_stack), save_grid, "save");


    // wychodzenie z programu @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    GtkWidget *Exit_button = gtk_button_new_with_label("Exit");
    g_signal_connect_swapped (Exit_button, "clicked", G_CALLBACK (gtk_widget_destroy), main_window);

    gtk_stack_add_named(GTK_STACK(main_stack), Exit_button, "exit");


    // pasek boczny wyboru zakładek @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


    GtkWidget *listbox_sidebar = gtk_list_box_new(); // glowny listbox na sidebar
    GtkStyleContext *context_listbox_sidebar = gtk_widget_get_style_context(listbox_sidebar);
    gtk_style_context_add_class(context_listbox_sidebar, "sidebar_list_box");

    // zakładka Wczytaj plik
    GtkWidget *load_file_sidebar_row = gtk_list_box_row_new();
    GtkStyleContext *context_load_file_sidebar_row = gtk_widget_get_style_context(load_file_sidebar_row);
    gtk_style_context_add_class(context_load_file_sidebar_row, "sidebar_list_box_row");
    gtk_style_context_add_class(context_load_file_sidebar_row, "sidebar_list_box_row_active");
    last_selected_list_box_row = load_file_sidebar_row;

    g_object_set_data (G_OBJECT (load_file_sidebar_row), "page_num", GINT_TO_POINTER (0)); // ustawianie numeru zeby potem w stack_names[0] mogl znalezc nazwe

    GtkWidget *sidebar_list_box_label1 = gtk_label_new("Wczytaj plik");

    gtk_container_add (GTK_CONTAINER(load_file_sidebar_row), sidebar_list_box_label1);
    gtk_container_add (GTK_CONTAINER(listbox_sidebar), load_file_sidebar_row);

    // zakładka Magazyn

    GtkWidget *magazyn_sidebar_row = gtk_list_box_row_new();
    GtkStyleContext *context_magazyn_sidebar_row = gtk_widget_get_style_context(magazyn_sidebar_row);
    gtk_style_context_add_class(context_magazyn_sidebar_row, "sidebar_list_box_row");

    g_object_set_data (G_OBJECT (magazyn_sidebar_row), "page_num", GINT_TO_POINTER (1)); // ustawianie numeru zeby potem w stack_names[1] mogl znalezc nazwe

    GtkWidget *sidebar_list_box_label2 = gtk_label_new("Magazyn");

    gtk_container_add (GTK_CONTAINER(magazyn_sidebar_row), sidebar_list_box_label2);
    gtk_container_add (GTK_CONTAINER(listbox_sidebar), magazyn_sidebar_row);

    // zakladka Zapisz
    GtkWidget *save_sidebar_row = gtk_list_box_row_new();
    GtkStyleContext *context_save_sidebar_row = gtk_widget_get_style_context(save_sidebar_row);
    gtk_style_context_add_class(context_save_sidebar_row, "sidebar_list_box_row");

    g_object_set_data (G_OBJECT (save_sidebar_row), "page_num", GINT_TO_POINTER (2)); // ustawianie numeru zeby potem w stack_names[2] mogl znalezc nazwe

    GtkWidget *sidebar_list_box_label3 = gtk_label_new("Zapisz");

    gtk_container_add (GTK_CONTAINER(save_sidebar_row), sidebar_list_box_label3);
    gtk_container_add (GTK_CONTAINER(listbox_sidebar), save_sidebar_row);

    // zakładka Exit

    GtkWidget *exit_sidebar_row = gtk_list_box_row_new();
    GtkStyleContext *context_exit_sidebar_row = gtk_widget_get_style_context(exit_sidebar_row);
    gtk_style_context_add_class(context_exit_sidebar_row, "sidebar_list_box_row");

    g_object_set_data (G_OBJECT (exit_sidebar_row), "page_num", GINT_TO_POINTER (3)); // ustawianie numeru zeby potem w stack_names[3] mogl znalezc nazwe

    GtkWidget *sidebar_list_box_label4 = gtk_label_new("Wyjdz");

    gtk_container_add (GTK_CONTAINER(exit_sidebar_row), sidebar_list_box_label4);
    gtk_container_add (GTK_CONTAINER(listbox_sidebar), exit_sidebar_row);


    g_signal_connect(listbox_sidebar, "row-activated", G_CALLBACK(on_sidebar_listbox_row_activated), NULL);


    // dodawanie wszystkiego do głównego okna @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box), listbox_sidebar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), main_stack, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(main_window), box);


    gtk_widget_show_all (main_window);
}

int main (int argc, char **argv){
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
