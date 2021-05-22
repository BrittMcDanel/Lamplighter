#include "engine.h"
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "game.h"
#include "stb_image_write.h"
#include <SDL2/SDL_image.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Shader::Shader() {
  shader_name = ShaderName::None;
  shader_program_id = 0;
}

Shader::Shader(ShaderName _shader_name, GLuint _shader_program_id) {
  shader_name = _shader_name;
  shader_program_id = _shader_program_id;
}

ImageLocation::ImageLocation() {
  src = Rect();
  u1 = 0.0;
  v1 = 0.0;
  u2 = 0.0;
  v2 = 0.0;
}

ImageLocation::ImageLocation(Image image, Rect _src) {
  src = _src;
  // this information is the true src, it is the location in the
  // image to render.
  u1 = (float)_src.x / (float)image.image_dims.x;
  v1 = (float)_src.y / (float)image.image_dims.y;
  u2 = u1 + ((float)_src.w / (float)image.image_dims.x);
  v2 = v1 + ((float)_src.h / (float)image.image_dims.y);
}

SpriteSrc::SpriteSrc() { image_location = ImageLocation(); }

SpriteSrc::SpriteSrc(ImageLocation _image_location) {
  image_location = _image_location;
}

void SpriteSrc::update(Rect &scaled_dst) {
  // update verts, uvs with new x, y
  // vertices
  // top right
  vertices[0] = scaled_dst.x + scaled_dst.w;
  vertices[1] = scaled_dst.y;

  // bottom right
  vertices[2] = scaled_dst.x + scaled_dst.w;
  vertices[3] = scaled_dst.y + scaled_dst.h;

  // top left
  vertices[4] = scaled_dst.x;
  vertices[5] = scaled_dst.y;

  // bottom right
  vertices[6] = scaled_dst.x + scaled_dst.w;
  vertices[7] = scaled_dst.y + scaled_dst.h;

  // bottom left
  vertices[8] = scaled_dst.x;
  vertices[9] = scaled_dst.y + scaled_dst.h;

  // top left
  vertices[10] = scaled_dst.x;
  vertices[11] = scaled_dst.y;

  // uvs
  // top right
  uvs[0] = image_location.u2;
  uvs[1] = image_location.v2;

  // bottom right
  uvs[2] = image_location.u2;
  uvs[3] = image_location.v1;

  // top left
  uvs[4] = image_location.u1;
  uvs[5] = image_location.v2;

  // bottom right
  uvs[6] = image_location.u2;
  uvs[7] = image_location.v1;

  // bottom left
  uvs[8] = image_location.u1;
  uvs[9] = image_location.v1;

  // top left
  uvs[10] = image_location.u1;
  uvs[11] = image_location.v2;
}

int Font::get_adjusted_line_height() { return (int)(baseline * 1.2); }

void Engine::start() {
  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    printf("Error: %s\n", SDL_GetError());
    return;
  }

  int flags = IMG_INIT_PNG;
  int initted = IMG_Init(flags);
  if ((initted & flags) != flags) {
    printf("IMG_Init: Failed to init required png support!\n");
    printf("IMG_Init: %s\n", IMG_GetError());
    // handle error
    abort();
  }

  // Decide GL+GLSL versions
#if __APPLE__
  // GL 3.2 Core + GLSL 150
  glsl_version = "#version 150";
  SDL_GL_SetAttribute(
      SDL_GL_CONTEXT_FLAGS,
      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
  // GL 3.0 + GLSL 130
  glsl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

  // Create window with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_DisplayMode current;
  SDL_GetCurrentDisplayMode(0, &current);

  base_resolution.set(640, 360);
  auto init_scale = 2;
  game_resolution.set(base_resolution.x * init_scale,
                      base_resolution.y * init_scale);
  window_resolution.set(game_resolution.x, game_resolution.y);
  game_rect = Rect(0, 0, game_resolution.x, game_resolution.y);
  scale = game_resolution.x / base_resolution.x;

  window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            window_resolution.x, window_resolution.y,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
                                SDL_WINDOW_MAXIMIZED);
  context = SDL_GL_CreateContext(window);

  SDL_GL_MakeCurrent(window, context);

  // enable VSync
  SDL_GL_SetSwapInterval(1);

  auto glewExperimental = GL_TRUE;
  bool err = glewInit() != GLEW_OK;
  if (err) {
    cout << "glew initialization error.\n";
    abort();
  }
  // initialize OpenGL buffers
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ubo);
  glBindVertexArray(vao);

  // OpenGL setup
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_SCISSOR_TEST);

  glViewport(0, 0, window_resolution.x, window_resolution.y);

  // init c++ 17 rnd
  random_device rd;
  mt19937 mt(rd());
  rnd = mt;

  // load cursors
  load_cursors();

  // load textures
  load_images();

  // load default shader
  load_default_shader();

  // load fonts
  load_fonts();

  // set default shader as the active shader
  set_active_shader(ShaderName::Default);
}

