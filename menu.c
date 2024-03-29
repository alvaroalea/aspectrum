/*=====================================================================
  menu.c ->    This file includes all the menu functions
                for the emulator, called from the main loop.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 Copyright (c) 2000 Santiago Romero Iglesias.
 Email: sromero@escomposlinux.org
 ======================================================================*/

#ifdef _DEBUG_
#include <mss.h>
#endif

#include <stdio.h>
#include "v_alleg.h"
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_audio.h>
#include "z80.h"
#include "langs.h"
#include "menu.h"
#include "monofnt.h"
#include "main.h"
#include "snaps.h"
#include "mem.h"
extern ALLEGRO_DISPLAY *display;
extern ALLEGRO_AUDIO_STREAM *audioStream;
extern ALLEGRO_EVENT_QUEUE *colaD;
extern ALLEGRO_EVENT_QUEUE *colaM; 
ALLEGRO_MENU *menuprinc = NULL;
extern int language;

#ifdef NO_USE_MENU
void DrawSelected (int x1, int y1, int x2, int y2, char *text, int bgcolor, int fgcolor, int textbgselcolor, char *tfont){ ; }
int menuopciones (void){ return 0; }

int v_alertYesNo (const char *Title, const char *Head, const char *Text){
  int ret;
  al_set_audio_stream_playing(audioStream,false);
  ret = al_show_native_message_box(display, Title, Head, Text, NULL, ALLEGRO_MESSAGEBOX_YES_NO);
  al_set_audio_stream_playing(audioStream,true);
  v_cleankeys();
  return (ret);  
}
int v_alertErrOK (const char *Title, const char *Head, const char *Text){
  int ret;
  al_set_audio_stream_playing(audioStream,false);
  ret = al_show_native_message_box(display, Title, Head, Text, NULL, ALLEGRO_MESSAGEBOX_ERROR);
  al_set_audio_stream_playing(audioStream,true);
  v_cleankeys();
  return (ret);  
}

/*---------------------------------------------------------------------------
// Filebox selection popup :-), it receives the Type of popup (Load SNA,
// Save SNA, Save SCR...) and returns the selected filename.
*--------------------------------------------------------------------------*/
//int FileMenu (char *tfont, char type, char *filename){ return 0; }
int FileMenu (char type, char *filename){
  extern tipo_emuopt emuopt;
  ALLEGRO_FILECHOOSER *dialogo;
  int ret;
  char *tmpname;

  const char extensions[FILEBOX_TYPES][7*9] = {
    "*.SNA;*.SP;*.Z80;*.SCR;*.sna;*.sp;*.z80;*.scr",  //carga
    "*.SNA;*.sna",
    "*.SCR;*.scr",
   "*.TAP;*.TZX;*.tap;*.tzx"          //cintas
  };
  const int mode[FILEBOX_TYPES]={
    ALLEGRO_FILECHOOSER_FILE_MUST_EXIST,
    ALLEGRO_FILECHOOSER_SAVE,
    ALLEGRO_FILECHOOSER_SAVE,
    0
  };
  /*
  if (emuopt.gunstick & GS_GUNSTICK)
    set_mouse_sprite (NULL);
  */
  al_set_audio_stream_playing(audioStream,false);
  //ASprintf("Creando dialogo\n");
  dialogo = al_create_native_file_dialog(NULL,lang_filemenu[(language * FILEBOX_TYPES) + type],extensions[type],mode[type]);
  ret =  al_show_native_file_dialog(display, dialogo);
  if (al_get_native_file_dialog_count(dialogo)==1){
    tmpname = al_get_native_file_dialog_path(dialogo,0);
    strcpy(filename, tmpname);
  }
  //ASprintf("r:%i      --- file=%s\n",ret,filename);
  
  al_destroy_native_file_dialog(dialogo);
  //ASprintf("r:%i      --- file=%s\n",ret,filename);
  al_set_audio_stream_playing(audioStream,true);
  //ASprintf("Destruyendo dialogo\n");
  
  //ret = file_select_ex (lang_filemenu[(language * FILEBOX_TYPES) + type], filename, extensions[type] ,512, 290, 170);

  // si usamos gunstick volvemos a poner el punto de mira 
  /*
  if (emuopt.gunstick & GS_GUNSTICK){
      set_mouse_sprite (emuopt.raton_bmp);
      set_mouse_sprite_focus (8, 8);
  }
  */
  v_cleankeys();
  return (ret);
}


