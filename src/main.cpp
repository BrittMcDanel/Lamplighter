#include "backward.h"
#include "engine.h"
#include "game.h"
#include "sprite.h"
#include "text.h"
#include "utils.h"
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define USE_EDITOR 1

#if USE_EDITOR
#include "editor.h"
#endif

using namespace std;
using namespace backward;

// convert -background none -fill black -font CodeFont.ttf -pointsize 16
// label:"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789\`\~\!@#$\%^&*()_+-={}|:\"<>?[]\\\;',./"
// fontatlas.png

int main(int argc, char *argv[]) {
  SignalHandling sh;
  StackTrace st;
  st.load_here(32);
  Printer p;

  if (argc < 3) {
    cout << "Usage: ./main <ipaddr:port> <--server>/<--client>" << endl;
    return 1;
  }

  // Get Server IP Address
  std::string server = std::string(argv[1]);
  bool is_host;
  if (std::string(argv[2]) == "--server") {
    is_host = true;
  } else {
    is_host = false;
  }

  Game *game = new Game();
  game->start(server, is_host);

#if USE_EDITOR
  game->editor_state.use_editor = true;
  auto editor = new Editor();
  editor->start(*game);
#endif

  // Main loop
  while (!game->engine.quit) {
    game->engine.clear();
    game->engine.update(*game);

    game->update();

#if USE_EDITOR
    // editor needs to be updated after game as it needs the data from the frame
    editor->update(*game);
#endif
    game->draw();

    // send game draw calls
    game->engine.present_render_buffer();

#if USE_EDITOR
    editor->draw(*game);
#endif

    // render
    SDL_GL_SwapWindow(game->engine.window);
    SDL_Delay(1000 / 60);
  }

  // Cleanup
#if USE_EDITOR
  editor->destroy();
  delete editor;
#endif

  // glfwDestroyWindow(game->engine.window);
  // glfwTerminate();

  SDL_GL_DeleteContext(game->engine.context);
  SDL_DestroyWindow(game->engine.window);
  SDL_Quit();

  game->stop();

  delete game;

  return 0;
}