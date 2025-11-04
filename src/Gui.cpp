#include "Gui.hpp"
#include "../clay/clay.h"
#include "ImageLoader.hpp"
#include "color/catppuccin_colors.h"
#include <cstring> // Para strlen
#include <functional>
#include <iostream>
#include <raylib.h>
#include <vector>

static Clay_Color root = base_light;
static const int baseroundcorners = 6;

#include "Reproductor.hpp" // para tener globalQueue
// En tu loop de UI
static std::string currentTitle = "Sin titulo";
static std::string currentCover = "Sin titulo";

void actualizarUI()
{
  // Verificar si hay nueva información disponible
  if (globalInfoQueue.hasUpdate())
    {
      MusicInfo info = globalInfoQueue.get();

      // IMPORTANTE: Actualizar la variable estática
      currentTitle = info.title.empty() ? "Sin titulo" : info.title;

      // Debug (opcional)
      std::cout << "Estado: " << info.estado << "\n";
      std::cout << "Título: " << info.title << "\n";

      if (!info.artworkUrl.empty())
        {
          std::cout << "Carátula: " << info.artworkUrl << "\n";
        }
      currentCover = info.artworkUrl.empty() ? "Sin titulo" : info.artworkUrl;
    }
}

const char *getsongname()
{
  return currentTitle.c_str();
}

const char *getportada()
{
  return currentCover.erase(0, 7).c_str();
}

static void botonPlayPause()
{
  globalQueue.push([](Reproductor &rep, bool &running) {
    if (rep.getEstado() == "Playing")
      rep.pause();
    else
      rep.play();
  });
}

static void botonAvanzar()
{
  int segundos = 5;
  globalQueue.push([segundos](Reproductor &rep, bool &running) {
    int64_t tiempo_actual = rep.player.time();
    int64_t duracion = rep.player.length();
    int64_t nuevo_tiempo = tiempo_actual + (segundos * 1000);

    // Evitar pasar de la duración
    if (nuevo_tiempo < duracion)
      {
        rep.player.setTime(nuevo_tiempo);
        std::cout << "⏩ Avanzando " << segundos << " segundos\n";
      }
    else
      {
        rep.player.setTime(duracion - 1000);
        std::cout << "⏭️ Cerca del final\n";
      }
  });
}

static void botonRetroceder()
{
  int segundos = 5;
  globalQueue.push([segundos](Reproductor &rep, bool &running) {
    int64_t tiempo_actual = rep.player.time();
    int64_t nuevo_tiempo = tiempo_actual - (segundos * 1000);

    // Evitar ir a tiempo negativo
    if (nuevo_tiempo > 0)
      {
        rep.player.setTime(nuevo_tiempo);
        std::cout << "⏪ Retrocediendo " << segundos << " segundos\n";
      }
    else
      {
        rep.player.setTime(0);
        std::cout << "⏮️ Inicio de la canción\n";
      }
  });
}

// implementacion del widget boton
using ButtonCallback = std::function<void()>;

