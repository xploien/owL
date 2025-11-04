#include "Clay.hpp"
#include "Reproductor.hpp"
#include <iostream>
#include <thread>

int main(int argc, char *argv[])
{

  AppClay app;
  app.Initialize(380, 550, "oWl", FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT, "resources/ComicShannsMonoNerdFont-Regular.otf", 48);

  std::cout << "El motor Clay Iniciado\n";

  std::thread HiloMusica(MusicServer, argc, argv);

  std::cout << "El motor de Musica Iniciado\n";
  while (!WindowShouldClose())
    {
      Clay_RenderCommandArray rendercommands = app.CreateLayout();

      BeginDrawing();
      // ClearBackground({239, 241, 245, 255});
      app.Render(rendercommands);

      EndDrawing();
    }

  return 0;
}
