//
// NanoRT(Software raytracer) backend for NanoVG.
//
// Copyright (c) 2015 Syoyo Fujita.
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

//
// nanovg_rt.h is based on nanovg_gl2.h
//
// Copyright (c) 2009-2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
#ifndef NANOVG_RT_H
#define NANOVG_RT_H

#include "nanort.h"

#ifdef __cplusplus
extern "C" {
#endif

// Create flags

enum NVGcreateFlags {
  // Flag indicating if geometry based anti-aliasing is used (may not be needed
  // when using MSAA).
  NVG_ANTIALIAS = 1 << 0,
  // Flag indicating if strokes should be drawn using stencil buffer. The
  // rendering will be a little
  // slower, but path overlaps (i.e. self-intersecting or sharp turns) will be
  // drawn just once.
  NVG_STENCIL_STROKES = 1 << 1,
  // Flag indicating that additional debug checks are done.
  NVG_DEBUG = 1 << 2,
};

NVGcontext *nvgCreateRT(int flags, int w, int h, int clrColor=0xff);
void nvgDeleteRT(NVGcontext *ctx);
void nvgClearBackgroundRT(NVGcontext *ctx, float r, float g, float b, float a); // Clear background.
unsigned char *nvgReadPixelsRT(NVGcontext *ctx); // Returns RGBA8 pixel data.

// These are additional flags on top of NVGimageFlags.
enum NVGimageFlagsRT {
  NVG_IMAGE_NODELETE = 1 << 16, // Do not delete RT texture handle.
};

#ifdef __cplusplus
}
#endif

#endif /* NANOVG_RT_H */

#ifdef NANOVG_RT_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nanovg.h"

namespace {

union fi {
  float f;
  unsigned int i;
};

inline unsigned int mask(int x) { return (1U << x) - 1; }

// from fmath.hpp
/*
  for given y > 0
  get f_y(x) := pow(x, y) for x >= 0
*/

class PowGenerator {
  enum { N = 11 };
  float tbl0_[256];
  struct {
    float app;
    float rev;
  } tbl1_[1 << N];

public:
  PowGenerator(float y) {
    for (int i = 0; i < 256; i++) {
      tbl0_[i] = ::powf(2, (i - 127) * y);
    }
    const double e = 1 / double(1 << 24);
    const double h = 1 / double(1 << N);
    const size_t n = 1U << N;
    for (size_t i = 0; i < n; i++) {
      double x = 1 + double(i) / n;
      double a = ::pow(x, (double)y);
      tbl1_[i].app = (float)a;
      double b = ::pow(x + h - e, (double)y);
      tbl1_[i].rev = (float)((b - a) / (h - e) / (1 << 23));
    }
  }
  float get(float x) const {
    fi fi;
    fi.f = x;
    int a = (fi.i >> 23) & mask(8);
    unsigned int b = fi.i & mask(23);
    unsigned int b1 = b & (mask(N) << (23 - N));
    unsigned int b2 = b & mask(23 - N);
    float f;
    int idx = b1 >> (23 - N);
    f = tbl0_[a] * (tbl1_[idx].app + float(b2) * tbl1_[idx].rev);
    return f;
  }
};

class TextureSampler {

public:
  typedef enum {
    FORMAT_BYTE = 0,
    FORMAT_FLOAT = 1,
  } Format;

  TextureSampler() : m_pow(1.0f) {
    // Make invalid texture
    m_width = -1;
    m_height = -1;
    m_image = NULL;
    m_components = -1;
  }

  TextureSampler(unsigned char *image, int width, int height, int components,
                 Format format, float gamma = 1.0f)
      : m_pow(gamma) {
    Set(image, width, height, components, format, gamma);
  }

  ~TextureSampler() {}

  void Set(const unsigned char *image, int width, int height, int components,
           Format format, float gamma = 1.0f) {
    m_width = width;
    m_height = height;
    m_image = image;
    m_invWidth = 1.0f / width;
    m_invHeight = 1.0f / height;
    m_components = components;
    m_format = format;
    m_gamma = gamma;
  }

  int width() const { return m_width; }

  int height() const { return m_height; }

  int components() const { return m_components; }

  const unsigned char *image() const { return m_image; }

  int format() const { return m_format; }

  float gamma() const { return m_gamma; }

  void fetch(float *rgba, float u, float v) const;

  bool IsValid() const {
    return (m_image != NULL) && (m_width > 0) && (m_height > 0);
  }

private:
  int m_width;
  int m_height;
  float m_invWidth;
  float m_invHeight;
  int m_components;
  const unsigned char *m_image;
  int m_format;
  float m_gamma;
  PowGenerator m_pow;
};

int inline fasterfloor(const float x) {
  if (x >= 0) {
    return (int)x;
  }

  int y = (int)x;
  if (std::abs(x - y) <= std::numeric_limits<float>::epsilon()) {
    // Do nothing.
  } else {
    y = y - 1;
  }

  return y;
}

inline float lerp(float x, float y, float t) { return x + t * (y - x); }

// bool myisnan(float a) {
//  volatile float d = a;
//  return d != d;
//}

inline void FilterByteLerp(float *rgba, const unsigned char *image, int i00,
                           int i10, int i01, int i11, float dx, float dy,
                           int stride, const PowGenerator &p) {
  float texel[4][4];

  const float inv = 1.0f / 255.0f;
  if (stride == 4) {

    // Assume color is already degamma'ed.
    for (int i = 0; i < 4; i++) {
      texel[0][i] = (float)image[i00 + i] * inv;
      texel[1][i] = (float)image[i10 + i] * inv;
      texel[2][i] = (float)image[i01 + i] * inv;
      texel[3][i] = (float)image[i11 + i] * inv;
    }

    for (int i = 0; i < 4; i++) {
      rgba[i] = lerp(lerp(texel[0][i], texel[1][i], dx),
                     lerp(texel[2][i], texel[3][i], dx), dy);
    }

  } else {

    for (int i = 0; i < stride; i++) {
      texel[0][i] = (float)image[i00 + i] * inv;
      texel[1][i] = (float)image[i10 + i] * inv;
      texel[2][i] = (float)image[i01 + i] * inv;
      texel[3][i] = (float)image[i11 + i] * inv;
    }

    for (int i = 0; i < stride; i++) {
      rgba[i] = texel[0][i]; // NEAREST
      // rgba[i] = lerp(lerp(texel[0][i], texel[1][i], dx), lerp(texel[2][i],
      // texel[3][i], dx), dy);
    }
  }

  if (stride < 4) {
    for (int i = stride; i < 4; i++) {
      rgba[i] = rgba[stride - 1];
    }
    // rgba[3] = 1.0;
  }
}
}

inline void FilterFloatLerp(float *rgba, const float *image, int i00, int i10,
                            int i01, int i11, float dx, float dy, int stride,
                            const PowGenerator &p) {
  float texel[4][4];

  if (stride == 4) {

    for (int i = 0; i < 4; i++) {
      texel[0][i] = image[i00 + i];
      texel[1][i] = image[i10 + i];
      texel[2][i] = image[i01 + i];
      texel[3][i] = image[i11 + i];
    }

    for (int i = 0; i < 4; i++) {
      rgba[i] = lerp(lerp(texel[0][i], texel[1][i], dx),
                     lerp(texel[2][i], texel[3][i], dx), dy);
    }

  } else {

    for (int i = 0; i < stride; i++) {
      texel[0][i] = image[i00 + i];
      texel[1][i] = image[i10 + i];
      texel[2][i] = image[i01 + i];
      texel[3][i] = image[i11 + i]; // alpha is linear
    }

    for (int i = 0; i < stride; i++) {
      rgba[i] = lerp(lerp(texel[0][i], texel[1][i], dx),
                     lerp(texel[2][i], texel[3][i], dx), dy);
    }
  }

  if (stride < 4) {
    rgba[3] = 1.0;
  }
}

void TextureSampler::fetch(float *rgba, float u, float v) const {

  if (!IsValid()) {
    if (rgba) {
      rgba[0] = 0.0f;
      rgba[1] = 0.0f;
      rgba[2] = 0.0f;
      rgba[3] = 1.0f;
    }
    return;
  }

  float sx = (float)fasterfloor(u);
  float sy = (float)fasterfloor(v);

  float uu = u - sx;
  float vv = v - sy;

  // clamp
  uu = std::max(uu, 0.0f);
  uu = std::min(uu, 1.0f);
  vv = std::max(vv, 0.0f);
  vv = std::min(vv, 1.0f);

  float px = m_width * uu;
  float py = m_height * vv;

  int x0 = (int)px;
  int y0 = (int)py;
  int x1 = ((x0 + 1) >= m_width) ? (m_width - 1) : (x0 + 1);
  int y1 = ((y0 + 1) >= m_height) ? (m_height - 1) : (y0 + 1);

  float dx = px - (float)x0;
  float dy = py - (float)y0;

  float w[4];

  w[0] = (1.0f - dx) * (1.0f - dy);
  w[1] = (1.0f - dx) * (dy);
  w[2] = (dx) * (1.0f - dy);
  w[3] = (dx) * (dy);

  int stride = m_components;

  int i00 = stride * (y0 * m_width + x0);
  int i01 = stride * (y0 * m_width + x1);
  int i10 = stride * (y1 * m_width + x0);
  int i11 = stride * (y1 * m_width + x1);

  if (m_format == FORMAT_BYTE) {
    FilterByteLerp(rgba, m_image, i00, i01, i10, i11, dx, dy, stride, m_pow);
  } else if (m_format == FORMAT_FLOAT) {
    FilterFloatLerp(rgba, reinterpret_cast<const float *>(m_image), i00, i01,
                    i10, i11, dx, dy, stride, m_pow);
  } else { // unknown
  }
}

inline void colorize_material_id(unsigned char col[3], unsigned int mid) {
  unsigned char table[7][3] = {{255, 0, 0},
                       {0, 0, 255},
                       {0, 255, 0},
                       {255, 0, 255},
                       {0, 255, 255},
                       {255, 255, 0},
                       {255, 255, 255}};

  int id = mid % 7;

  col[0] = table[id][0];
  col[1] = table[id][1];
  col[2] = table[id][2];
}

enum RTNVGuniformLoc {
  RTNVG_LOC_VIEWSIZE,
  RTNVG_LOC_TEX,
  RTNVG_LOC_FRAG,
  RTNVG_MAX_LOCS
};

enum RTNVGshaderType {
  NSVG_SHADER_FILLGRAD,
  NSVG_SHADER_FILLIMG,
  NSVG_SHADER_SIMPLE,
  NSVG_SHADER_IMG
};

#if NANOVG_GL_USE_UNIFORMBUFFER
enum RTNVGuniformBindings {
  RTNVG_FRAG_BINDING = 0,
};
#endif

struct RTNVGshader {
  unsigned int prog;
  unsigned int frag;
  unsigned int vert;
  int loc[RTNVG_MAX_LOCS];
};
typedef struct RTNVGshader RTNVGshader;

struct RTNVGtexture {
  int id;
  unsigned int tex;
  int width, height;
  int type;
  int flags;
  unsigned char *data;
};
typedef struct RTNVGtexture RTNVGtexture;