void Engine::set_cursor(CursorType _cursor_type) {
  cursor_at_end_of_frame = _cursor_type;
}

void Engine::change_cursor_to_end_of_frame_cursor() {
  if (current_cursor_type == cursor_at_end_of_frame) {
    return;
  }
  current_cursor_type = cursor_at_end_of_frame;
  switch (current_cursor_type) {
  case CursorType::Default: {
    SDL_SetCursor(cursor);
    break;
  }
  case CursorType::Hand: {
    SDL_SetCursor(cursor_hand);
    break;
  }
  case CursorType::Sword: {
    SDL_SetCursor(cursor_sword);
    break;
  }
  case CursorType::SwordInvalid: {
    SDL_SetCursor(cursor_sword_invalid);
    break;
  }
  case CursorType::Invalid: {
    SDL_SetCursor(cursor_invalid);
    break;
  }
  default: {
    cout << "Engine::set_cursor: cursor type not handled "
         << static_cast<int>(current_cursor_type);
    abort();
    break;
  }
  }
}

SDL_Cursor *Engine::load_cursor(const char *file_path) {
  SDL_Surface *cursor_surface = IMG_Load(file_path);
  if (!cursor_surface) {
    printf("set cursor error");
    abort();
  }
  SDL_Cursor *_cursor = SDL_CreateColorCursor(cursor_surface, 0, 0);
  if (!_cursor) {
    printf("set cursor error");
    abort();
  }
  SDL_FreeSurface(cursor_surface);
  return _cursor;
}

void Engine::load_cursors() {
  cursor = load_cursor("../assets/ui/cursor.png");
  cursor_hand = load_cursor("../assets/ui/cursor-hand.png");
  cursor_sword = load_cursor("../assets/ui/cursor-sword.png");
  cursor_sword_invalid = load_cursor("../assets/ui/cursor-sword-invalid.png");
  cursor_invalid = load_cursor("../assets/ui/cursor-invalid.png");
  current_cursor_type = CursorType::Default;
  SDL_SetCursor(cursor);
}

int Engine::get_random_int(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  return dist(rnd);
}

uint32_t Engine::get_random_uint32(uint32_t min, uint32_t max) {
  std::uniform_int_distribution<uint32_t> dist(min, max);
  return dist(rnd);
}

uint64_t Engine::get_random_uint64(uint64_t min, uint64_t max) {
  std::uniform_int_distribution<uint64_t> dist(min, max);
  return dist(rnd);
}

double Engine::get_random_double(double min, double max) {
  std::uniform_real_distribution<double> dist(min, max);
  return dist(rnd);
}

boost::uuids::uuid Engine::get_guid() { return uuid_gen(); }

void Engine::start_clipping(const Rect &scaled_rect) {
  // present everything before this clip is applied,
  // it will clip previous renders.
  present_render_buffer();
  glEnable(GL_SCISSOR_TEST);
  auto clip_rect = game_rect;
  clip_rect.x += scaled_rect.x;
  clip_rect.y += scaled_rect.y;
  clip_rect.w = scaled_rect.w;
  clip_rect.h = scaled_rect.h;
  glScissor(clip_rect.x, clip_rect.y, clip_rect.w, clip_rect.h);
}

void Engine::end_clipping() {
  // present clipped renders and then disable clipping.
  present_render_buffer();
  glDisable(GL_SCISSOR_TEST);
}