static void buttonCallbackWrapper(Clay_ElementId id, Clay_PointerData pointerdata, intptr_t userdata)
{
  if (pointerdata.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
    {
      ButtonCallback *callback = reinterpret_cast<ButtonCallback *>(userdata);
      if (callback && *callback)
        {
          (*callback)();
        }
    }
}
static void altbutton(Clay_String text, ButtonCallback callback)
{
  static std::vector<ButtonCallback> callbacks;
  callbacks.push_back(callback);

  Clay_Color actualcolor = yellow_dark;
  CLAY(.wrapped = {
         .layout = {
           .sizing = {.width = CLAY_SIZING_PERCENT(0.31)},
           .padding = {16, 16, 8, 8},
           .childGap = 32,
           .childAlignment = {.x = CLAY_ALIGN_X_CENTER},
         },
         .backgroundColor = actualcolor,
         .cornerRadius = baseroundcorners,
       })
  {
    Clay_OnHover(buttonCallbackWrapper, reinterpret_cast<intptr_t>(&callbacks.back()));
    CLAY_TEXT(text, CLAY_TEXT_CONFIG(.wrapped = {
                                       .textColor = text_light,
                                       .fontId = Gui::FONT_ID_BODY_16,
                                       .fontSize = 20,
                                     }));
  }
}
// Gui
void Gui::mylayout()
{
  actualizarUI();
  CLAY(.wrapped = {
         .id = CLAY_ID("root"),
         .layout =
           {
             .sizing =
               {
                 .width = CLAY_SIZING_GROW(),
                 .height = CLAY_SIZING_GROW(),
               },
             .padding = {12, 12, 12, 12},
             .childGap = 12,
             .childAlignment =
               {
                 // .x = CLAY_ALIGN_X_CENTER,
                 // .y = CLAY_ALIGN_Y_CENTER
               },
             .layoutDirection = CLAY_TOP_TO_BOTTOM,
           },
         .backgroundColor = root,
       })
  {
    CLAY(.wrapped = {.layout =
                       {
                         .sizing =
                           {
                             .width = CLAY_SIZING_PERCENT(1),
                             .height = CLAY_SIZING_PERCENT(0.7),
                           },
                         .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}},
                     .backgroundColor = {220, 138, 120, 255}})
    {
      // CLAY_TEXT(CLAY_STRING("UwU"),
      //           CLAY_TEXT_CONFIG(.wrapped = {
      //                              .textColor = {76, 79, 105, 255},
      //                              .fontId = FONT_ID_BODY_16,
      //                              .fontSize = 24,
      //                            }));
      // Crear Clay_String manualmente para contenido dinámico
      CLAY(.wrapped = {
             .layout = {.sizing = {.width = CLAY_SIZING_PERCENT(0.9), .height = CLAY_SIZING_PERCENT(0.9)}},
             .image = {.imageData = ImageLoader::GetImage("Music_IMG", getportada())},
           }){};
    }
    CLAY(.wrapped = {
           .id = CLAY_ID("panel"),
           .layout =
             {
               .sizing =
                 {
                   .width = CLAY_SIZING_GROW(),
                   .height = CLAY_SIZING_PERCENT(0.3),
                 },
               // .padding = {12, 12, 12, 12},
               .childGap = 12,
               .childAlignment =
                 {
                   // .x = CLAY_ALIGN_X_CENTER,
                   // .y = CLAY_ALIGN_Y_CENTER
                 },
               .layoutDirection = CLAY_TOP_TO_BOTTOM,
             },
           .backgroundColor = root,
         })
    {
      CLAY(.wrapped = {
             .id = CLAY_ID("Titulo"),
             .layout{
               .sizing =
                 {
                   .width = CLAY_SIZING_PERCENT(1),
                   .height = CLAY_SIZING_PERCENT(0.33),
                 },
               // .padding = {12, 12, 12, 12}
               .childAlignment = {
                 .x = CLAY_ALIGN_X_CENTER,
                 .y = CLAY_ALIGN_Y_CENTER,
               }},
             .backgroundColor = crust_light,
           })
      {
        // Crear Clay_String manualmente para contenido dinámico
        const char *songName = getsongname();
        Clay_String songString = {
          .length = (int)strlen(songName),
          .chars = songName};
        CLAY_TEXT(songString,

                  CLAY_TEXT_CONFIG(.wrapped = {

                                     .textColor = {76, 79, 105, 255},
                                     .fontId = FONT_ID_BODY_16,
                                     .fontSize = 24,
                                   }));
      }
      CLAY(.wrapped = {
             .id = CLAY_ID("Control"),
             .layout{
               .sizing =
                 {
                   .width = CLAY_SIZING_PERCENT(1),
                   .height = CLAY_SIZING_PERCENT(0.33),
                 },
               .childAlignment = {
                 .x = CLAY_ALIGN_X_CENTER,
                 .y = CLAY_ALIGN_Y_CENTER,
               }},
             .backgroundColor = crust_light,
           })
      {
        CLAY_TEXT(CLAY_STRING("---------------- "),
                  CLAY_TEXT_CONFIG(.wrapped = {

                                     .textColor = {76, 79, 105, 255},
                                     .fontId = FONT_ID_BODY_16,
                                     .fontSize = 24,
                                   }));
      }
      CLAY(.wrapped = {
             .id = CLAY_ID("Control4"),
             .layout{
               .sizing =
                 {
                   .width = CLAY_SIZING_PERCENT(1),
                   .height = CLAY_SIZING_PERCENT(0.33),
                 },
               .childGap = 16,
               .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}},
             .backgroundColor = crust_light,
           })
      {
        altbutton(CLAY_STRING("󰒮"), botonRetroceder);
        altbutton(CLAY_STRING(""), botonPlayPause);
        altbutton(CLAY_STRING("󰒭"), botonAvanzar);
      }
    }
    // CLAY(.wrapped = {
    //          .id = CLAY_ID("miniseparador"),
    //          .layout{
    //              .sizing =
    //                  {
    //                      .width = CLAY_SIZING_PERCENT(0.9),
    //                      .height = CLAY_SIZING_FIXED(12),
    //                  },
    //          },
    //          .backgroundColor = root,
    //      }) {}
  }
}