enum RTNVGcallType {
  RTNVG_NONE = 0,
  RTNVG_FILL,
  RTNVG_CONVEXFILL,
  RTNVG_STROKE,
  RTNVG_TRIANGLES,
};

struct RTNVGcall {
  int type;
  int image;
  int pathOffset;
  int pathCount;
  int triangleOffset;
  int triangleCount;
  int uniformOffset;
};
typedef struct RTNVGcall RTNVGcall;

struct RTNVGpath {
  int fillOffset;
  int fillCount;
  int strokeOffset;
  int strokeCount;
};
typedef struct RTNVGpath RTNVGpath;

struct RTNVGfragUniforms {
#if NANOVG_GL_USE_UNIFORMBUFFER
  float scissorMat[12]; // matrices are actually 3 vec4s
  float paintMat[12];
  struct NVGcolor innerCol;
  struct NVGcolor outerCol;
  float scissorExt[2];
  float scissorScale[2];
  float extent[2];
  float radius;
  float feather;
  float strokeMult;
  float strokeThr;
  int texType;
  int type;
#else
// note: after modifying layout or size of uniform array,
// don't forget to also update the fragment shader source!
#define NANOVG_GL_UNIFORMARRAY_SIZE 11
  union {
    struct {
      float scissorMat[12]; // matrices are actually 3 vec4s
      float paintMat[12];
      struct NVGcolor innerCol;
      struct NVGcolor outerCol;
      float scissorExt[2];
      float scissorScale[2];
      float extent[2];
      float radius;
      float feather;
      float strokeMult;
      float strokeThr;
      float texType;
      float type;
    };
    float uniformArray[NANOVG_GL_UNIFORMARRAY_SIZE][4];
  };
#endif
};
typedef struct RTNVGfragUniforms RTNVGfragUniforms;

struct RTNVGcontext {
  RTNVGshader shader;
  RTNVGtexture *textures;
  float view[2];
  int ntextures;
  int ctextures;
  int textureId;
  unsigned int vertBuf;
#if NANOVG_GL_USE_UNIFORMBUFFER
  unsigned int fragBuf;
#endif
  int fragSize;
  int flags;

  // Per frame buffers
  RTNVGcall *calls;
  int ccalls;
  int ncalls;
  RTNVGpath *paths;
  int cpaths;
  int npaths;
  struct NVGvertex *verts;
  int cverts;
  int nverts;
  unsigned char *uniforms;
  int cuniforms;
  int nuniforms;

// cached state
#if NANOVG_GL_USE_STATE_FILTER
  unsigned int boundTexture;
  unsigned int stencilMask;
  int stencilFunc;
  int stencilFuncRef;
  unsigned int stencilFuncMask;
#endif

  unsigned char *pixels; // RGBA
  int width;
  int height;
};
typedef struct RTNVGcontext RTNVGcontext;

static int rtnvg__maxi(int a, int b) { return a > b ? a : b; }

#ifdef NANOVG_GLES2
static unsigned int rtnvg__nearestPow2(unsigned int num) {
  unsigned n = num > 0 ? num - 1 : 0;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;
  return n;
}
#endif

static void rtnvg__bindTexture(RTNVGcontext *rt, unsigned int tex) {
#if NANOVG_GL_USE_STATE_FILTER
  if (rt->boundTexture != tex) {
    rt->boundTexture = tex;
    rtBindTexture(GL_TEXTURE_2D, tex);
  }
#else
// rtBindTexture(GL_TEXTURE_2D, tex);
#endif
}

static void rtnvg__stencilMask(RTNVGcontext *rt, unsigned int mask) {
#if NANOVG_GL_USE_STATE_FILTER
  if (rt->stencilMask != mask) {
    rt->stencilMask = mask;
    rtStencilMask(mask);
  }
#else
// rtStencilMask(mask);
#endif
}

#if 0
static void rtnvg__stencilFunc(RTNVGcontext* rt, int func, int ref, unsigned int mask)
{
#if NANOVG_GL_USE_STATE_FILTER
	if ((rt->stencilFunc != func) ||
		(rt->stencilFuncRef != ref) ||
		(rt->stencilFuncMask != mask)) {

		rt->stencilFunc = func;
		rt->stencilFuncRef = ref;
		rt->stencilFuncMask = mask;
		rtStencilFunc(func, ref, mask);
	}
#else
	//rtStencilFunc(func, ref, mask);
#endif
}
#endif

static RTNVGtexture *rtnvg__allocTexture(RTNVGcontext *rt) {
  RTNVGtexture *tex = NULL;
  int i;

  for (i = 0; i < rt->ntextures; i++) {
    if (rt->textures[i].id == 0) {
      tex = &rt->textures[i];
      break;
    }
  }
  if (tex == NULL) {
    if (rt->ntextures + 1 > rt->ctextures) {
      RTNVGtexture *textures;
      int ctextures = rtnvg__maxi(rt->ntextures + 1, 4) +
                      rt->ctextures / 2; // 1.5x Overallocate
      textures = (RTNVGtexture *)realloc(rt->textures,
                                         sizeof(RTNVGtexture) * ctextures);
      if (textures == NULL)
        return NULL;
      rt->textures = textures;
      rt->ctextures = ctextures;
    }
    tex = &rt->textures[rt->ntextures++];
  }

  memset(tex, 0, sizeof(*tex));
  tex->id = ++rt->textureId;

  return tex;
}

static RTNVGtexture *rtnvg__findTexture(RTNVGcontext *rt, int id) {
  int i;
  for (i = 0; i < rt->ntextures; i++)
    if (rt->textures[i].id == id)
      return &rt->textures[i];
  return NULL;
}

static int rtnvg__deleteTexture(RTNVGcontext *rt, int id) {
  int i;
  for (i = 0; i < rt->ntextures; i++) {
    if (rt->textures[i].id == id) {
      if (rt->textures[i].tex != 0 &&
          (rt->textures[i].flags & NVG_IMAGE_NODELETE) == 0) {
        free(rt->textures[i].data);
        // glDeleteTextures(1, &rt->textures[i].tex);
      }
      memset(&rt->textures[i], 0, sizeof(rt->textures[i]));
      return 1;
    }
  }
  return 0;
}

#if 0
static void rtnvg__dumpShaderError(unsigned int shader, const char* name, const char* type)
{
	//char str[512+1];
	//int len = 0;
	//glGetShaderInfoLog(shader, 512, &len, str);
	//if (len > 512) len = 512;
	//str[len] = '\0';
	//printf("Shader %s/%s error:\n%s\n", name, type, str);
}

static void rtnvg__dumpProgramError(unsigned int prog, const char* name)
{
	//char str[512+1];
	//int len = 0;
	//glGetProgramInfoLog(prog, 512, &len, str);
	//if (len > 512) len = 512;
	//str[len] = '\0';
	//printf("Program %s error:\n%s\n", name, str);
}
#endif

static void rtnvg__checkError(RTNVGcontext *rt, const char *str) {
  // int err;
  // if ((rt->flags & NVG_DEBUG) == 0) return;
  // err = glGetError();
  // if (err != GL_NO_ERROR) {
  //	printf("Error %08x after %s\n", err, str);
  //	return;
  //}
}

static int rtnvg__createShader(RTNVGshader *shader, const char *name,
                               const char *header, const char *opts,
                               const char *vshader, const char *fshader) {
#if 0
	int status;
	unsigned int prog, vert, frag;
	const char* str[3];
	str[0] = header;
	str[1] = opts != NULL ? opts : "";

	memset(shader, 0, sizeof(*shader));

	prog = glCreateProgram();
	vert = glCreateShader(GL_VERTEX_SHADER);
	frag = glCreateShader(GL_FRAGMENT_SHADER);
	str[2] = vshader;
	glShaderSource(vert, 3, str, 0);
	str[2] = fshader;
	glShaderSource(frag, 3, str, 0);

	glCompileShader(vert);
	glGetShaderiv(vert, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		rtnvg__dumpShaderError(vert, name, "vert");
		return 0;
	}

	glCompileShader(frag);
	glGetShaderiv(frag, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		rtnvg__dumpShaderError(frag, name, "frag");
		return 0;
	}

	glAttachShader(prog, vert);
	glAttachShader(prog, frag);

	glBindAttribLocation(prog, 0, "vertex");
	glBindAttribLocation(prog, 1, "tcoord");

	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		rtnvg__dumpProgramError(prog, name);
		return 0;
	}

	shader->prog = prog;
	shader->vert = vert;
	shader->frag = frag;
#endif

  return 1;
}

static void rtnvg__deleteShader(RTNVGshader *shader) {
  // if (shader->prog != 0)
  //	glDeleteProgram(shader->prog);
  // if (shader->vert != 0)
  //	glDeleteShader(shader->vert);
  // if (shader->frag != 0)
  //	glDeleteShader(shader->frag);
}

static void rtnvg__getUniforms(RTNVGshader *shader) {
// shader->loc[RTNVG_LOC_VIEWSIZE] = glGetUniformLocation(shader->prog,
// "viewSize");
// shader->loc[RTNVG_LOC_TEX] = glGetUniformLocation(shader->prog, "tex");

#if NANOVG_GL_USE_UNIFORMBUFFER
// shader->loc[RTNVG_LOC_FRAG] = glGetUniformBlockIndex(shader->prog, "frag");
#else
// shader->loc[RTNVG_LOC_FRAG] = glGetUniformLocation(shader->prog, "frag");
#endif
}