void Engine::update(Game &game) {
  // get the window size every frame and adjust the game_rect/viewport.
  // these calls seem to be very fast, don't notice any fps difference
  // even though its being done every frame.
  auto prev_window_resolution = window_resolution;
  SDL_GetWindowSize(window, &window_resolution.x, &window_resolution.y);
  scale = game_resolution.x / base_resolution.x;
  game_rect_top_padding = 40;
  game_rect.x = (window_resolution.x - game_resolution.x) / 2;
  // + 20 to leave some room for the navbar in editor
  game_rect.y =
      (window_resolution.y - game_resolution.y) - game_rect_top_padding;
  game_rect.w = game_resolution.x;
  game_rect.h = game_resolution.y;
  game_rect_scaled = game_rect;
  game_rect_scaled.x /= scale;
  game_rect_scaled.y /= scale;
  game_rect_scaled.w /= scale;
  game_rect_scaled.h /= scale;
  // literally the origin (bottom-left) and bounding box of drawn grapics.
  // do not even need to clip all drawn sprites are within this box
  // automatically.
  glViewport(game_rect.x, game_rect.y, game_rect.w, game_rect.h);

  fps += 1;
  auto prev_current_time = current_time;
  current_time = SDL_GetTicks();
  delta_time = current_time - prev_current_time;
  if (current_time - prev_frame_time >= 1000) {
    prev_frame_time = current_time;
    printf("fps: %d\n", fps);
    fps = 0;
  }
  SDL_GetMouseState(&mouse_point.x, &mouse_point.y);
  // set in case both x and y are zero, then scale if either
  // aren't
  mouse_point_scaled = mouse_point;
  if (mouse_point.x > 0) {
    mouse_point_scaled.x = mouse_point.x / scale;
  }
  if (mouse_point.y > 0) {
    mouse_point_scaled.y = mouse_point.y / scale;
  }
  mouse_point_opengl_origin = mouse_point;
  mouse_point_opengl_origin.y = window_resolution.y - mouse_point.y;
  mouse_point_opengl_origin_scaled = mouse_point_opengl_origin;
  if (mouse_point_opengl_origin.x > 0) {
    mouse_point_opengl_origin_scaled.x = mouse_point_opengl_origin.x / scale;
  }
  if (mouse_point_opengl_origin.y > 0) {
    mouse_point_opengl_origin_scaled.y = mouse_point_opengl_origin.y / scale;
  }
  mouse_point_opengl_origin_scaled_camera = mouse_point_opengl_origin_scaled;
  mouse_point_opengl_origin_scaled_camera.x += camera.dst.x;
  mouse_point_opengl_origin_scaled_camera.y += camera.dst.y;

  mouse_in_game_rect =
      rect_contains_point(game_rect_scaled, mouse_point_opengl_origin_scaled);
  if (mouse_in_game_rect) {
    auto diff_x_scaled =
        mouse_point_opengl_origin_scaled.x - game_rect_scaled.x;
    auto diff_y_scaled =
        mouse_point_opengl_origin_scaled.y - game_rect_scaled.y;
    mouse_point_game_rect_scaled.set(diff_x_scaled, diff_y_scaled);
    mouse_point_game_rect_scaled_camera = mouse_point_game_rect_scaled;
    mouse_point_game_rect_scaled_camera.x += camera.dst.x;
    mouse_point_game_rect_scaled_camera.y += camera.dst.y;
  } else {
    // invalid mouse coords, if you set it to 0, 0 a unit at 0, 0 could
    // potentially get an input event.
    mouse_point_game_rect_scaled.set(-1, -1);
    mouse_point_game_rect_scaled_camera.set(-1, -1);
  }

  is_mouse_down = false;
  is_mouse_up = false;
  is_right_mouse_down = false;
  is_right_mouse_up = false;
  is_mouse_wheel_up = false;
  is_mouse_wheel_down = false;
  for (int i = 0; i < NUM_KEYS; i++) {
    keys_down[i] = false;
    keys_up[i] = false;
  }
  text_input_this_frame.clear();

  // handle the event loop here instead of in engine so that
  // imgui can be handled in once place
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (game.editor_state.use_editor) {
      ImGui_ImplSDL2_ProcessEvent(&e);
    }
    switch (e.type) {
    case SDL_QUIT: {
      quit = true;
      break;
    }
    case SDL_MOUSEBUTTONDOWN: {
      if (e.button.button == SDL_BUTTON_LEFT) {
        is_mouse_down = true;
        is_mouse_held_down = true;
      }
      if (e.button.button == SDL_BUTTON_RIGHT) {
        is_right_mouse_down = true;
        is_right_mouse_held_down = true;
      }
      break;
    }
    case SDL_MOUSEBUTTONUP: {
      if (e.button.button == SDL_BUTTON_LEFT) {
        is_mouse_held_down = false;
        is_mouse_up = true;
      }
      if (e.button.button == SDL_BUTTON_RIGHT) {
        is_right_mouse_held_down = false;
        is_right_mouse_up = true;
      }
      break;
    }
    case SDL_MOUSEWHEEL: {
      if (e.wheel.y == -1) {
        is_mouse_wheel_down = true;
      } else if (e.wheel.y == 1) {
        is_mouse_wheel_up = true;
      }
      break;
    }
    case SDL_TEXTINPUT: {
      text_input_this_frame += e.text.text;
      // printf("text input %s\n", text_input_this_frame.c_str());
      break;
    }
    case SDL_KEYDOWN: {
      auto key_idx = (int)e.key.keysym.sym;
      if (key_idx < 0 || key_idx > NUM_KEYS - 1) {
        break;
      }
      keys_down[e.key.keysym.sym] = true;
      keys_held_down[e.key.keysym.sym] = true;
      break;
    }
    case SDL_KEYUP: {
      auto key_idx = (int)e.key.keysym.sym;
      if (key_idx < 0 || key_idx > NUM_KEYS - 1) {
        break;
      }
      keys_held_down[e.key.keysym.sym] = false;
      keys_up[e.key.keysym.sym] = true;
      break;
    }
    }
  }
}