int about_proc (void){
  //FIXME Location of dialog.
  v_alertErrOK("Acerca de...","Aspectrum Version: "VERSION,"(C) 2000-2003 Santiago Romero, Kak y Alvaro Alea\nDistribuido bajo licencia GPL V2");
   return D_CLOSE;
}
void MainMenu (void){
  ALLEGRO_MENU_INFO menu_info[] = {   
    ALLEGRO_START_OF_MENU("Archivo", 1),
      { "Cargar Snapshot...   F3", 10, 0, NULL },
      { "Guardar Snapshot...   F2", 11, 0, NULL },
      { "Guardar Imagen de Pantalla...   F4", 12, 0, NULL },
      ALLEGRO_MENU_SEPARATOR,
      { "Salir   F11", 13, 0, NULL },
      ALLEGRO_END_OF_MENU,
    ALLEGRO_START_OF_MENU("Opciones", 2),
      { "Snapshots...", 21, ALLEGRO_MENU_ITEM_DISABLED, NULL },
      { "Cintas...", 22, ALLEGRO_MENU_ITEM_DISABLED, NULL },
      { "Video...", 23, ALLEGRO_MENU_ITEM_DISABLED, NULL },
      { "Opciones", 24, ALLEGRO_MENU_ITEM_DISABLED, NULL },
      { "Grabaciones", 25, ALLEGRO_MENU_ITEM_DISABLED, NULL },
      { "Cambiar Idioma    F8", 26, 0, NULL },
      ALLEGRO_END_OF_MENU,
    ALLEGRO_START_OF_MENU("Maquina", 3),
      { "Reset    F5", 31, 0, NULL },
      { "NMI", 32, 0, NULL },
      { "Ejecutar Debbuger   F1", 33, 0, NULL },
      ALLEGRO_START_OF_MENU("Seleccionar Hardware", 34),
        { "16K", 341, ALLEGRO_MENU_ITEM_CHECKBOX, NULL },
        { "48K", 342, ALLEGRO_MENU_ITEM_CHECKED, NULL },
        { "Inves 48K", 343, ALLEGRO_MENU_ITEM_CHECKBOX, NULL },
        { "128K", 344, ALLEGRO_MENU_ITEM_CHECKBOX, NULL },
        { "+2", 345, ALLEGRO_MENU_ITEM_CHECKBOX, NULL },
        { "+2A", 346, ALLEGRO_MENU_ITEM_CHECKBOX, NULL },
        ALLEGRO_END_OF_MENU,
      ALLEGRO_END_OF_MENU,
    ALLEGRO_START_OF_MENU("Cinta", 4),
      { "Abrir Cinta...   F6", 41, 0, NULL },
      { "Ver Cinta", 42, ALLEGRO_MENU_ITEM_DISABLED, NULL },
      { "Rebobinar Cinta", 43, 0, NULL },
      { "Play", 44, ALLEGRO_MENU_ITEM_DISABLED, NULL },
      { "Stop", 45, ALLEGRO_MENU_ITEM_DISABLED, NULL },
      { "Grabar", 46, ALLEGRO_MENU_ITEM_DISABLED, NULL },
      ALLEGRO_END_OF_MENU,
    ALLEGRO_START_OF_MENU("Ayuda", 5),
      { "Ayuda Teclado", 51, 0, NULL },
      { "Ayuda Debbuger  F12", 52, 0, NULL },
      { "Acerca de...", 53, 0, NULL },
      ALLEGRO_END_OF_MENU,  
  ALLEGRO_END_OF_MENU, 
  };

//ASprintf("antes de crear\n");
menuprinc = al_build_menu(menu_info);
//ASprintf("antes de asignar\n");
if (!al_set_display_menu(display, menuprinc)) {
 // pmenu = al_clone_menu_for_popup(menu);
 // al_destroy_menu(menu);
 // menu = pmenu;
 ASprintf("pues fallo el menu...\n");
}
al_register_event_source(colaM, al_enable_menu_event_source(menuprinc));

//ASprintf("asigno\n");
}