static int rtnvg__renderCreate(void *uptr) {
  RTNVGcontext *rt = (RTNVGcontext *)uptr;
  int align = 4;

  // TODO: mediump float may not be enough for GLES2 in iOS.
  // see the following discussion: https://github.com/memononen/nanovg/issues/46
  static const char *shaderHeader = "#version 100\n"
                                    "#define NANOVG_GL2 1\n"

#if NANOVG_GL_USE_UNIFORMBUFFER
                                    "#define USE_UNIFORMBUFFER 1\n"
#else
                                    "#define UNIFORMARRAY_SIZE 11\n"
#endif
                                    "\n";

  static const char *fillVertShader =
      "#ifdef NANOVG_GL3\n"
      "	uniform vec2 viewSize;\n"
      "	in vec2 vertex;\n"
      "	in vec2 tcoord;\n"
      "	out vec2 ftcoord;\n"
      "	out vec2 fpos;\n"
      "#else\n"
      "	uniform vec2 viewSize;\n"
      "	attribute vec2 vertex;\n"
      "	attribute vec2 tcoord;\n"
      "	varying vec2 ftcoord;\n"
      "	varying vec2 fpos;\n"
      "#endif\n"
      "void main(void) {\n"
      "	ftcoord = tcoord;\n"
      "	fpos = vertex;\n"
      "	gl_Position = vec4(2.0*vertex.x/viewSize.x - 1.0, 1.0 - "
      "2.0*vertex.y/viewSize.y, 0, 1);\n"
      "}\n";

  static const char *fillFragShader =
      "#ifdef GL_ES\n"
      "#if defined(GL_FRAGMENT_PRECISION_HIGH) || defined(NANOVG_GL3)\n"
      " precision highp float;\n"
      "#else\n"
      " precision mediump float;\n"
      "#endif\n"
      "#endif\n"
      "#ifdef NANOVG_GL3\n"
      "#ifdef USE_UNIFORMBUFFER\n"
      "	layout(std140) uniform frag {\n"
      "		mat3 scissorMat;\n"
      "		mat3 paintMat;\n"
      "		vec4 innerCol;\n"
      "		vec4 outerCol;\n"
      "		vec2 scissorExt;\n"
      "		vec2 scissorScale;\n"
      "		vec2 extent;\n"
      "		float radius;\n"
      "		float feather;\n"
      "		float strokeMult;\n"
      "		float strokeThr;\n"
      "		int texType;\n"
      "		int type;\n"
      "	};\n"
      "#else\n" // NANOVG_GL3 && !USE_UNIFORMBUFFER
      "	uniform vec4 frag[UNIFORMARRAY_SIZE];\n"
      "#endif\n"
      "	uniform sampler2D tex;\n"
      "	in vec2 ftcoord;\n"
      "	in vec2 fpos;\n"
      "	out vec4 outColor;\n"
      "#else\n" // !NANOVG_GL3
      "	uniform vec4 frag[UNIFORMARRAY_SIZE];\n"
      "	uniform sampler2D tex;\n"
      "	varying vec2 ftcoord;\n"
      "	varying vec2 fpos;\n"
      "#endif\n"
      "#ifndef USE_UNIFORMBUFFER\n"
      "	#define scissorMat mat3(frag[0].xyz, frag[1].xyz, frag[2].xyz)\n"
      "	#define paintMat mat3(frag[3].xyz, frag[4].xyz, frag[5].xyz)\n"
      "	#define innerCol frag[6]\n"
      "	#define outerCol frag[7]\n"
      "	#define scissorExt frag[8].xy\n"
      "	#define scissorScale frag[8].zw\n"
      "	#define extent frag[9].xy\n"
      "	#define radius frag[9].z\n"
      "	#define feather frag[9].w\n"
      "	#define strokeMult frag[10].x\n"
      "	#define strokeThr frag[10].y\n"
      "	#define texType int(frag[10].z)\n"
      "	#define type int(frag[10].w)\n"
      "#endif\n"
      "\n"
      "float sdroundrect(vec2 pt, vec2 ext, float rad) {\n"
      "	vec2 ext2 = ext - vec2(rad,rad);\n"
      "	vec2 d = abs(pt) - ext2;\n"
      "	return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - rad;\n"
      "}\n"
      "\n"
      "// Scissoring\n"
      "float scissorMask(vec2 p) {\n"
      "	vec2 sc = (abs((scissorMat * vec3(p,1.0)).xy) - scissorExt);\n"
      "	sc = vec2(0.5,0.5) - sc * scissorScale;\n"
      "	return clamp(sc.x,0.0,1.0) * clamp(sc.y,0.0,1.0);\n"
      "}\n"
      "#ifdef EDGE_AA\n"
      "// Stroke - from [0..1] to clipped pyramid, where the slope is 1px.\n"
      "float strokeMask() {\n"
      "	return min(1.0, (1.0-abs(ftcoord.x*2.0-1.0))*strokeMult) * min(1.0, "
      "ftcoord.y);\n"
      "}\n"
      "#endif\n"
      "\n"
      "void main(void) {\n"
      "   vec4 result;\n"
      "	float scissor = scissorMask(fpos);\n"
      "#ifdef EDGE_AA\n"
      "	float strokeAlpha = strokeMask();\n"
      "#else\n"
      "	float strokeAlpha = 1.0;\n"
      "#endif\n"
      "	if (type == 0) {			// Gradient\n"
      "		// Calculate gradient color using box gradient\n"
      "		vec2 pt = (paintMat * vec3(fpos,1.0)).xy;\n"
      "		float d = clamp((sdroundrect(pt, extent, radius) + feather*0.5) / "
      "feather, 0.0, 1.0);\n"
      "		vec4 color = mix(innerCol,outerCol,d);\n"
      "		// Combine alpha\n"
      "		color *= strokeAlpha * scissor;\n"
      "		result = color;\n"
      "	} else if (type == 1) {		// Image\n"
      "		// Calculate color fron texture\n"
      "		vec2 pt = (paintMat * vec3(fpos,1.0)).xy / extent;\n"
      "#ifdef NANOVG_GL3\n"
      "		vec4 color = texture(tex, pt);\n"
      "#else\n"
      "		vec4 color = texture2D(tex, pt);\n"
      "#endif\n"
      "		if (texType == 1) color = vec4(color.xyz*color.w,color.w);"
      "		if (texType == 2) color = vec4(color.x);"
      "		// Apply color tint and alpha.\n"
      "		color *= innerCol;\n"
      "		// Combine alpha\n"
      "		color *= strokeAlpha * scissor;\n"
      "		result = color;\n"
      "	} else if (type == 2) {		// Stencil fill\n"
      "		result = vec4(1,1,1,1);\n"
      "	} else if (type == 3) {		// Textured tris\n"
      "#ifdef NANOVG_GL3\n"
      "		vec4 color = texture(tex, ftcoord);\n"
      "#else\n"
      "		vec4 color = texture2D(tex, ftcoord);\n"
      "#endif\n"
      "		if (texType == 1) color = vec4(color.xyz*color.w,color.w);"
      "		if (texType == 2) color = vec4(color.x);"
      "		color *= scissor;\n"
      "		result = color * innerCol;\n"
      "	}\n"
      "#ifdef EDGE_AA\n"
      "	if (strokeAlpha < strokeThr) discard;\n"
      "#endif\n"
      "#ifdef NANOVG_GL3\n"
      "	outColor = result;\n"
      "#else\n"
      "	gl_FragColor = result;\n"
      "#endif\n"
      "}\n";

  rtnvg__checkError(rt, "init");

  if (rt->flags & NVG_ANTIALIAS) {
    if (rtnvg__createShader(&rt->shader, "shader", shaderHeader,
                            "#define EDGE_AA 1\n", fillVertShader,
                            fillFragShader) == 0)
      return 0;
  } else {
    if (rtnvg__createShader(&rt->shader, "shader", shaderHeader, NULL,
                            fillVertShader, fillFragShader) == 0)
      return 0;
  }

  rtnvg__checkError(rt, "uniform locations");
  rtnvg__getUniforms(&rt->shader);

// Create dynamic vertex array
// glGenBuffers(1, &rt->vertBuf);

#if NANOVG_GL_USE_UNIFORMBUFFER
// Create UBOs
// glUniformBlockBinding(rt->shader.prog, rt->shader.loc[RTNVG_LOC_FRAG],
// RTNVG_FRAG_BINDING);
// glGenBuffers(1, &rt->fragBuf);
// glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);
#endif
  rt->fragSize =
      sizeof(RTNVGfragUniforms) + align - sizeof(RTNVGfragUniforms) % align;

  rtnvg__checkError(rt, "create done");

  // glFinish();

  // printf("renderCreate\n");

  return 1;
}

static int rtnvg__renderCreateTexture(void *uptr, int type, int w, int h,
                                      int imageFlags,
                                      const unsigned char *data) {
  // printf("createTexture\n");
  RTNVGcontext *rt = (RTNVGcontext *)uptr;
  RTNVGtexture *tex = rtnvg__allocTexture(rt);

  if (tex == NULL)
    return 0;

  // glGenTextures(1, &tex->tex);
  tex->width = w;
  tex->height = h;
  tex->type = type;
  tex->flags = imageFlags;

  // Retain texture image.
  int components = 1;
  if (tex->type == NVG_TEXTURE_RGBA) {
    components = 4;
  }
  tex->data = (unsigned char *)malloc(tex->width * tex->height * components);
  if (data != NULL) {
    memcpy(tex->data, data, tex->width * tex->height * components);
    rtnvg__bindTexture(rt, tex->tex);
  }

  // @todo { mip mapping }

  rtnvg__checkError(rt, "create tex");
  rtnvg__bindTexture(rt, 0);

  return tex->id;
}

static int rtnvg__renderDeleteTexture(void *uptr, int image) {
  RTNVGcontext *rt = (RTNVGcontext *)uptr;
  return rtnvg__deleteTexture(rt, image);
}

static int rtnvg__renderUpdateTexture(void *uptr, int image, int x, int y,
                                      int w, int h, const unsigned char *data) {
  // printf("UpdateTexture. %d, %d, %p\n", x, y, data);
  RTNVGcontext *rt = (RTNVGcontext *)uptr;
  RTNVGtexture *tex = rtnvg__findTexture(rt, image);

  if (tex == NULL)
    return 0;
  rtnvg__bindTexture(rt, tex->tex);

  int offset = 0;
  // No support for all of skip, need to update a whole row at a time.
  if (tex->type == NVG_TEXTURE_RGBA) {
    offset = y * tex->width * 4;
    data += offset;
  } else {
    offset = y * tex->width;
    data += y * tex->width;
  }
  x = 0;
  w = tex->width;

  int components = 1;
  if (tex->type == NVG_TEXTURE_RGBA) {
    components = 4;
  }

  memcpy(tex->data + offset, data, w * h * components);

  // if (tex->type == NVG_TEXTURE_RGBA)
  //	//glTexSubImage2D(GL_TEXTURE_2D, 0, x,y, w,h, GL_RGBA, GL_UNSIGNED_BYTE,
  //data);
  // else
  //	//glTexSubImage2D(GL_TEXTURE_2D, 0, x,y, w,h, GL_RED, GL_UNSIGNED_BYTE,
  //data);

  // glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  rtnvg__bindTexture(rt, 0);

  return 1;
}

static int rtnvg__renderGetTextureSize(void *uptr, int image, int *w, int *h) {
  RTNVGcontext *rt = (RTNVGcontext *)uptr;
  RTNVGtexture *tex = rtnvg__findTexture(rt, image);
  if (tex == NULL)
    return 0;
  *w = tex->width;
  *h = tex->height;
  return 1;
}

static void rtnvg__xformToMat3x4(float *m3, float *t) {
  m3[0] = t[0];
  m3[1] = t[1];
  m3[2] = 0.0f;
  m3[3] = 0.0f;
  m3[4] = t[2];
  m3[5] = t[3];
  m3[6] = 0.0f;
  m3[7] = 0.0f;
  m3[8] = t[4];
  m3[9] = t[5];
  m3[10] = 1.0f;
  m3[11] = 0.0f;
}

static NVGcolor rtnvg__premulColor(NVGcolor c) {
  c.r *= c.a;
  c.g *= c.a;
  c.b *= c.a;
  return c;
}