void Engine::clear() {
  // int display_w, display_h;
  // glViewport(0, 0, display_w, display_h);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Engine::clear_render_buffer() {
  num_draw_calls = 0;
  vertices.clear();
  uvs.clear();
}

void Engine::push_to_render_buffer(short *_vertices_to_add, int _vertices_size,
                                   float *_uvs_to_add, int _uvs_size) {
  num_draw_calls += 1;
  for (int i = 0; i < _vertices_size; i++) {
    vertices.push_back(_vertices_to_add[i]);
  }
  for (int i = 0; i < _uvs_size; i++) {
    uvs.push_back(_uvs_to_add[i]);
  }
}

// sends verts/uvs for the active texture to the gpu
void Engine::present_render_buffer() {
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(short) * vertices.size(), &vertices[0],
               GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, 2 * sizeof(short), 0);

  glBindBuffer(GL_ARRAY_BUFFER, ubo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * uvs.size(), &uvs[0],
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(GLfloat), 0);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glBindVertexArray(vao);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(short) * vertices.size(),
                  &vertices[0]);
  glDrawArrays(GL_TRIANGLES, 0, num_draw_calls * 6);

  // all the data is sent, clear render buffer to prepare for new draw calls
  clear_render_buffer();
}

void Engine::set_active_shader(ShaderName _shader_name) {
  glUseProgram(get_shader(_shader_name).shader_program_id);
}

void Engine::set_active_image(Image &image) {
  if (image.texture_id == current_texture_id) {
    return;
  }
  current_texture_id = image.texture_id;

  // send verts/uvs for the prev active texture to the gpu
  present_render_buffer();
  // clear the render buffer
  clear_render_buffer();
  // set the new texture as active
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, image.texture_id);
}

void Engine::load_images() {
  load_image(ImageName::UI, "../assets/ui/ui.png");
  load_image(ImageName::Tiles, "../assets/tiles/tiles.png");
  load_image(ImageName::Units, "../assets/units/units.png");
  load_image(ImageName::Abilities, "../assets/abilities/abilities.png");
  load_image(ImageName::AbilityIcons, "../assets/abilities/ability_icons.png");
  load_image(ImageName::Buildings, "../assets/buildings/buildings.png");
  load_image(ImageName::Misc, "../assets/misc/misc.png");
  load_image(ImageName::Items, "../assets/items/items.png");
  load_image(ImageName::TreasureChests,
             "../assets/treasurechests/treasurechests.png");
  load_image(ImageName::AbilityTargets,
             "../assets/abilities/ability_targets.png");
}

Image Engine::load_image(ImageName _image_name, const char *_image_path) {
  GLuint texture_id;
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  int width, height;
  // load image using SOIL and store it on GPU
  {
    int n;
    unsigned char *image = stbi_load(_image_path, &width, &height, &n, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image);

    if (image == 0) {
      printf("Failed to load texture image. %s\n", _image_path);
      exit(1);
    } else {
      stbi_image_free(image);
    }
  }

  Image image;
  image.image_name = _image_name;
  image.texture_id = texture_id;
  image.image_dims.set(width, height);
  images.push_back(image);

  // return a copy of the image for fonts
  return image;
}