int MainMenuClick (void){
  int ret=0;
  ALLEGRO_EVENT evento;
  if (al_peek_next_event(colaD, &evento)) {
    //ASprintf ("%i\n",evento.type);
    if (evento.type==ALLEGRO_EVENT_DISPLAY_CLOSE) {
        ret = gKEY_F11;  
    }
    al_drop_next_event(colaD); 
  }

  if (al_peek_next_event(colaM, &evento)) {
    if (evento.type==ALLEGRO_EVENT_MENU_CLICK) {
      switch (evento.user.data1){
      case (33): //F1
        ret = gKEY_F1;
        break;
      case (11): //F2
        ret = gKEY_F2;
        break;
      case (10): //F3
        ret = gKEY_F3;
        break;
      case (12): //F4
        ret = gKEY_F4;
        break;
      case (31): //F5
        ret = gKEY_F5;
        break;
      case (41): //F6
        ret = gKEY_F6;
        break;
      case (13): //F11
        ret = gKEY_F11;
        break;
      case (51): //F12
        ret = gKEY_F12;
        break;
      case (341):
        menuhardware(SPECMDL_16K);
        break;
      case (342):
        menuhardware(SPECMDL_48K);
        break;
      case (343):
        menuhardware(SPECMDL_INVES);
        break;
      case (344):
        menuhardware(SPECMDL_128K);
        break;
      case (345):
        menuhardware(SPECMDL_PLUS2);
        break;
      case (346):
        menuhardware(SPECMDL_PLUS3);
        break;
      }
    al_drop_next_event(colaM);  
    }
  }
  return ret;
}

int menuhardware (int model){
  extern tipo_hwopt hwopt;
  extern Z80Regs spectrumZ80;
  if (model!=0) {
    if (hwopt.hw_model!=model){
      if (v_alertYesNo ("Cambiar Modelo Hardware","Esto reseteara el emulador.", "Confirme que desea cambiar.") == 1){
        end_spectrum();
        init_spectrum(model,"");
        //printf("Inicio %i\n",c-2);
        Z80Reset (&spectrumZ80, 69888);
      }
    } 
  }
}

void MainMenuUpdateModel(int model){
  al_set_menu_item_flags(menuprinc,341,model==SPECMDL_16K?ALLEGRO_MENU_ITEM_CHECKED:NULL);
  al_set_menu_item_flags(menuprinc,342,model==SPECMDL_48K?ALLEGRO_MENU_ITEM_CHECKED:NULL);
  al_set_menu_item_flags(menuprinc,343,model==SPECMDL_INVES?ALLEGRO_MENU_ITEM_CHECKED:NULL);
  al_set_menu_item_flags(menuprinc,344,model==SPECMDL_128K?ALLEGRO_MENU_ITEM_CHECKED:NULL);
  al_set_menu_item_flags(menuprinc,345,model==SPECMDL_PLUS2?ALLEGRO_MENU_ITEM_CHECKED:NULL);
  al_set_menu_item_flags(menuprinc,346,model==SPECMDL_PLUS3?ALLEGRO_MENU_ITEM_CHECKED:NULL);
}

#else  // def NO_USE_MENU

#ifdef I_HAVE_AGUP
#include <agup.h>
//#include <agtk.h>
//#include <aphoton.h>
//#include <awin95.h>
//#include <aase.h>
#define GUI_check_proc d_agup_check_proc
#define GUI_radio_proc d_agup_radio_proc
#else
#define GUI_check_proc d_check_proc
#define GUI_radio_proc d_radio_proc
#endif


#define NUM_FILES 9

extern int language;
static volatile int selected_opt = -1;