static int rtnvg__convertPaint(RTNVGcontext *rt, RTNVGfragUniforms *frag,
                               NVGpaint *paint, NVGscissor *scissor,
                               float width, float fringe, float strokeThr) {
  // printf("convertPaint\n");
  RTNVGtexture *tex = NULL;
  float invxform[6];

  memset(frag, 0, sizeof(*frag));

  frag->innerCol = rtnvg__premulColor(paint->innerColor);
  frag->outerCol = rtnvg__premulColor(paint->outerColor);

  if (scissor->extent[0] < -0.5f || scissor->extent[1] < -0.5f) {
    memset(frag->scissorMat, 0, sizeof(frag->scissorMat));
    frag->scissorExt[0] = 1.0f;
    frag->scissorExt[1] = 1.0f;
    frag->scissorScale[0] = 1.0f;
    frag->scissorScale[1] = 1.0f;
  } else {
    nvgTransformInverse(invxform, scissor->xform);
    rtnvg__xformToMat3x4(frag->scissorMat, invxform);
    frag->scissorExt[0] = scissor->extent[0];
    frag->scissorExt[1] = scissor->extent[1];
    frag->scissorScale[0] = sqrtf(scissor->xform[0] * scissor->xform[0] +
                                  scissor->xform[2] * scissor->xform[2]) /
                            fringe;
    frag->scissorScale[1] = sqrtf(scissor->xform[1] * scissor->xform[1] +
                                  scissor->xform[3] * scissor->xform[3]) /
                            fringe;
  }

  memcpy(frag->extent, paint->extent, sizeof(frag->extent));
  frag->strokeMult = (width * 0.5f + fringe * 0.5f) / fringe;
  frag->strokeThr = strokeThr;

  if (paint->image != 0) {
    tex = rtnvg__findTexture(rt, paint->image);
    if (tex == NULL)
      return 0;
    if ((tex->flags & NVG_IMAGE_FLIPY) != 0) {
      float flipped[6];
      nvgTransformScale(flipped, 1.0f, -1.0f);
      nvgTransformMultiply(flipped, paint->xform);
      nvgTransformInverse(invxform, flipped);
    } else {
      nvgTransformInverse(invxform, paint->xform);
    }
    frag->type = NSVG_SHADER_FILLIMG;

    if (tex->type == NVG_TEXTURE_RGBA)
      frag->texType = (tex->flags & NVG_IMAGE_PREMULTIPLIED) ? 0.0f : 1.0f;
    else
      frag->texType = 2;
    //		printf("frag->texType = %d\n", frag->texType);
  } else {
    frag->type = NSVG_SHADER_FILLGRAD;
    frag->radius = paint->radius;
    frag->feather = paint->feather;
    nvgTransformInverse(invxform, paint->xform);
  }

  rtnvg__xformToMat3x4(frag->paintMat, invxform);

  return 1;
}

static RTNVGfragUniforms *nvg__fragUniformPtr(RTNVGcontext *rt, int i);

static void rtnvg__setUniforms(RTNVGcontext *rt, int uniformOffset, int image) {
#if NANOVG_GL_USE_UNIFORMBUFFER
// glBindBufferRange(GL_UNIFORM_BUFFER, RTNVG_FRAG_BINDING, rt->fragBuf,
// uniformOffset, sizeof(RTNVGfragUniforms));
#else
// RTNVGfragUniforms* frag = nvg__fragUniformPtr(rt, uniformOffset);
// glUniform4fv(rt->shader.loc[RTNVG_LOC_FRAG], NANOVG_GL_UNIFORMARRAY_SIZE,
// &(frag->uniformArray[0][0]));
#endif

  if (image != 0) {
    RTNVGtexture *tex = rtnvg__findTexture(rt, image);
    rtnvg__bindTexture(rt, tex != NULL ? tex->tex : 0);
    rtnvg__checkError(rt, "tex paint tex");
  } else {
    rtnvg__bindTexture(rt, 0);
  }
}

static void rtnvg__renderViewport(void *uptr, float width, float height, float devicePixelRatio) {
  (void)devicePixelRatio;
  RTNVGcontext *rt = (RTNVGcontext *)uptr;
  rt->view[0] = width;
  rt->view[1] = height;
}

static float rtnvg__sdroundrect(float pt[2], float ext[2], float rad) {
  float ext2[2];
  ext2[0] = ext[0] - rad;
  ext2[1] = ext[1] - rad;
  float d[2];
  d[0] = fabsf(pt[0]) - ext2[0];
  d[1] = fabsf(pt[1]) - ext2[1];
  // printf("d = %f, %f\n", d[0], d[1]);
  float max_d[2];
  max_d[0] = (d[0] < 0.0f) ? 0.0f : d[0];
  max_d[1] = (d[1] < 0.0f) ? 0.0f : d[1];

  float d_val = (d[0] > d[1]) ? d[0] : d[1];
  d_val = (d_val < 0.0f) ? d_val : 0.0f;
  // printf("d_val = %f\n", d_val);

  float val = d_val + sqrtf(max_d[0] * max_d[0] + max_d[1] * max_d[1]) - rad;
  // printf("val = %f\n", val);

  return val;
}

static unsigned char ftouc(float x) {
  int i = (int)(x * 255.0f);
  i = (i < 0) ? 0 : i;
  i = (i > 255) ? 255 : i;
  return (unsigned char)i;
}

static float uctof(unsigned char x) { return (float)x / 255.0f; }

static float fclamp(float x, float minval, float maxval) {
  float y = (x < minval) ? minval : x;
  y = (y > maxval) ? maxval : y;
  return y;
}

static void rtnvg__alphaBlend(unsigned char *dst, const float col[4]) {
  // @todo { linear space compisition? }

  float d0 = uctof(dst[0]);
  float d1 = uctof(dst[1]);
  float d2 = uctof(dst[2]);
  float d3 = uctof(dst[3]);

  float alpha = fclamp(col[3], 0.0f, 1.0f);
  float r0 = col[0] + d0 * (1.0f - alpha);
  float r1 = col[1] + d1 * (1.0f - alpha);
  float r2 = col[2] + d2 * (1.0f - alpha);
  float r3 = col[3] + d3 * (1.0f - alpha);

  dst[0] = ftouc(r0);
  dst[1] = ftouc(r1);
  dst[2] = ftouc(r2);
  dst[3] = ftouc(r3);
}

static float rtnvg__scissorMask(float scissorMat[12], float scissorExt[2],
                                float scissorScale[2], float x, float y) {

  //(abs((scissorMat * vec3(p,1.0)).xy) - scissorExt);
  //	sc = vec2(0.5,0.5) - sc * scissorScale;
  //	return clamp(sc.x,0.0,1.0) * clamp(sc.y,0.0,1.0);

  float pp[2];
  pp[0] = scissorMat[0] * x + scissorMat[4] * y + scissorMat[8];
  pp[1] = scissorMat[1] * x + scissorMat[5] * y + scissorMat[9];

  float sc[2];
  sc[0] = fabsf(pp[0]) - scissorExt[0];
  sc[1] = fabsf(pp[1]) - scissorExt[1];

  sc[0] = 0.5f - sc[0] * scissorScale[0];
  sc[1] = 0.5f - sc[1] * scissorScale[1];

  return fclamp(sc[0], 0.0f, 1.0f) * fclamp(sc[1], 0.0f, 1.0f);
}

static void rtnvg__shade(float color[4], RTNVGcontext *rt,
                         RTNVGfragUniforms *frag, float x, float y, float tu,
                         float tv, int imageId) {
  float scissor = rtnvg__scissorMask(frag->scissorMat, frag->scissorExt,
                                     frag->scissorScale, x, y);
  float pt[2];
  pt[0] = frag->paintMat[0] * x + frag->paintMat[4] * y + frag->paintMat[8];
  pt[1] = frag->paintMat[1] * x + frag->paintMat[5] * y + frag->paintMat[9];
  // printf("scissor = %f, %f, %f, %f,   %f, %f, %f, %f,   %f, %f, %f, %f\n",
  //  frag->scissorMat[0],
  //  frag->scissorMat[1],
  //  frag->scissorMat[2],
  //  frag->scissorMat[3],
  //  frag->scissorMat[4],
  //  frag->scissorMat[5],
  //  frag->scissorMat[6],
  //  frag->scissorMat[7],
  //  frag->scissorMat[8],
  //  frag->scissorMat[9],
  //  frag->scissorMat[10],
  //  frag->scissorMat[11]);
  // printf("type = %f\n", frag->type);
  // printf("innerCol = %f, %f, %f, %f\n",
  //  frag->innerCol.r,
  //  frag->innerCol.g,
  //  frag->innerCol.b,
  //  frag->innerCol.a);
  // printf("outerCol = %f, %f, %f, %f\n",
  //  frag->outerCol.r,
  //  frag->outerCol.g,
  //  frag->outerCol.b,
  //  frag->outerCol.a);
  // printf("paintMat = %f, %f, %f, %f,   %f, %f, %f, %f   %f, %f, %f, %f\n",
  //  frag->paintMat[0],
  //  frag->paintMat[1],
  //  frag->paintMat[2],
  //  frag->paintMat[3],
  //  frag->paintMat[4],
  //  frag->paintMat[5],
  //  frag->paintMat[6],
  //  frag->paintMat[7],
  //  frag->paintMat[8],
  //  frag->paintMat[9],
  //  frag->paintMat[10],
  //  frag->paintMat[11]);

  // printf("pt = %f, %f\n", pt[0], pt[1]);
  color[0] = color[1] = color[2] = 0;
  color[3] = 1.0f;

  int type = (int)frag->type;
  if (type == 0) { // grad fill
                   // Calculate gradient color using box gradient

    // printf("feather = %f, rad = %f\n", frag->feather, frag->radius);
    // printf("extent = %f, %f\n", frag->extent[0], frag->extent[1]);
    float d = fclamp((rtnvg__sdroundrect(pt, frag->extent, frag->radius) +
                      frag->feather * 0.5f) /
                         (float)frag->feather,
                     0.0f, 1.0f);
    // printf("d = %f\n", (rtnvg__sdroundrect(pt, frag->extent, frag->radius) +
    // frag->feather*0.5f) / (float)frag->feather);
    color[0] = frag->innerCol.r * (1.0f - d) + frag->outerCol.r * d;
    color[1] = frag->innerCol.g * (1.0f - d) + frag->outerCol.g * d;
    color[2] = frag->innerCol.b * (1.0f - d) + frag->outerCol.b * d;
    color[3] = frag->innerCol.a * (1.0f - d) + frag->outerCol.a * d;

    float strokeAlpha = 1.0f; // @fixme.
    color[0] *= strokeAlpha * scissor;
    color[1] *= strokeAlpha * scissor;
    color[2] *= strokeAlpha * scissor;
    color[3] *= strokeAlpha * scissor;
  } else if (type == 1) { // Image
                          // Calculate color from texture

    int texType = (int)frag->texType;
    RTNVGtexture *tex = rtnvg__findTexture(rt, imageId);
    TextureSampler sampler;
    int components = 1;
    if (tex->type == NVG_TEXTURE_RGBA) {
      components = 4;
    }
    sampler.Set(tex->data, tex->width, tex->height, components,
                TextureSampler::FORMAT_BYTE);
    float tcol[4];
    sampler.fetch(tcol, pt[0] / frag->extent[0], pt[1] / frag->extent[1]);

    if (texType == 2) { // Use R channel.
      color[0] = frag->innerCol.r * tcol[0];
      color[1] = frag->innerCol.g * tcol[0];
      color[2] = frag->innerCol.b * tcol[0];
      color[3] = frag->innerCol.a * tcol[0];
    } else {
      color[0] = frag->innerCol.r * tcol[0] * tcol[3];
      color[1] = frag->innerCol.g * tcol[1] * tcol[3];
      color[2] = frag->innerCol.b * tcol[2] * tcol[3];
      color[3] = frag->innerCol.a * tcol[3];
    }

    float strokeAlpha = 1.0f; // @fixme.
    color[0] *= strokeAlpha * scissor;
    color[1] *= strokeAlpha * scissor;
    color[2] *= strokeAlpha * scissor;
    color[3] *= strokeAlpha * scissor;

  } else if (type == 3) { // textured tri
    int texType = (int)frag->texType;
    //"		if (texType == 1) color = vec4(color.xyz*color.w,color.w);"
    //"		if (texType == 2) color = vec4(color.x);"
    // if (texType == 1) {
    //}
    // if (texType == 2) {
    // printf("tu, tv = %f, %f\n", tu, tv);

    RTNVGtexture *tex = rtnvg__findTexture(rt, imageId);
    TextureSampler sampler;
    int components = 1;
    if (tex->type == NVG_TEXTURE_RGBA) {
      components = 4;
    }
    sampler.Set(tex->data, tex->width, tex->height, components,
                TextureSampler::FORMAT_BYTE);
    // printf("texId = %d, data = %p, w = %d\n", rt->textureId, tex->data,
    // tex->width);
    float tcol[4];
    sampler.fetch(tcol, tu, tv);

    if (texType == 2) { // Use R channel.
      color[0] = frag->innerCol.r * tcol[0];
      color[1] = frag->innerCol.g * tcol[0];
      color[2] = frag->innerCol.b * tcol[0];
      color[3] = frag->innerCol.a * tcol[0];
    } else {
      color[0] = frag->innerCol.r * tcol[0] * tcol[3];
      color[1] = frag->innerCol.g * tcol[1] * tcol[3];
      color[2] = frag->innerCol.b * tcol[2] * tcol[3];
      color[3] = frag->innerCol.a * tcol[3];
    }

    float strokeAlpha = 1.0f; // @fixme.
    color[0] *= strokeAlpha * scissor;
    color[1] *= strokeAlpha * scissor;
    color[2] *= strokeAlpha * scissor;
    color[3] *= strokeAlpha * scissor;
  }
}