Image Engine::get_image(ImageName _image_name) {
  if (_image_name == ImageName::None) {
    "Engine::get_image. ImageName::None was passed in.\n";
    abort();
  }
  for (auto &texture : images) {
    if (texture.image_name == _image_name) {
      return texture;
    }
  }
  cout << "Engine::get_texture. Texture not found. " << (int)_image_name
       << "\n";
  abort();
  return images.at(0);
}

void Engine::load_default_shader() {
  // uses half window dimensions, not sure why, probalby must be redone
  // everytime the window resolution changes.
  string vertexShader = "#version 330\n"
                        "layout (location = 0) in vec2 vert;\n"
                        "layout (location = 1) in vec2 _uv;\n"
                        "out vec2 uv;\n"
                        "void main()\n"
                        "{\n"
                        "    uv = _uv;\n"
                        "    gl_Position = vec4(vert.x / " +
                        to_string(window_resolution.x / 2) +
                        " - 1.0, vert.y / " +
                        to_string(window_resolution.y / 2) +
                        " - 1.0, 0.0, "
                        "1.0);\n"
                        "}\n";

  string fragmentShader = "#version 330\n"
                          "out vec4 color;\n"
                          "in vec2 uv;\n"
                          "uniform sampler2D tex;\n"
                          "void main()\n"
                          "{\n"
                          "    color = texture(tex, uv);\n"
                          "}\n";

  // make and add the shader
  auto shader_program_id =
      getShaderProgramId(vertexShader.c_str(), fragmentShader.c_str());
  auto default_shader_idx = get_shader_idx(ShaderName::Default);
  auto default_shader = Shader(ShaderName::Default, shader_program_id);

  // replace or add default shader
  if (default_shader_idx != -1) {
    shaders[default_shader_idx] = default_shader;
  } else {
    shaders.push_back(default_shader);
  }
}

Shader Engine::get_shader(ShaderName _shader_name) {
  for (auto &shader : shaders) {
    if (shader.shader_name == _shader_name) {
      return shader;
    }
  }
  cout << "Engine::get_shader. Shader not found.\n";
  abort();
  return shaders.at(0);
}

int Engine::get_shader_idx(ShaderName _shader_name) {
  for (size_t i = 0; i < shaders.size(); i++) {
    if (shaders[i].shader_name == _shader_name) {
      return i;
    }
  }
  return -1;
}

GLuint compileShader(const GLchar *source, GLuint shaderType) {
  GLuint shaderHandler;

  shaderHandler = glCreateShader(shaderType);
  glShaderSource(shaderHandler, 1, &source, 0);
  glCompileShader(shaderHandler);

  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(shaderHandler, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shaderHandler, 512, 0, infoLog);
    printf("Error in compilation of shader:\n%s\n", infoLog);
    exit(1);
  };

  return shaderHandler;
}

GLuint getShaderProgramId(const char *vertexFile, const char *fragmentFile) {
  GLuint programId, vertexHandler, fragmentHandler;

  vertexHandler = compileShader(vertexFile, GL_VERTEX_SHADER);
  fragmentHandler = compileShader(fragmentFile, GL_FRAGMENT_SHADER);

  programId = glCreateProgram();
  glAttachShader(programId, vertexHandler);
  glAttachShader(programId, fragmentHandler);
  glLinkProgram(programId);

  GLint success;
  GLchar infoLog[512];
  glGetProgramiv(programId, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(programId, 512, 0, infoLog);
    printf("Error in linking of shaders:\n%s\n", infoLog);
    exit(1);
  }

  glDeleteShader(vertexHandler);
  glDeleteShader(fragmentHandler);

  return programId;
}