int dummy_proc (void)
{
  if (language == 0)
    galert ("Still unimplemented", NULL, NULL, "Ok", NULL, 0, 0);
  else if (language == 1)
    galert ("Todavia sin implementar", NULL, NULL, "Ok", NULL, 0, 0);
  return D_O_K;
}

int quit_proc (void)
{
  selected_opt = DIALOG_QUIT;
  return D_CLOSE;
}

int load_proc (void)
{
  selected_opt = DIALOG_SNAP_LOAD;
  return D_CLOSE;
}

int save_proc (void)
{
  selected_opt = DIALOG_SNAP_SAVE;
  return D_CLOSE;
}

int debugger_proc (void)
{
  selected_opt = DIALOG_DEBUGGER_0;
  return D_CLOSE;
}

int reset_proc (void)
{
  selected_opt = DIALOG_RESET;
  return D_CLOSE;
}

int savescr_proc (void)
{
  selected_opt = DIALOG_SAVE_SCR;
  return D_CLOSE;
}

int opentape_proc (void)
{
  selected_opt = DIALOG_OPEN_TAPE;
  return D_CLOSE;
}

int rewindtape_proc (void)
{
  selected_opt = DIALOG_REWIND_TAPE;
  return D_CLOSE;
}
int opciones_proc (void)
{
  selected_opt = DIALOG_OPTIONS;
  return D_CLOSE;
}
int hardware_proc (void)
{
  selected_opt = DIALOG_HARDWARE;
  return D_CLOSE;
}

int changelang_proc (void)
{
  selected_opt = DIALOG_CHANGE_LANG;
  return D_CLOSE;
}



int about_proc (void)
{
   DIALOG dlg[] ={
      { d_agup_window_proc, 0, 0, 250, 100, 0, 0, 0, 0, 0, 0,(void *)"Acerca de...", NULL, NULL },
      { d_ctext_proc, 125, 30, 0, 0, agup_fg_color, agup_bg_color, 0, 0, 0, 0, (void *)"Aspectrum Version: "VERSION, NULL, NULL },
      { d_ctext_proc, 125, 40, 0, 0, agup_fg_color, agup_bg_color, 0, 0, 0, 0, (void *)"(C) 2000-2003 Santiago Romero, Kak y Alvaro Alea", NULL, NULL },
      { d_ctext_proc, 125, 50, 0, 0, agup_fg_color, agup_bg_color, 0, 0, 0, 0, (void *)"Distribuido bajo licencia GPL V2", NULL, NULL },
      {gui_button_proc, 85, 75, 70, 16, 0, 0, 13, D_EXIT, 0, 0, "OK", NULL, NULL},
      { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }

    };

   centre_dialog(dlg);
   do_dialog(dlg, -1);
   return D_CLOSE;

/*  alert("Aspectrum Version: "VERSION,"(C) 2000-2003 Santiago Romero, Kak y Alvaro Alea",
	"Distribuido bajo licencia GPL V2","OK",NULL,13,27);
  selected_opt = DIALOG_ABOUT;
  return D_REDRAW;
*/

}



/*-----------------------------------------------------------------
 * This function implements the main options menu.
 ------------------------------------------------------------------*/
int
MainMenu (Z80Regs * regs, char *tfont)
{

//  int i, end = 0, keypress;
  int selected = 0;
//  int fonth = 12;
//  int menux = 20, menuy = 15, menuw = 280, menuh = 170;
//  int bgcolor = 15, titlecolor = 14;
//  int bgselcolor = 11, textbgselcolor = 13;
//  int fgcolor = 0;

#include "dialog_en.h"
#include "dialog_es.h"
  selected_opt=-1;
  selected = -1;
  set_dialog_color (menu_dlg_EN, gui_fg_color, gui_bg_color);
  set_dialog_color (menu_dlg_ES, gui_fg_color, gui_bg_color);
  if (language == 0)
    do_dialog (menu_dlg_EN, -1);
  else
    do_dialog (menu_dlg_ES, -1);

  return selected_opt;
}  