static void rtnvg__fill(RTNVGcontext *rt, RTNVGcall *call) {
  // printf("__fill\n");
  RTNVGpath *paths = &rt->paths[call->pathOffset];
  (void)paths;
  int i, npaths = call->pathCount;
  (void)i;
  (void)npaths;

  // Draw shapes
  // glEnable(GL_STENCIL_TEST);
  rtnvg__stencilMask(rt, 0xff);
  // rtnvg__stencilFunc(rt, GL_ALWAYS, 0, 0xff);
  // glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  // set bindpoint for solid loc
  rtnvg__setUniforms(rt, call->uniformOffset, 0);
  rtnvg__checkError(rt, "fill simple");

  // glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
  // glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
  // glDisable(GL_CULL_FACE);
  // for (i = 0; i < npaths; i++)
  //	glDrawArrays(GL_TRIANGLE_FAN, paths[i].fillOffset, paths[i].fillCount);
  // glEnable(GL_CULL_FACE);

  // Draw anti-aliased pixels
  // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  // for (i = 0; i < npaths; i++) {
  //  printf("fill: offset = %d, count = %d\n", paths[i].fillOffset,
  //  paths[i].fillCount);
  //}
  rtnvg__setUniforms(rt, call->uniformOffset + rt->fragSize, call->image);
  rtnvg__checkError(rt, "fill fill");

  if (rt->flags & NVG_ANTIALIAS) {
    // rtnvg__stencilFunc(rt, GL_EQUAL, 0x00, 0xff);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    // Draw fringes
    // for (i = 0; i < npaths; i++) {
    //	glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset,
    //paths[i].strokeCount);
    // printf("storoke: offset = %d, count = %d\n", paths[i].strokeOffset,
    // paths[i].strokeCount);
    //}
  }

  // Draw fill
  // rtnvg__stencilFunc(rt, GL_NOTEQUAL, 0x0, 0xff);
  // glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
  // glDrawArrays(GL_TRIANGLES, call->triangleOffset, call->triangleCount);
  // printf("GL_TRIANGLES: drawFill: triangleOffset: %d, triangleCount: %d\n",
  // call->triangleOffset, call->triangleCount);

  // render
  {
    std::vector<float> vertices;
    std::vector<float> texcoords;
    std::vector<unsigned int> faces;

    // Convert geometry to nanort friendly format.

    // TRIANGLE_FAN -> TRIANGLES.
    for (int k = 0; k < npaths; k++) {
      int npolys = paths[k].fillCount - 2;
      int voffset = (int)vertices.size() / 3;
      for (int n = 0; n < npolys; n++) {
        faces.push_back(voffset + 0);
        faces.push_back(voffset + n + 1);
        faces.push_back(voffset + n + 2);
      }

      for (int n = 0; n < paths[k].fillCount; n++) {
        vertices.push_back(rt->verts[paths[k].fillOffset + n].x);
        vertices.push_back(rt->verts[paths[k].fillOffset + n].y);
        vertices.push_back(0.0f);
        texcoords.push_back(rt->verts[paths[k].fillOffset + n].u);
        texcoords.push_back(rt->verts[paths[k].fillOffset + n].v);
      }
    }

    // printf("vertices.size = %d\n", (int)vertices.size() / 3);
    // printf("faces.size = %d\n", (int)faces.size() / 3);

    if (faces.size() > 0) {

      unsigned char *rgba = rt->pixels;

      nanort::BVHBuildOptions options; // Use default option

      // printf("  BVH build option:\n");
      // printf("    # of leaf primitives: %d\n", options.minLeafPrimitives);
      // printf("    SAH binsize         : %d\n", options.binSize);

      nanort::BVHAccel accel;
      bool ret =
          accel.Build(&vertices.at(0), &faces.at(0), (unsigned int)faces.size() / 3, options);
      assert(ret);
      (void)ret;

      // nanort::BVHBuildStatistics stats = accel.GetStatistics();

      // printf("  BVH statistics:\n");
      // printf("    # of leaf   nodes: %d\n", stats.numLeafNodes);
      // printf("    # of branch nodes: %d\n", stats.numBranchNodes);
      // printf("  Max tree depth   : %d\n", stats.maxTreeDepth);
      // float bmin[3], bmax[3];
      // accel.BoundingBox(bmin, bmax);
      // printf("  Bmin               : %f, %f, %f\n", bmin[0], bmin[1],
      // bmin[2]);
      // printf("  Bmax               : %f, %f, %f\n", bmax[0], bmax[1],
      // bmax[2]);

      // const float tFar = 1.0e+30f;

      int bound[4]; // l,t,r,b
      bound[0] = (int)rt->verts[call->triangleOffset + 0].x - 1;
      bound[1] = (int)rt->verts[call->triangleOffset + 2].y - 1;
      bound[2] = (int)rt->verts[call->triangleOffset + 1].x + 1;
      bound[3] = (int)rt->verts[call->triangleOffset + 0].y + 1;
      if (bound[0] < 0)           bound[0] = 0;
      if (bound[1] < 0)           bound[1] = 0;
      if (bound[2] >= rt->width)  bound[2] = rt->width  - 1;
      if (bound[3] >= rt->height) bound[3] = rt->height - 1;
      // printf("drawFill: triangleOffset: %d, triangleCount: %d\n",
      // call->triangleOffset, call->triangleCount);
      // Shoot rays.
      for (int y = bound[1]; y < bound[3]; y++) {
        // bound check
        if (y < 0) continue;
        if (y >= rt->height) continue;
        for (int x = bound[0]; x < bound[2]; x++) {
          // bound check
          if (x < 0) continue;
          if (x >= rt->width) continue;

          // Use multi-hit ray traversal to detect overdraw.
          nanort::StackVector<nanort::Intersection, 128> isects;
          int maxIsects = 128;

          // Simple ortho camera.

          nanort::Ray ray;
          ray.org[0] = x + 0.5f;
          ray.org[1] = y + 0.5f;
          ray.org[2] = 1.0f;

          ray.dir[0] = 0.0f;
          ray.dir[1] = 0.0f;
          ray.dir[2] = -1.0f;

          bool hit = accel.MultiHitTraverse(isects, maxIsects, &vertices.at(0),
                                            &faces.at(0), ray);

          // odd # of intersections --> valid hit.
          if (hit && (isects->size() % 2 == 1)) {
            float col[4];
            RTNVGfragUniforms *frag =
                nvg__fragUniformPtr(rt, call->uniformOffset + rt->fragSize);
            rtnvg__shade(col, rt, frag, ray.org[0], ray.org[1], 0.0f, 0.0f,
                         call->image);
            rtnvg__alphaBlend(&rgba[4 * (y * rt->width + x)], col);
          }
        }
      }
    }
  }

  // glDisable(GL_STENCIL_TEST);
}