void Engine::load_font(const char *_font_file_path_no_extension,
                       FontColor _font_color) {
  auto base_file_path = string(_font_file_path_no_extension);
  auto fnt_file_path = base_file_path + ".fnt";
  auto png_file_path = base_file_path + ".png";
  ifstream file(fnt_file_path.c_str());
  if (!file.good()) {
    cout << "Engine::get_font_char_info_from_file. File error "
         << fnt_file_path.c_str() << "\n";
    abort();
  }
  Font font;
  font.image = load_image(ImageName::FontAtlas, png_file_path.c_str());
  font.font_color = _font_color;
  // add num chars char_infos to font so that the info can be retrieved
  // through random access.
  for (size_t i = 0; i < NUM_CHARS_IN_FONT; i++) {
    font.char_infos.push_back(CharInfo());
  }
  auto parse_mode = FontParseMode::None;
  string line;
  int line_number = -1;
  while (getline(file, line)) {
    // cout << line << endl;
    line_number += 1;
    // cout << line_number << " " << line << "\n";
    if (line_number == 0) {
      auto split = string_split(line, ' ');
      for (auto &s : split) {
        if (s.find("=") != string::npos) {
          auto split_equals = string_split(s, '=');
          auto num_key = split_equals.at(0);
          auto num_val_str = split_equals.at(1);
          // can't stoi everything as sometimes the value is not an int
          if (num_key == "size") {
            font.font_size = stoi(num_val_str);
          } else if (num_key == "bold" && num_val_str == "1") {
            font.font_style = FontStyle::Bold;
          } else if (num_key == "italic" && num_val_str == "1") {
            font.font_style = FontStyle::Italic;
          } else if (num_key == "padding") {
            auto comma_split = string_split(num_val_str, ',');
            GAME_ASSERT(comma_split.size() == 4);
            font.pl = stoi(comma_split[0]);
            font.pr = stoi(comma_split[1]);
            font.pt = stoi(comma_split[2]);
            font.pb = stoi(comma_split[3]);
          } else if (num_key == "spacing") {
            auto comma_split = string_split(num_val_str, ',');
            GAME_ASSERT(comma_split.size() == 2);
            font.spacing.x = stoi(comma_split[0]);
            font.spacing.y = stoi(comma_split[1]);
            // I'm thinking font spacing should be 0, 0 right now.
            // padding can be > 0
            if (font.spacing.x != 0) {
              cout << "Engine::load_font. Font spacing x is not 0. Font "
                      "spacing x: "
                   << font.spacing.x << "\n";
              abort();
            } else if (font.spacing.y != 0) {
              cout << "Engine::load_font. Font spacing y is not 0. Font "
                      "spacing y: "
                   << font.spacing.y << "\n";
              abort();
            }
          }
        }
      }
      continue;
    } else if (line_number == 1) {
      auto split = string_split(line, ' ');
      for (auto &s : split) {
        if (s.find("=") != string::npos) {
          auto split_equals = string_split(s, '=');
          auto num_key = split_equals.at(0);
          auto num_val_str = split_equals.at(1);
          // can't stoi everything as sometimes the value is not an int
          if (num_key == "lineHeight") {
            font.line_height = stoi(num_val_str);
          } else if (num_key == "base") {
            font.baseline = stoi(num_val_str);
          }
        }
      }
      continue;
    } else if (line_number == 2) {
      continue;
    } else if (line_number == 3) {
      parse_mode = FontParseMode::Chars;
      auto num_chars = 0;

      auto split = string_split(line, '=');
      auto num_chars_str = split.at(1);
      string_trim(num_chars_str);
      num_chars = stoi(num_chars_str);
      if (num_chars > NUM_CHARS_IN_FONT) {
        cout << "Engine::load_font. num chars is > NUM_CHARS_IN_FONT. "
             << num_chars << " > " << NUM_CHARS_IN_FONT << "\n";
        abort();
      }
      // cout << "num chars " << num_chars_str << " " << num_chars_str.size() <<
      // "\n "; continue as the next line begins the char info section
      continue;
    } else if (line.find("kernings count=") != string::npos) {
      parse_mode = FontParseMode::Kerning;
      // continue as the next line begins the kerning info section
      continue;
    }

    if (parse_mode == FontParseMode::Chars) {
      auto split = string_split(line, ' ');
      // 1 char info per line
      CharInfo char_info;
      for (auto &s : split) {
        if (s.find("=") != string::npos) {
          auto split_equals = string_split(s, '=');
          auto num_key = split_equals.at(0);
          auto num_val_str = split_equals.at(1);
          auto num_val = stoi(num_val_str);
          // cout << "num key val " << num_key << " " << num_val << "\n";
          if (num_key == "id") {
            char_info.id = num_val;
          } else if (num_key == "x") {
            char_info.x = num_val;
          } else if (num_key == "y") {
            char_info.y = num_val;
          } else if (num_key == "width") {
            char_info.w = num_val;
          } else if (num_key == "height") {
            char_info.h = num_val;
          } else if (num_key == "xoffset") {
            char_info.xoffset = num_val;
          } else if (num_key == "yoffset") {
            char_info.yoffset = num_val;
          } else if (num_key == "xadvance") {
            char_info.xadvance = num_val;
          } else if (num_key == "page") {
            char_info.page = num_val;
          } else if (num_key == "chnl") {
            char_info.chnl = num_val;
          } else {
            cout << "char_info key not handled. " << num_key << "\n";
            abort();
          }
        }
      }
      // adjust for padding (for fonts with outlines and shadows that have a
      // padding greater than 0)
      auto padding_lr = font.pl + font.pr;
      char_info.xadvance -= padding_lr;
      /*cout << "char_info " << char_info.id << " " << char_info.x << " " <<
         char_info.y << " "
           << char_info.w << " " << char_info.h << " " << char_info.xoffset << "
         "
           << char_info.yoffset << " " << char_info.xadvance << " " <<
         char_info.page << " "
           << char_info.chnl << "\n";*/
      // set char info
      // add sprite src to char_info
      char_info.src =
          SpriteSrc(ImageLocation(font.image, Rect(char_info.x, char_info.y,
                                                   char_info.w, char_info.h)));
      GAME_ASSERT(char_info.id >= 0 &&
                  char_info.id <= (int)font.char_infos.size() - 1);
      font.char_infos.at(char_info.id) = char_info;
    } else if (parse_mode == FontParseMode::Kerning) {
      auto split = string_split(line, ' ');
      // 1 kerning info per line
      KerningPair kerning;
      for (auto &s : split) {
        if (s.find("=") != string::npos) {
          auto split_equals = string_split(s, '=');
          auto num_key = split_equals.at(0);
          auto num_val_str = split_equals.at(1);
          auto num_val = stoi(num_val_str);
          if (num_key == "first") {
            kerning.first = num_val;
          } else if (num_key == "second") {
            kerning.second = num_val;
          } else if (num_key == "amount") {
            kerning.amount = num_val;
          }
        }
      }
      // cout << "kern " << kerning.first << " " << kerning.second << " " <<
      // kerning.amount << "\n";
      GAME_ASSERT(kerning.first <= (int)font.char_infos.size() - 1);
      font.char_infos.at(kerning.first).kerning_pairs.push_back(kerning);
    }
  }

  // add font to fonts
  fonts.push_back(font);
}