/*
    gbox( menux, menuy, menux+menuw, menuy+menuh, bgcolor );
    grectangle( menux+1, menuy+1, menux+menuw-1, menuy+menuh-1, fgcolor );
    gbox( menux+1, menuy+1, menux+menuw-1, menuy+1+fonth, fgcolor );
    GFXprintf_tovideo( menux+7, menuy+4, lang_menu_title[language],
                       tfont, titlecolor, fgcolor, 0);
   
    
    for(i=0; i<NUM_MENU_OPTIONS; i++)
      GFXprintf_tovideo( menux+12, (fonth*i)+42, 
                         lang_main_options[(language*NUM_MENU_OPTIONS)+i],
                         tfont, fgcolor, bgcolor, 0);
       
   
    while(!end)
    {
       DrawSelected( menux+2, (fonth*selected)+42-3,
                     menux+menuw-2, (fonth*selected)+42+fonth-3,
                     lang_main_options[(language*NUM_MENU_OPTIONS)+selected],
                     fgcolor, bgselcolor, textbgselcolor, tfont );

       dumpVirtualToScreen();
       DrawSelected( menux+2, (fonth*selected)+42-3,
                     menux+menuw-2, (fonth*selected)+42+fonth-3,
                     lang_main_options[(language*NUM_MENU_OPTIONS)+selected],
                     fgcolor, bgcolor, bgcolor, tfont );
       keypress = readkey();
       switch( keypress >> 8 )
       {
         case KEY_DOWN: if( selected < NUM_MENU_OPTIONS-1 ) selected++;
                        break;
         case KEY_UP:   if( selected > 0 ) selected--;
                        break;
         case KEY_ESC:  return(0);
                        break;
         case KEY_F1:   return(1);
                        break;
         case KEY_F2:   return(2);
                        break;
         case KEY_F3:   return(3);
                        break;
         case KEY_F4:   return(4);
                        break;
         case KEY_F5:   return(5);
                        break;
         case KEY_F6:   return(1);
                        break;
         case KEY_F7:   return(7);
                        break;
         case KEY_F8:  return(8);
                        break;
         case KEY_F10:  return(3);
                        break;

         case KEY_ENTER:
         case KEY_SPACE:
                        if( selected <= 8 )
                            return(selected);
                        else return(10);
       };
    }
  }
*/



// Draws the selected or unselected option...
void
DrawSelected (int x1, int y1, int x2, int y2, char *text, int fgcolor,
	      int bgcolor, int textbgcolor, char *tfont)
{
  gbox (x1, y1, x2, y2, bgcolor);
  GFXprintf_tovideo (x1 + 12 - 2, y1 + 3, text, tfont, fgcolor, textbgcolor,
		     0);
}




/*
char *lista_joys(int index int *list_size)
{
	switch (index)
	{
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		default: if (index<0)
		{
			list_size=3;
			return NULL;
		}
		
	}
}
*/