static void rtnvg__convexFill(RTNVGcontext *rt, RTNVGcall *call) {
  // printf("__convexFill\n");
  RTNVGpath *paths = &rt->paths[call->pathOffset];
  int i, npaths = call->pathCount;
  (void)i;
  (void)npaths;
  (void)paths;

  rtnvg__setUniforms(rt, call->uniformOffset, call->image);
  rtnvg__checkError(rt, "convex fill");

  // for (i = 0; i < npaths; i++) {
  //  printf("[%d] offset = %d, fillCount = %d\n", i, paths[i].fillOffset,
  //  paths[i].fillCount);
  //}
  //	glDrawArrays(GL_TRIANGLE_FAN, paths[i].fillOffset, paths[i].fillCount);
  if (rt->flags & NVG_ANTIALIAS) {
    // Draw fringes
    // for (i = 0; i < npaths; i++) {
    //	//glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset,
    //paths[i].strokeCount);
    //  printf("[%d] stroke: offset = %d, fillCount = %d\n", i,
    //  paths[i].strokeOffset, paths[i].strokeCount);
    //}
  }

  // render
  {
    std::vector<float> vertices;
    std::vector<float> texcoords;
    std::vector<unsigned int> faces;

    // Convert geometry to nanort friendly format.

    // TRIANGLE_FAN -> TRIANGLES.
    for (int k = 0; k < npaths; k++) {
      int npolys = paths[k].fillCount - 2;
      int voffset = (int)vertices.size() / 3;
      for (int n = 0; n < npolys; n++) {
        faces.push_back(voffset + 0);
        faces.push_back(voffset + n + 1);
        faces.push_back(voffset + n + 2);
      }

      for (int n = 0; n < paths[k].fillCount; n++) {
        vertices.push_back(rt->verts[paths[k].fillOffset + n].x);
        vertices.push_back(rt->verts[paths[k].fillOffset + n].y);
        vertices.push_back(0.0f);
        texcoords.push_back(rt->verts[paths[k].fillOffset + n].u);
        texcoords.push_back(rt->verts[paths[k].fillOffset + n].v);
      }
    }

    if (faces.size() > 0) {

      unsigned char *rgba = rt->pixels;

      nanort::BVHBuildOptions options; // Use default option

      // printf("  BVH build option:\n");
      // printf("    # of leaf primitives: %d\n", options.minLeafPrimitives);
      // printf("    SAH binsize         : %d\n", options.binSize);

      nanort::BVHAccel accel;
      bool ret =
          accel.Build(&vertices.at(0), &faces.at(0), (unsigned int)faces.size() / 3, options);
      assert(ret);
      (void)ret;

      // nanort::BVHBuildStatistics stats = accel.GetStatistics();

      // printf("  BVH statistics:\n");
      // printf("    # of leaf   nodes: %d\n", stats.numLeafNodes);
      // printf("    # of branch nodes: %d\n", stats.numBranchNodes);
      // printf("  Max tree depth   : %d\n", stats.maxTreeDepth);
      // float bmin[3], bmax[3];
      // accel.BoundingBox(bmin, bmax);
      // printf("  Bmin               : %f, %f, %f\n", bmin[0], bmin[1],
      // bmin[2]);
      // printf("  Bmax               : %f, %f, %f\n", bmax[0], bmax[1],
      // bmax[2]);

      // const float tFar = 1.0e+30f;

      int bound[4]; // l,t,r,b
      bound[0] = (int)rt->verts[call->triangleOffset + 0].x - 1;
      bound[1] = (int)rt->verts[call->triangleOffset + 2].y - 1;
      bound[2] = (int)rt->verts[call->triangleOffset + 1].x + 1;
      bound[3] = (int)rt->verts[call->triangleOffset + 0].y + 1;
      if (bound[0] < 0)           bound[0] = 0;
      if (bound[1] < 0)           bound[1] = 0;
      if (bound[2] >= rt->width)  bound[2] = rt->width - 1;
      if (bound[3] >= rt->height) bound[3] = rt->height - 1;
      // printf("drawFill: triangleOffset: %d, triangleCount: %d\n",
      // call->triangleOffset, call->triangleCount);
      // Shoot rays.
      nanort::StackVector<nanort::Intersection, 128> isects;
      for (int y = bound[1]; y < bound[3]; y++) {
        // bound check
        if (y < 0) continue;
        if (y >= rt->height) continue;
        for (int x = bound[0]; x < bound[2]; x++) {
          // bound check
          if (x < 0) continue;
          if (x >= rt->width) continue;

          // Use multi-hit ray traversal to detect overdraw.
          isects.container().clear();
          int maxIsects = 128;

          // Simple ortho camera.

          nanort::Ray ray;
          ray.org[0] = x + 0.5f;
          ray.org[1] = y + 0.5f;
          ray.org[2] = 1.0f;

          ray.dir[0] = 0.0f;
          ray.dir[1] = 0.0f;
          ray.dir[2] = -1.0f;

          bool hit = accel.MultiHitTraverse(isects, maxIsects, &vertices.at(0),
                                            &faces.at(0), ray);

          // odd # of intersections --> valid hit.
          if (hit && (isects->size() % 2 == 1)) {
            float col[4];
            RTNVGfragUniforms *frag =
                nvg__fragUniformPtr(rt, call->uniformOffset);
            rtnvg__shade(col, rt, frag, ray.org[0], ray.org[1], 0.0f, 0.0f,
                         call->image);
            rtnvg__alphaBlend(&rgba[4 * (y * rt->width + x)], col);
          }
        }
      }
    }
  }
}

static void rtnvg__stroke(RTNVGcontext *rt, RTNVGcall *call) {
  // printf("__stroke\n");
  RTNVGpath *paths = &rt->paths[call->pathOffset];
  int npaths = call->pathCount, i;
  (void)i;
  (void)npaths;
  (void)paths;

  if (rt->flags & NVG_STENCIL_STROKES) {

    // printf("stencil_strokes\n");
    // glEnable(GL_STENCIL_TEST);
    rtnvg__stencilMask(rt, 0xff);

    // Fill the stroke base without overlap
    // rtnvg__stencilFunc(rt, GL_EQUAL, 0x0, 0xff);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    rtnvg__setUniforms(rt, call->uniformOffset + rt->fragSize, call->image);
    rtnvg__checkError(rt, "stroke fill 0");
    // for (i = 0; i < npaths; i++) {
    //  printf("[%d] strokeOfft: %d, strokeCount: %d\n", i,
    //  paths[i].strokeOffset, paths[i].strokeCount);
    ////	glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset,
    ///paths[i].strokeCount);
    //}

    // Draw anti-aliased pixels.
    rtnvg__setUniforms(rt, call->uniformOffset, call->image);
    // rtnvg__stencilFunc(rt, GL_EQUAL, 0x00, 0xff);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    // for (i = 0; i < npaths; i++)
    //	glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset,
    //paths[i].strokeCount);

    // Clear stencil buffer.
    // glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    // rtnvg__stencilFunc(rt, GL_ALWAYS, 0x0, 0xff);
    // glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
    rtnvg__checkError(rt, "stroke fill 1");
    // for (i = 0; i < npaths; i++)
    //	glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset,
    //paths[i].strokeCount);
    // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // glDisable(GL_STENCIL_TEST);

    //		rtnvg__convertPaint(rt, nvg__fragUniformPtr(rt, call->uniformOffset +
    //rt->fragSize), paint, scissor, strokeWidth, fringe, 1.0f - 0.5f/255.0f);

  } else {
    rtnvg__setUniforms(rt, call->uniformOffset, call->image);
    rtnvg__checkError(rt, "stroke fill");
    // Draw Strokes
    // for (i = 0; i < npaths; i++)
    //	glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset,
    //paths[i].strokeCount);
  }

  // render
  {
    std::vector<float> vertices;
    std::vector<float> texcoords;
    std::vector<unsigned int> faces;

    // Convert geometry to nanort friendly format.

    // TRIANGLE_STRIP -> TRIANGLES.
    for (int k = 0; k < npaths; k++) {
      int npolys = paths[k].strokeCount - 2;
      int voffset = (int)vertices.size() / 3;
      for (int n = 0; n < npolys; n++) {
        int n0, n1, n2;
        // flip vertex order for even and odd triangle.
        if ((n % 2) == 0) {
          n0 = n + 0;
          n1 = n + 1;
          n2 = n + 2;
        } else {
          n0 = n + 1;
          n1 = n + 0;
          n2 = n + 2;
        }
        faces.push_back(voffset + n0);
        faces.push_back(voffset + n1);
        faces.push_back(voffset + n2);
      }

      for (int n = 0; n < paths[k].strokeCount; n++) {
        vertices.push_back(rt->verts[paths[k].strokeOffset + n].x);
        vertices.push_back(rt->verts[paths[k].strokeOffset + n].y);
        vertices.push_back(0.0f);
        texcoords.push_back(rt->verts[paths[k].strokeOffset + n].u);
        texcoords.push_back(rt->verts[paths[k].strokeOffset + n].v);
      }
    }

    // printf("vertices.size = %d\n", (int)vertices.size() / 3);
    // printf("faces.size = %d\n", (int)faces.size() / 3);

    if (faces.size() > 0) {

      unsigned char *rgba = rt->pixels;

      nanort::BVHBuildOptions options; // Use default option

      // printf("  BVH build option:\n");
      // printf("    # of leaf primitives: %d\n", options.minLeafPrimitives);
      // printf("    SAH binsize         : %d\n", options.binSize);

      nanort::BVHAccel accel;
      bool ret =
          accel.Build(&vertices.at(0), &faces.at(0), (unsigned int)faces.size() / 3, options);
      assert(ret);
      (void)ret;

      // nanort::BVHBuildStatistics stats = accel.GetStatistics();

      // printf("  BVH statistics:\n");
      // printf("    # of leaf   nodes: %d\n", stats.numLeafNodes);
      // printf("    # of branch nodes: %d\n", stats.numBranchNodes);
      // printf("  Max tree depth   : %d\n", stats.maxTreeDepth);
      float bmin[3], bmax[3];
      accel.BoundingBox(bmin, bmax);
      // printf("  Bmin               : %f, %f, %f\n", bmin[0], bmin[1],
      // bmin[2]);
      // printf("  Bmax               : %f, %f, %f\n", bmax[0], bmax[1],
      // bmax[2]);

      int bound[4]; // l,t,r,b
      bound[0] = (int)bmin[0] - 1;
      bound[1] = (int)bmin[1] - 1;
      bound[2] = (int)bmax[0] + 1;
      bound[3] = (int)bmax[1] + 1;
      if (bound[0] < 0)           bound[0] = 0;
      if (bound[1] < 0)           bound[1] = 0;
      if (bound[2] >= rt->width)  bound[2] = rt->width - 1;
      if (bound[3] >= rt->height) bound[3] = rt->height - 1;
      // Shoot rays.
      for (int y = bound[1]; y < bound[3]; y++) {
        for (int x = bound[0]; x < bound[2]; x++) {
          // Use multi-hit ray traversal to detect overdraw.
          nanort::StackVector<nanort::Intersection, 128> isects;
          int maxIsects = 128;

          // Simple ortho camera.

          nanort::Ray ray;
          ray.org[0] = x + 0.5f;
          ray.org[1] = y + 0.5f;
          ray.org[2] = 1.0f;

          ray.dir[0] = 0.0f;
          ray.dir[1] = 0.0f;
          ray.dir[2] = -1.0f;

          bool hit = accel.MultiHitTraverse(isects, maxIsects, &vertices.at(0),
                                            &faces.at(0), ray);

          // odd # of intersections --> valid hit.
          if (hit && (isects->size() % 2 == 1)) {
            float col[4];
            RTNVGfragUniforms *frag =
                nvg__fragUniformPtr(rt, call->uniformOffset);
            rtnvg__shade(col, rt, frag, ray.org[0], ray.org[1], 0.0f, 0.0f,
                         call->image);
            rtnvg__alphaBlend(&rgba[4 * (y * rt->width + x) + 0], col);
          }
        }
      }
    }
  }
}