void Engine::draw_string(Font &font, string &val, Vec2 dst) {
  set_active_image(font.image);
  auto dst_copy = dst;
  int kerning_pair_amount = 0;
  for (size_t i = 0; i < val.size(); i++) {
    char c = val[i];
    if (i < val.size() - 1) {
      kerning_pair_amount = get_kerning_pair_amount(font, c, val[i + 1]);
    } else {
      kerning_pair_amount = 0;
    }
    auto &char_info = font.char_infos.at((int)c);
    // cout << c << " " << (int)c << " " << char_info.yoffset << "\n";
    auto char_aligned_to_top_y = dst_copy.y - char_info.h;
    auto dst_rect = Rect(dst_copy.x + char_info.xoffset,
                         char_aligned_to_top_y - char_info.yoffset, char_info.w,
                         char_info.h);
    char_info.src.update(dst_rect);
    push_to_render_buffer(char_info.src.vertices, 12, char_info.src.uvs, 12);
    dst_copy.x += char_info.xadvance + kerning_pair_amount;
  }
}

// called when drawing one char at a time
void Engine::set_char_dsts(Font &font, string &val, vector<CharDst> &char_dsts,
                           Vec2 dst) {
  auto dst_copy = dst;
  int kerning_pair_amount = 0;
  for (size_t i = 0; i < val.size(); i++) {
    char c = val[i];
    if (i < val.size() - 1) {
      kerning_pair_amount = get_kerning_pair_amount(font, c, val[i + 1]);
    } else {
      kerning_pair_amount = 0;
    }
    auto &char_info = font.char_infos.at((int)c);
    auto char_aligned_to_top_y = dst_copy.y - char_info.h;
    auto dst_rect = Rect(dst_copy.x + char_info.xoffset,
                         char_aligned_to_top_y - char_info.yoffset, char_info.w,
                         char_info.h);
    char_dsts.push_back(CharDst(c, dst_rect));
    dst_copy.x += char_info.xadvance + kerning_pair_amount;
  }
}