int
menuopciones (void)
{
  extern tipo_emuopt emuopt;
  DIALOG dialogo[] = {
    { d_agup_window_proc, 0, 0, 195, 90, 0, 0, 0, 0, 0, 0,(void *)lang_generaloptions[language], NULL, NULL },
//  {gui_shadow_box_proc, 0, 0, 195, 90, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL},
//  {d_text_proc, 25, 5, 100, 16, 0, 0, 0, 0, 0, 0, lang_generaloptions[language], NULL, NULL},
    {GUI_check_proc, 10, 30, 50, 8, 0, 0, 'G', 0, 1, 0,lang_emulagunstick[language], NULL, NULL},
    {GUI_check_proc, 10, 45, 50, 8, 0, 0, 'S', 0, 1, 0,lang_soundactive[language], NULL, NULL},
    {gui_button_proc, 40, 70, 70, 16, 0, 0, 13, D_EXIT, 0, 0, "OK", NULL, NULL},
    {gui_button_proc, 120, 70, 70, 16, 0, 0, 27, D_EXIT, 0, 0, "Cancel", NULL, NULL},
//     {d_list_proc,10,30,50,16, 0,0,NULL,d1,d2,lista_joys,*dp1,*dp2},
    {NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL}
  };

  /* ponemos en gris el boton, o el raton en flecha segun las
     opciones 
   */
  if (!(emuopt.gunstick & GS_HAYMOUSE))
    dialogo[1].flags |= D_DISABLED;
  if (emuopt.gunstick & GS_GUNSTICK)
    {
      dialogo[1].flags |= D_SELECTED;
      set_mouse_sprite (NULL);
    }
  /* centramos el cuadro de dialogo y si se pulsa aceptar ocultamos
     o mostramos el raton segun sea necesario
   */
  centre_dialog (dialogo);
  set_dialog_color (dialogo, gui_fg_color, gui_bg_color);
  if (popup_dialog (dialogo, 2) == 3)	//2 del que lleva el foco, 3 del boton de aceptar.
    {
      if (dialogo[1].flags & D_SELECTED)
	{
	  emuopt.gunstick |= GS_GUNSTICK;
	  show_mouse (screen);
	}
      else
	{
	  emuopt.gunstick &= ~GS_GUNSTICK;
	  set_mouse_sprite (NULL);
	  show_mouse (NULL);

	}
    }
  /* si usamos gunstick volvemos a poner el punto de mira
   */
  if (emuopt.gunstick & GS_GUNSTICK)
    {
      set_mouse_sprite (emuopt.raton_bmp);
      set_mouse_sprite_focus (8, 8);
    }
  return (0);
}


int
menuhardware (int model)
{
  extern tipo_hwopt hwopt;
  extern Z80Regs spectrumZ80;	
  int c;
  DIALOG dialogo[] = {
    {d_agup_window_proc, 0, 0, 195, 155, 0, 0, 0, 0, 0, 0,"Seleccion de Hardware", NULL, NULL },
    {gui_button_proc, 30, 130, 70, 16, 0, 0, 13, D_EXIT, 0, 0, "OK", NULL, NULL},
    {gui_button_proc, 110, 130, 70, 16, 0, 0, 27, D_EXIT, 0, 0, "Cancel", NULL, NULL},
    {GUI_radio_proc, 10, 30, 50, 8, 0, 0, 0, 0, 1, 0,"Spectrum 16K", (void *)1,(void *)0},
    {GUI_radio_proc, 10, 45, 50, 8, 0, 0, 0, 0, 1, 0,"Spectrum 48K", (void *)1,(void *)0},
    {GUI_radio_proc, 10, 60, 50, 8, 0, 0, 0, 0, 1, 0,"Inves Spectrum+ 48K", (void *)1,(void *)0},
    {GUI_radio_proc, 10, 75, 50, 8, 0, 0, 0, 0, 1, 0,"Spectrum 128K Espanol", (void *)1,(void *)0},
    {GUI_radio_proc, 10, 90, 50, 8, 0, 0, 0, 0, 1, 0,"Spectrum +2 (128K)", (void *)1,(void *)0},
    {GUI_radio_proc, 10,105, 50, 8, 0, 0, 0, 0, 1, 0,"Spectrum +3 (128K)",(void *)1,(void *)0},
    {NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL}
  };

  /* Selecionamos el tipo de arquitetura actual.
   */
  dialogo[hwopt.hw_model+2].flags |= D_SELECTED;
       
   /* centramos el cuadro de dialogo y si se pulsa aceptar ocultamos
     o mostramos el raton segun sea necesario
   */
  centre_dialog (dialogo);
  set_dialog_color (dialogo, gui_fg_color, gui_bg_color);
  if (popup_dialog (dialogo, 2) == 1)	//2 del que lleva el foco, 1 del boton de aceptar.
    {
     for(c=3;c<8;c++) if ((dialogo[c].flags & D_SELECTED)==D_SELECTED) break ;
	 //printf("Sale %i\n",c-2);
	 end_spectrum();
	 init_spectrum(c-2,"");
	 //printf("Inicio %i\n",c-2);
	 Z80Reset (&spectrumZ80, 69888);
    }
  return (0);
}
#endif // else - ifdef NO_USE_MENU