static void rtnvg__triangles(RTNVGcontext *rt, RTNVGcall *call) {
  // printf("__triangles\n");
  rtnvg__setUniforms(rt, call->uniformOffset, call->image);
  rtnvg__checkError(rt, "triangles fill");

  // printf("triangleOfft: %d, triangleCount: %d\n", call->triangleOffset,
  // call->triangleCount);
  // glDrawArrays(GL_TRIANGLES, call->triangleOffset, call->triangleCount);

  // render
  {
    std::vector<float> vertices;
    std::vector<float> texcoords;
    std::vector<unsigned int> faces;

    // Convert geometry to nanort friendly format.

    // TRIANGLES.
    {
      int npolys = call->triangleCount / 3;
      for (int n = 0; n < npolys; n++) {
        faces.push_back(3 * n + 0);
        faces.push_back(3 * n + 1);
        faces.push_back(3 * n + 2);

        for (int k = 0; k < 3; k++) {
          // Adjust Z index to solve triangle overlapping.
          vertices.push_back(rt->verts[call->triangleOffset + 3 * n + k].x);
          vertices.push_back(rt->verts[call->triangleOffset + 3 * n + k].y);
          vertices.push_back(-(float)n);

          texcoords.push_back(rt->verts[call->triangleOffset + 3 * n + k].u);
          texcoords.push_back(rt->verts[call->triangleOffset + 3 * n + k].v);
        }
      }
    }

    // printf("vertices.size = %d\n", (int)vertices.size() / 3);
    // printf("faces.size = %d\n", (int)faces.size() / 3);

    if (faces.size() > 0) {

      unsigned char *rgba = rt->pixels;

      nanort::BVHBuildOptions options; // Use default option

      // printf("  BVH build option:\n");
      // printf("    # of leaf primitives: %d\n", options.minLeafPrimitives);
      // printf("    SAH binsize         : %d\n", options.binSize);

      nanort::BVHAccel accel;
      bool ret =
          accel.Build(&vertices.at(0), &faces.at(0), (unsigned int)faces.size() / 3, options);
      assert(ret);
      (void)ret;

      // nanort::BVHBuildStatistics stats = accel.GetStatistics();

      // printf("  BVH statistics:\n");
      // printf("    # of leaf   nodes: %d\n", stats.numLeafNodes);
      // printf("    # of branch nodes: %d\n", stats.numBranchNodes);
      // printf("  Max tree depth   : %d\n", stats.maxTreeDepth);
      float bmin[3], bmax[3];
      accel.BoundingBox(bmin, bmax);
      // printf("  Bmin               : %f, %f, %f\n", bmin[0], bmin[1],
      // bmin[2]);
      // printf("  Bmax               : %f, %f, %f\n", bmax[0], bmax[1],
      // bmax[2]);

      int bound[4]; // l,t,r,b
      bound[0] = (int)bmin[0] - 1;
      bound[1] = (int)bmin[1] - 1;
      bound[2] = (int)bmax[0] + 1;
      bound[3] = (int)bmax[1] + 1;
      if (bound[0] < 0)           bound[0] = 0;
      if (bound[1] < 0)           bound[1] = 0;
      if (bound[2] >= rt->width)  bound[2] = rt->width - 1;
      if (bound[3] >= rt->height) bound[3] = rt->height - 1;
      // Shoot rays.
      for (int y = bound[1]; y < bound[3]; y++) {
        for (int x = bound[0]; x < bound[2]; x++) {

          int nsamples = 1;
          if (rt->flags & NVG_ANTIALIAS) {
            nsamples = 2; // 2x2 super sampling.
          }

          float pixelCol[4] = {0.0f, 0.0f, 0.0f, 0.0f};
          for (int sy = 0; sy < nsamples; sy++) {
            for (int sx = 0; sx < nsamples; sx++) {

              // Use multi-hit ray traversal to detect overdraw.
              nanort::StackVector<nanort::Intersection, 128> isects;
              int maxIsects = 128;

              // Simple ortho camera.

              nanort::Ray ray;
              ray.org[0] = x + ((float)sx + 0.5f) / (float)nsamples;
              ray.org[1] = y + ((float)sy + 0.5f) / (float)nsamples;
              ray.org[2] = 1.0f;

              ray.dir[0] = 0.0f;
              ray.dir[1] = 0.0f;
              ray.dir[2] = -1.0f;

              bool hit = accel.MultiHitTraverse(
                  isects, maxIsects, &vertices.at(0), &faces.at(0), ray);

              if (hit) {

                float fragCol[4];

                // Assume larger i => layer in back
                for (size_t i = 0; i < isects->size(); i++) {
                  float U = isects[i].u;
                  float V = isects[i].v;
                  // Compute interpolated texcoord.
                  int f0 = faces[3 * isects[i].faceID + 0];
                  int f1 = faces[3 * isects[i].faceID + 1];
                  int f2 = faces[3 * isects[i].faceID + 2];
                  float tu = (1.0f - U - V) * texcoords[2 * f0 + 0] +
                             U * texcoords[2 * f1 + 0] +
                             V * texcoords[2 * f2 + 0];
                  float tv = (1.0f - U - V) * texcoords[2 * f0 + 1] +
                             U * texcoords[2 * f1 + 1] +
                             V * texcoords[2 * f2 + 1];
                  RTNVGfragUniforms *frag =
                      nvg__fragUniformPtr(rt, call->uniformOffset);
                  rtnvg__shade(fragCol, rt, frag, ray.org[0], ray.org[1], tu,
                               tv, call->image);
                  pixelCol[0] += fragCol[0];
                  pixelCol[1] += fragCol[1];
                  pixelCol[2] += fragCol[2];
                  pixelCol[3] += fragCol[3];
                }
              }
            }
          }

          float ndiv = 1.0f / (nsamples * nsamples);
          pixelCol[0] *= ndiv;
          pixelCol[1] *= ndiv;
          pixelCol[2] *= ndiv;
          pixelCol[3] *= ndiv; // @fixme
          rtnvg__alphaBlend(&rgba[4 * (y * rt->width + x) + 0], pixelCol);
        }
      }
    }
  }
}

static void rtnvg__renderCancel(void *uptr) {
  // printf("__renderCancel\n");
  RTNVGcontext *rt = (RTNVGcontext *)uptr;
  rt->nverts = 0;
  rt->npaths = 0;
  rt->ncalls = 0;
  rt->nuniforms = 0;
}

static void rtnvg__renderFlush(void *uptr) {
  // printf("__renderFlush\n");
  RTNVGcontext *rt = (RTNVGcontext *)uptr;

  if (rt->ncalls > 0) {
    // printf("nverts = %d\n", rt->nverts);
    // printf("ncalls = %d\n", rt->ncalls);
    for (int i = 0; i < rt->ncalls; i++) {
      RTNVGcall *call = &rt->calls[i];
      if (call->type == RTNVG_FILL)
        rtnvg__fill(rt, call);
      else if (call->type == RTNVG_CONVEXFILL)
        rtnvg__convexFill(rt, call);
      else if (call->type == RTNVG_STROKE)
        rtnvg__stroke(rt, call);
      else if (call->type == RTNVG_TRIANGLES)
        rtnvg__triangles(rt, call);
    }

    rtnvg__bindTexture(rt, 0);
  }

  // Reset calls
  rt->nverts = 0;
  rt->npaths = 0;
  rt->ncalls = 0;
  rt->nuniforms = 0;
}

static int rtnvg__maxVertCount(const NVGpath *paths, int npaths) {
  int i, count = 0;
  for (i = 0; i < npaths; i++) {
    count += paths[i].nfill;
    count += paths[i].nstroke;
  }
  return count;
}

static RTNVGcall *rtnvg__allocCall(RTNVGcontext *rt) {
  RTNVGcall *ret = NULL;
  if (rt->ncalls + 1 > rt->ccalls) {
    RTNVGcall *calls;
    int ccalls =
        rtnvg__maxi(rt->ncalls + 1, 128) + rt->ccalls / 2; // 1.5x Overallocate
    calls = (RTNVGcall *)realloc(rt->calls, sizeof(RTNVGcall) * ccalls);
    if (calls == NULL)
      return NULL;
    rt->calls = calls;
    rt->ccalls = ccalls;
  }
  ret = &rt->calls[rt->ncalls++];
  memset(ret, 0, sizeof(RTNVGcall));
  return ret;
}

static int rtnvg__allocPaths(RTNVGcontext *rt, int n) {
  int ret = 0;
  if (rt->npaths + n > rt->cpaths) {
    RTNVGpath *paths;
    int cpaths =
        rtnvg__maxi(rt->npaths + n, 128) + rt->cpaths / 2; // 1.5x Overallocate
    paths = (RTNVGpath *)realloc(rt->paths, sizeof(RTNVGpath) * cpaths);
    if (paths == NULL)
      return -1;
    rt->paths = paths;
    rt->cpaths = cpaths;
  }
  ret = rt->npaths;
  rt->npaths += n;
  return ret;
}

static int rtnvg__allocVerts(RTNVGcontext *rt, int n) {
  // printf("alloc verts: %d\n", n);
  int ret = 0;
  if (rt->nverts + n > rt->cverts) {
    NVGvertex *verts;
    int cverts =
        rtnvg__maxi(rt->nverts + n, 4096) + rt->cverts / 2; // 1.5x Overallocate
    verts = (NVGvertex *)realloc(rt->verts, sizeof(NVGvertex) * cverts);
    if (verts == NULL)
      return -1;
    rt->verts = verts;
    rt->cverts = cverts;
  }
  ret = rt->nverts;
  rt->nverts += n;
  return ret;
}

static int rtnvg__allocFragUniforms(RTNVGcontext *rt, int n) {
  int ret = 0, structSize = rt->fragSize;
  if (rt->nuniforms + n > rt->cuniforms) {
    unsigned char *uniforms;
    int cuniforms = rtnvg__maxi(rt->nuniforms + n, 128) +
                    rt->cuniforms / 2; // 1.5x Overallocate
    uniforms = (unsigned char *)realloc(rt->uniforms, structSize * cuniforms);
    if (uniforms == NULL)
      return -1;
    rt->uniforms = uniforms;
    rt->cuniforms = cuniforms;
  }
  ret = rt->nuniforms * structSize;
  rt->nuniforms += n;
  return ret;
}

static RTNVGfragUniforms *nvg__fragUniformPtr(RTNVGcontext *rt, int i) {
  return (RTNVGfragUniforms *)&rt->uniforms[i];
}

static void rtnvg__vset(NVGvertex *vtx, float x, float y, float u, float v) {
  // printf("v: (x, y, u, v) = %f, %f, %f, %f\n", x, y, u, v);
  vtx->x = x;
  vtx->y = y;
  vtx->u = u;
  vtx->v = v;
}