// called when drawing one char at a time
void Engine::draw_char_dsts(Font &font, vector<CharDst> &char_dsts,
                            int stop_at_idx) {
  set_active_image(font.image);
  for (int i = 0; i < stop_at_idx; i++) {
    auto &char_dst = char_dsts[i];
    auto &char_info = font.char_infos.at((int)char_dst.c);
    char_info.src.update(char_dst.dst);
    push_to_render_buffer(char_info.src.vertices, 12, char_info.src.uvs, 12);
  }
}

Vec2 Engine::measure_string(Font &font, string &val) {
  auto dst = Vec2(0, 0);
  dst.y = font.line_height;
  int kerning_pair_amount = 0;
  for (size_t i = 0; i < val.size(); i++) {
    char c = val[i];
    if (i < val.size() - 1) {
      kerning_pair_amount = get_kerning_pair_amount(font, c, val[i + 1]);
    } else {
      kerning_pair_amount = 0;
    }
    auto &char_info = font.char_infos.at((int)c);
    auto char_aligned_to_top_y = dst.y - char_info.h;
    auto x = dst.x + char_info.xoffset;
    auto y = char_aligned_to_top_y - char_info.yoffset;
    auto w = char_info.w;
    auto h = char_info.h;
    auto dst_rect = Rect(x, y, w, h);
    dst.x += char_info.xadvance + kerning_pair_amount;
  }
  return dst;
}

int Engine::get_kerning_pair_amount(Font &font, char first, char second) {
  auto &char_info = font.char_infos.at(first);
  for (auto &kerning_pair : char_info.kerning_pairs) {
    if (kerning_pair.second == second) {
      return kerning_pair.amount;
    }
  }
  // no kerning needed for character pair
  return 0;
}

void Engine::load_fonts() {
  load_font("../assets/fonts/atlases/white/fs10", FontColor::White);
  load_font("../assets/fonts/atlases/white/fs20", FontColor::White);
  load_font("../assets/fonts/atlases/white/fs30", FontColor::White);
  load_font("../assets/fonts/atlases/white/fs40", FontColor::White);
  load_font("../assets/fonts/atlases/black/fs10", FontColor::Black);
  load_font("../assets/fonts/atlases/black/fs20", FontColor::Black);
  load_font("../assets/fonts/atlases/black/fs30", FontColor::Black);
  load_font("../assets/fonts/atlases/black/fs40", FontColor::Black);
  load_font("../assets/fonts/atlases/red/fs10", FontColor::Red);
  load_font("../assets/fonts/atlases/red/fs20", FontColor::Red);
  load_font("../assets/fonts/atlases/red/fs30", FontColor::Red);
  load_font("../assets/fonts/atlases/red/fs40", FontColor::Red);
  load_font("../assets/fonts/atlases/white_shadow/fs10",
            FontColor::WhiteShadow);
  load_font("../assets/fonts/atlases/white_shadow/fs20",
            FontColor::WhiteShadow);
  load_font("../assets/fonts/atlases/white_shadow/fs30",
            FontColor::WhiteShadow);
  load_font("../assets/fonts/atlases/white_shadow/fs40",
            FontColor::WhiteShadow);
  load_font("../assets/fonts/atlases/red_shadow/fs10", FontColor::RedShadow);
  load_font("../assets/fonts/atlases/red_shadow/fs20", FontColor::RedShadow);
  load_font("../assets/fonts/atlases/red_shadow/fs30", FontColor::RedShadow);
  load_font("../assets/fonts/atlases/red_shadow/fs40", FontColor::RedShadow);
  load_font("../assets/fonts/atlases/red_gradient/fs10",
            FontColor::RedGradient);
  load_font("../assets/fonts/atlases/red_gradient/fs20",
            FontColor::RedGradient);
  load_font("../assets/fonts/atlases/red_gradient/fs30",
            FontColor::RedGradient);
  load_font("../assets/fonts/atlases/red_gradient/fs40",
            FontColor::RedGradient);
}

int Engine::get_font_handle(int _font_size, FontColor _font_color) {
  for (size_t i = 0; i < fonts.size(); i++) {
    auto &font = fonts[i];
    if (font.font_size == _font_size && font.font_color == _font_color) {
      return i;
    }
  }
  cout << "Engine::get_font_handle. font not found. font size: " << _font_size
       << " font color enum " << (int)_font_color << "\n";
  abort();
  return -1;
}