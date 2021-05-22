#ifndef ENGINE_H
#define ENGINE_H

#define GL_GLEXT_PROTOTYPES

#include <GL/glew.h>

#include "camera.h"
#include "constants.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include "stb_truetype.h"
#include "utils.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace std;

#define NUM_KEYS 322
#define NUM_CHARS_IN_FONT 128

struct Game;

struct Image {
  ImageName image_name = ImageName::None;
  GLuint texture_id = 0;
  Vec2 image_dims = Vec2(0, 0);
};

struct Shader {
  ShaderName shader_name = ShaderName::None;
  GLuint shader_program_id = 0;
  Shader();
  Shader(ShaderName _shader_name, GLuint _shader_program_id);
};

struct ImageLocation {
  Rect src;
  float u1 = 0.0;
  float v1 = 0.0;
  float u2 = 0.0;
  float v2 = 0.0;
  ImageLocation();
  ImageLocation(Image image, Rect _src);
};

struct SpriteSrc {
  ImageLocation image_location = ImageLocation();
  short vertices[12];
  float uvs[12];
  SpriteSrc();
  SpriteSrc(ImageLocation _image_location);
  void update(Rect &scaled_dst);
};

struct KerningPair {
  int first = 0;
  int second = 0;
  int amount = 0;
};

struct CharInfo {
  int id = 0;
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  int xoffset = 0;
  int yoffset = 0;
  int xadvance = 0;
  int page = 0;
  int chnl = 0;
  vector<KerningPair> kerning_pairs = vector<KerningPair>();
  SpriteSrc src = SpriteSrc();
};

struct Font {
  Image image = Image();
  FontColor font_color = FontColor::None;
  string png_file_name = "";
  vector<CharInfo> char_infos = vector<CharInfo>();
  int pl = 0;
  int pr = 0;
  int pt = 0;
  int pb = 0;
  Vec2 spacing = Vec2(0, 0);
  int font_size = 1;
  FontStyle font_style = FontStyle::Normal;
  int line_height = 1;
  int baseline = 1;
  int get_adjusted_line_height();
};

struct Engine {
  CursorType current_cursor_type = CursorType::Default;
  CursorType cursor_at_end_of_frame = CursorType::Default;
  SDL_Cursor *cursor = nullptr;
  SDL_Cursor *cursor_hand = nullptr;
  SDL_Cursor *cursor_sword = nullptr;
  SDL_Cursor *cursor_sword_invalid = nullptr;
  SDL_Cursor *cursor_invalid = nullptr;
  Camera camera = Camera();
  mt19937 rnd;
  boost::uuids::random_generator uuid_gen;
  boost::uuids::string_generator string_gen;
  Font temp_font;
  unsigned char ttf_buffer[1 << 20];
  unsigned char temp_bitmap[512 * 512];
  string glsl_version = "";
  GLuint current_texture_id = 0;
  int frames = 0;
  Vec2 base_resolution = Vec2();
  Vec2 game_resolution = Vec2();
  Vec2 window_resolution = Vec2();
  int game_rect_top_padding;
  Rect game_rect = Rect();
  Rect game_rect_scaled = Rect();
  bool initial_window_resolution_received = false;
  int scale = 1;
  SDL_Window *window = nullptr;
  SDL_GLContext context;
  bool mouse_in_game_rect = false;
  Vec2 mouse_point;
  Vec2 mouse_point_scaled;
  Vec2 mouse_point_opengl_origin;
  Vec2 mouse_point_opengl_origin_scaled;
  Vec2 mouse_point_opengl_origin_scaled_camera;
  Vec2 mouse_point_game_rect_scaled;
  Vec2 mouse_point_game_rect_scaled_camera;
  bool is_mouse_down;
  bool is_mouse_held_down;
  bool is_mouse_up;
  bool is_right_mouse_down;
  bool is_right_mouse_held_down;
  bool is_right_mouse_up;
  bool is_mouse_wheel_up;
  bool is_mouse_wheel_down;
  bool keys_down[NUM_KEYS];
  bool keys_held_down[NUM_KEYS];
  bool keys_up[NUM_KEYS];
  string text_input_this_frame;
  Uint32 current_time;
  Uint32 prev_frame_time;
  int delta_time;
  int fps;
  bool quit;
  vector<Image> images = vector<Image>();
  vector<Shader> shaders = vector<Shader>();
  vector<Font> fonts = vector<Font>();
  // render buffers
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint ubo = 0;
  int num_draw_calls = 0;
  vector<short> vertices = vector<short>();
  vector<float> uvs = vector<float>();
  void start();
  void update(Game &game);
  void clear();
  void clear_render_buffer();
  void push_to_render_buffer(short *_vertices_to_add, int _vertices_size,
                             float *_uvs_to_add, int _uvs_size);
  void present_render_buffer();
  void set_active_shader(ShaderName _shader_name);
  void set_active_image(Image &image);
  void set_cursor(CursorType _cursor_type);
  void change_cursor_to_end_of_frame_cursor();
  SDL_Cursor *load_cursor(const char *file_path);
  void load_cursors();
  void load_images();
  Image load_image(ImageName _image_name, const char *_image_path);
  void load_default_shader();
  void load_fonts();
  Image get_image(ImageName _image_name);
  Shader get_shader(ShaderName _shader_name);
  int get_shader_idx(ShaderName _shader_name);
  void load_font(const char *_font_file_path_no_extension,
                 FontColor _font_color);
  void draw_string(Font &font, string &val, Vec2 dst);
  Vec2 measure_string(Font &font, string &val);
  void set_char_dsts(Font &font, string &val, vector<CharDst> &char_dsts,
                     Vec2 dst);
  void draw_char_dsts(Font &font, vector<CharDst> &char_dsts, int stop_at_idx);
  int get_kerning_pair_amount(Font &font, char first, char second);
  int get_font_handle(int _font_size, FontColor _font_color);
  int get_random_int(int min, int max);
  uint32_t get_random_uint32(uint32_t min, uint32_t max);
  uint64_t get_random_uint64(uint64_t min, uint64_t max);
  double get_random_double(double min, double max);
  boost::uuids::uuid get_guid();
  void start_clipping(const Rect &scaled_rect);
  void end_clipping();
};

GLuint compileShader(const GLchar *source, GLuint shaderType);
GLuint getShaderProgramId(const char *vertexFile, const char *fragmentFile);

#endif // ENGINE_H