static void rtnvg__renderFill(void *uptr, NVGpaint *paint, NVGcompositeOperationState compositeOperation, NVGscissor *scissor,
                              float fringe, const float *bounds,
                              const NVGpath *paths, int npaths) {
  // printf("__renderFill\n");
  RTNVGcontext *rt = (RTNVGcontext *)uptr;
  RTNVGcall *call = rtnvg__allocCall(rt);
  NVGvertex *quad;
  RTNVGfragUniforms *frag;
  int i, maxverts, offset;

  if (call == NULL)
    return;

  call->type = RTNVG_FILL;
  call->pathOffset = rtnvg__allocPaths(rt, npaths);
  if (call->pathOffset == -1)
    goto error;
  call->pathCount = npaths;
  call->image = paint->image;
  // TODO(syoyo): Implement
  // call->blendFunc = glnvg_blendCompositeOperation(compositeOperation)
  (void)compositeOperation;

  // printf("pathOffset = %d\n", call->pathOffset);

  if (npaths == 1 && paths[0].convex)
    call->type = RTNVG_CONVEXFILL;

  // Allocate vertices for all the paths.
  maxverts = rtnvg__maxVertCount(paths, npaths) + 6;
  // printf("maxverts = %d\n", maxverts);
  offset = rtnvg__allocVerts(rt, maxverts);
  if (offset == -1)
    goto error;

  // printf("npaths = %d\n", npaths);
  for (i = 0; i < npaths; i++) {
    RTNVGpath *copy = &rt->paths[call->pathOffset + i];
    const NVGpath *path = &paths[i];
    memset(copy, 0, sizeof(RTNVGpath));
    if (path->nfill > 0) {
      copy->fillOffset = offset;
      copy->fillCount = path->nfill;
      memcpy(&rt->verts[offset], path->fill, sizeof(NVGvertex) * path->nfill);
      // printf("nfill = %d\n", path->nfill);
      // for (int k = 0; k < path->nfill; k++) {
      //  printf("v[%d] = %f, %f, %f, %f\n", k,
      //    path->fill[k].x,
      //    path->fill[k].y,
      //    path->fill[k].u,
      //    path->fill[k].v);
      //}
      offset += path->nfill;
    }
    if (path->nstroke > 0) {
      copy->strokeOffset = offset;
      copy->strokeCount = path->nstroke;
      memcpy(&rt->verts[offset], path->stroke,
             sizeof(NVGvertex) * path->nstroke);
      offset += path->nstroke;
    }
  }

  // Quad
  call->triangleOffset = offset;
  call->triangleCount = 6;
  quad = &rt->verts[call->triangleOffset];
  rtnvg__vset(&quad[0], bounds[0], bounds[3], 0.5f, 1.0f);
  rtnvg__vset(&quad[1], bounds[2], bounds[3], 0.5f, 1.0f);
  rtnvg__vset(&quad[2], bounds[2], bounds[1], 0.5f, 1.0f);

  rtnvg__vset(&quad[3], bounds[0], bounds[3], 0.5f, 1.0f);
  rtnvg__vset(&quad[4], bounds[2], bounds[1], 0.5f, 1.0f);
  rtnvg__vset(&quad[5], bounds[0], bounds[1], 0.5f, 1.0f);

  // Setup uniforms for draw calls
  if (call->type == RTNVG_FILL) {
    call->uniformOffset = rtnvg__allocFragUniforms(rt, 2);
    if (call->uniformOffset == -1)
      goto error;
    // Simple shader for stencil
    frag = nvg__fragUniformPtr(rt, call->uniformOffset);
    memset(frag, 0, sizeof(*frag));
    frag->strokeThr = -1.0f;
    frag->type = NSVG_SHADER_SIMPLE;
    // Fill shader
    rtnvg__convertPaint(
        rt, nvg__fragUniformPtr(rt, call->uniformOffset + rt->fragSize), paint,
        scissor, fringe, fringe, -1.0f);
  } else {
    call->uniformOffset = rtnvg__allocFragUniforms(rt, 1);
    if (call->uniformOffset == -1)
      goto error;
    // Fill shader
    rtnvg__convertPaint(rt, nvg__fragUniformPtr(rt, call->uniformOffset), paint,
                        scissor, fringe, fringe, -1.0f);
  }

  return;

error:
  // We get here if call alloc was ok, but something else is not.
  // Roll back the last call to prevent drawing it.
  if (rt->ncalls > 0)
    rt->ncalls--;
}

static void rtnvg__renderStroke(void *uptr, NVGpaint *paint,
                                NVGcompositeOperationState compositeOperation,
                                NVGscissor *scissor, float fringe,
                                float strokeWidth, const NVGpath *paths,
                                int npaths) {
  // printf("renderStroke\n");
  RTNVGcontext *rt = (RTNVGcontext *)uptr;
  RTNVGcall *call = rtnvg__allocCall(rt);
  int i, maxverts, offset;

  if (call == NULL)
    return;

  call->type = RTNVG_STROKE;
  call->pathOffset = rtnvg__allocPaths(rt, npaths);
  if (call->pathOffset == -1)
    goto error;
  call->pathCount = npaths;
  call->image = paint->image;
  // TODO(syoyo): Implement
  // call->blendFunc = glnvg_blendCompositeOperation(compositeOperation)
  (void)compositeOperation;

  // Allocate vertices for all the paths.
  maxverts = rtnvg__maxVertCount(paths, npaths);
  offset = rtnvg__allocVerts(rt, maxverts);
  if (offset == -1)
    goto error;

  for (i = 0; i < npaths; i++) {
    RTNVGpath *copy = &rt->paths[call->pathOffset + i];
    const NVGpath *path = &paths[i];
    memset(copy, 0, sizeof(RTNVGpath));
    if (path->nstroke) {
      copy->strokeOffset = offset;
      copy->strokeCount = path->nstroke;
      memcpy(&rt->verts[offset], path->stroke,
             sizeof(NVGvertex) * path->nstroke);
      offset += path->nstroke;
    }
  }

  if (rt->flags & NVG_STENCIL_STROKES) {
    // Fill shader
    call->uniformOffset = rtnvg__allocFragUniforms(rt, 2);
    if (call->uniformOffset == -1)
      goto error;

    rtnvg__convertPaint(rt, nvg__fragUniformPtr(rt, call->uniformOffset), paint,
                        scissor, strokeWidth, fringe, -1.0f);
    rtnvg__convertPaint(
        rt, nvg__fragUniformPtr(rt, call->uniformOffset + rt->fragSize), paint,
        scissor, strokeWidth, fringe, 1.0f - 0.5f / 255.0f);

  } else {
    // Fill shader
    call->uniformOffset = rtnvg__allocFragUniforms(rt, 1);
    if (call->uniformOffset == -1)
      goto error;
    rtnvg__convertPaint(rt, nvg__fragUniformPtr(rt, call->uniformOffset), paint,
                        scissor, strokeWidth, fringe, -1.0f);
  }

  return;

error:
  // We get here if call alloc was ok, but something else is not.
  // Roll back the last call to prevent drawing it.
  if (rt->ncalls > 0)
    rt->ncalls--;
}

static void rtnvg__renderTriangles(void *uptr, NVGpaint *paint,
                                   NVGcompositeOperationState compositeOperation,
                                   NVGscissor *scissor, const NVGvertex *verts,
                                   int nverts) {
  // printf("renderTriangles\n");
  RTNVGcontext *rt = (RTNVGcontext *)uptr;
  RTNVGcall *call = rtnvg__allocCall(rt);
  RTNVGfragUniforms *frag;

  if (call == NULL)
    return;

  call->type = RTNVG_TRIANGLES;
  call->image = paint->image;
  // TODO(syoyo): Implement
  // call->blendFunc = glnvg_blendCompositeOperation(compositeOperation)
  (void)compositeOperation;

  // Allocate vertices for all the paths.
  call->triangleOffset = rtnvg__allocVerts(rt, nverts);
  if (call->triangleOffset == -1)
    goto error;
  call->triangleCount = nverts;

  memcpy(&rt->verts[call->triangleOffset], verts, sizeof(NVGvertex) * nverts);

  // Fill shader
  call->uniformOffset = rtnvg__allocFragUniforms(rt, 1);
  if (call->uniformOffset == -1)
    goto error;
  frag = nvg__fragUniformPtr(rt, call->uniformOffset);
  rtnvg__convertPaint(rt, frag, paint, scissor, 1.0f, 1.0f, -1.0f);
  frag->type = NSVG_SHADER_IMG;

  return;

error:
  // We get here if call alloc was ok, but something else is not.
  // Roll back the last call to prevent drawing it.
  if (rt->ncalls > 0)
    rt->ncalls--;
}

static void rtnvg__renderDelete(void *uptr) {
  // printf("__renderDelete\n");
  RTNVGcontext *rt = (RTNVGcontext *)uptr;
  int i;
  if (rt == NULL)
    return;

  rtnvg__deleteShader(&rt->shader);

  // if (rt->vertBuf != 0)
  //	glDeleteBuffers(1, &rt->vertBuf);

  for (i = 0; i < rt->ntextures; i++) {
    // if (rt->textures[i].tex != 0 && (rt->textures[i].flags &
    // NVG_IMAGE_NODELETE) == 0)
    //	glDeleteTextures(1, &rt->textures[i].tex);
  }
  free(rt->textures);

  free(rt->paths);
  free(rt->verts);
  free(rt->uniforms);
  free(rt->calls);

  free(rt);
}

inline NVGcontext *nvgCreateRT(int flags, int w, int h, int clrColor) {
  NVGparams params;
  NVGcontext *ctx = NULL;
  RTNVGcontext *rt = (RTNVGcontext *)malloc(sizeof(RTNVGcontext));
  if (rt == NULL)
    goto error;
  memset(rt, 0, sizeof(RTNVGcontext));

  memset(&params, 0, sizeof(params));
  params.renderCreate = rtnvg__renderCreate;
  params.renderCreateTexture = rtnvg__renderCreateTexture;
  params.renderDeleteTexture = rtnvg__renderDeleteTexture;
  params.renderUpdateTexture = rtnvg__renderUpdateTexture;
  params.renderGetTextureSize = rtnvg__renderGetTextureSize;
  params.renderViewport = rtnvg__renderViewport;
  params.renderCancel = rtnvg__renderCancel;
  params.renderFlush = rtnvg__renderFlush;
  params.renderFill = rtnvg__renderFill;
  params.renderStroke = rtnvg__renderStroke;
  params.renderTriangles = rtnvg__renderTriangles;
  params.renderDelete = rtnvg__renderDelete;
  params.userPtr = rt;
  params.edgeAntiAlias = flags & NVG_ANTIALIAS ? 1 : 0;

  rt->flags = flags;

  rt->width = w;
  rt->height = h;
  rt->pixels = (unsigned char *)malloc(rt->width * rt->height * 4);
  for (size_t i = 0; i < rt->width * rt->height; i++)
    memcpy(rt->pixels + 4 * i, &clrColor, 4);

  ctx = nvgCreateInternal(&params);
  if (ctx == NULL)
    goto error;

  return ctx;

error:
  free(rt->pixels);
  // 'gl' is freed by nvgDeleteInternal.
  if (ctx != NULL)
    nvgDeleteInternal(ctx);
  return NULL;
}

inline void nvgDeleteRT(NVGcontext *ctx) {
  RTNVGcontext *rt = (RTNVGcontext *)nvgInternalParams(ctx)->userPtr;
  free(rt->pixels);
  // printf("delete\n");
  nvgDeleteInternal(ctx);
}

inline int nvglCreateImageFromHandle(NVGcontext *ctx, unsigned int textureId, int w,
                              int h, int imageFlags) {
  RTNVGcontext *rt = (RTNVGcontext *)nvgInternalParams(ctx)->userPtr;
  RTNVGtexture *tex = rtnvg__allocTexture(rt);

  printf("Not supported.\n");
  exit(-1);
  if (tex == NULL)
    return 0;

  tex->type = NVG_TEXTURE_RGBA;
  tex->tex = textureId;
  tex->flags = imageFlags;
  tex->width = w;
  tex->height = h;

  return tex->id;
}

inline unsigned int nvglImageHandle(NVGcontext *ctx, int image) {
  RTNVGcontext *rt = (RTNVGcontext *)nvgInternalParams(ctx)->userPtr;
  RTNVGtexture *tex = rtnvg__findTexture(rt, image);
  return tex->tex;
}

inline void nvgClearBackgroundRT(NVGcontext *ctx, float r, float g, float b, float a) {
  RTNVGcontext *rt = (RTNVGcontext *)nvgInternalParams(ctx)->userPtr;
  unsigned char red = ftouc(r);
  unsigned char green = ftouc(g);
  unsigned char blue = ftouc(b);
  unsigned char alpha = ftouc(a);
  for (size_t i = 0; i < rt->width * rt->height; i++) {
    rt->pixels[4 * i + 0] = red;
    rt->pixels[4 * i + 1] = green;
    rt->pixels[4 * i + 2] = blue;
    rt->pixels[4 * i + 3] = alpha;
  }
}

inline unsigned char *nvgReadPixelsRT(NVGcontext *ctx) {
  RTNVGcontext *rt = (RTNVGcontext *)nvgInternalParams(ctx)->userPtr;
  return rt->pixels;
}

#endif /* NANOVG_RT_IMPLEMENTATION */
