#include "stb_image_ext.h"

#include <cstdint>

static void* mallocDefault(size_t size)                   { return malloc(size); }
static void* reallocDefault(void* oldPtr, size_t size)    { return realloc(oldPtr, size); }
static void  freeDefault(void* ptr)                       { free(ptr); }

static StbiMemAllocFunc gAllocFunc      = mallocDefault;
static StbiMemReallocFunc gReallocFunc  = reallocDefault;
static StbiMemFreeFunc gFreeFunc        = freeDefault;

void stbi_set_allocator_functions(StbiMemAllocFunc allocFunc, StbiMemReallocFunc reallocFunc, StbiMemFreeFunc freeFunc)
{
    gAllocFunc = allocFunc;
    gReallocFunc = reallocFunc;
    gFreeFunc = freeFunc;
}

// stb_image configuration
#define STBI_MALLOC(sz)           gAllocFunc(sz)
#define STBI_REALLOC(p,newsz)     gReallocFunc(p,newsz)
#define STBI_FREE(p)              gFreeFunc(p)

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP

#define STBI_NO_STDIO
#define STBI_FAILURE_USERMSG
#define STBI_MAX_DIMENSIONS 16384

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ============================================================================
// Расширение для загрузки JPEG напрямую в RGB565 с переиспользованием буфера
// ============================================================================

// TODO: stbi__YCbCr_to_RGB565_simd

static void stbi__YCbCr_to_RGB565_row(stbi_uc *out, const stbi_uc *y, const stbi_uc *pcb, const stbi_uc *pcr, int count, int step)
{
   int i;
   for (i = 0; i < count; ++i) {
      int y_fixed = (y[i] << 20) + (1 << 19);
      int r, g, b;
      int cr = pcr[i] - 128;
      int cb = pcb[i] - 128;
      r = y_fixed +  cr * stbi__float2fixed(1.40200f);
      g = y_fixed + (cr * -stbi__float2fixed(0.71414f)) + ((cb * -stbi__float2fixed(0.34414f)) & 0xffff0000);
      b = y_fixed                                      +   cb * stbi__float2fixed(1.77200f);
      r >>= 20;
      g >>= 20;
      b >>= 20;
      if ((unsigned) r > 255) { if (r < 0) r = 0; else r = 255; }
      if ((unsigned) g > 255) { if (g < 0) g = 0; else g = 255; }
      if ((unsigned) b > 255) { if (b < 0) b = 0; else b = 255; }

      // Упаковка в 16-битный RGB565 (Little Endian)
      uint16_t rgb565 = (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
      out[0] = (stbi_uc)(rgb565 & 0xFF);
      out[1] = (stbi_uc)(rgb565 >> 8);
      out += step;
   }
}

static stbi_uc *stbi__load_jpeg_rgb565(stbi__jpeg *z, int *out_x, int *out_y, int *comp)
{
   int n, decode_n, is_rgb;
   z->s->img_n = 0;

   if (!stbi__decode_jpeg_image(z)) {
      stbi__cleanup_jpeg(z);
      return NULL;
   }

   n = z->s->img_n;
   if (comp) *comp = n;
   is_rgb = z->s->img_n == 3 && (z->rgb == 3 || (z->app14_color_transform == 0 && !z->jfif));
   decode_n = (z->s->img_n == 3 && !is_rgb) ? 3 : z->s->img_n;

   // Поддерживаем только стандартный цветной (3 канала) или ч/б (1 канал) JPEG
   if (decode_n != 3 && decode_n != 1) {
      stbi__cleanup_jpeg(z);
      return stbi__errpuc("unsupported channels", "JPEG must be 1 or 3 channels for RGB565");
   }

   {
      int k;
      unsigned int i, j;
      stbi_uc *output = NULL;
      stbi_uc *coutput[4] = { NULL, NULL, NULL, NULL };

      stbi__resample res_comp[4];

      for (k = 0; k < decode_n; ++k) {
         stbi__resample *r = &res_comp[k];
         z->img_comp[k].linebuf = (stbi_uc *) stbi__malloc(z->s->img_x + 3);
         if (!z->img_comp[k].linebuf) { stbi__cleanup_jpeg(z); return stbi__errpuc("outofmem", "Out of memory"); }

         r->hs      = z->img_h_max / z->img_comp[k].h;
         r->vs      = z->img_v_max / z->img_comp[k].v;
         r->ystep   = r->vs >> 1;
         r->w_lores = (z->s->img_x + r->hs - 1) / r->hs;
         r->ypos    = 0;
         r->line0   = r->line1 = z->img_comp[k].data;

         if      (r->hs == 1 && r->vs == 1) r->resample = resample_row_1;
         else if (r->hs == 1 && r->vs == 2) r->resample = stbi__resample_row_v_2;
         else if (r->hs == 2 && r->vs == 1) r->resample = stbi__resample_row_h_2;
         else if (r->hs == 2 && r->vs == 2) r->resample = z->resample_row_hv_2_kernel;
         else                               r->resample = stbi__resample_row_generic;
      }

      // ХАК ЭКОНОМИИ ОЗУ: выделяем 1 буфер размером W * H * 2 байта.
      // Вторая половина буфера временно хранит строку Y, а с начала пишется RGB565!
      output = (stbi_uc *) stbi__malloc_mad3(2, z->s->img_x, z->s->img_y, 0);
      if (!output) { stbi__cleanup_jpeg(z); return stbi__errpuc("outofmem", "Out of memory"); }

      // Буферы Cb и Cr нужны только на 1 строку каждый! (Экономим еще W*H*2 байт!)
      stbi_uc *row_y  = (stbi_uc *) stbi__malloc(z->s->img_x);
      stbi_uc *row_cb = (decode_n == 3) ? (stbi_uc *) stbi__malloc(z->s->img_x) : NULL;
      stbi_uc *row_cr = (decode_n == 3) ? (stbi_uc *) stbi__malloc(z->s->img_x) : NULL;

      if (!row_y || (decode_n == 3 && (!row_cb || !row_cr))) {
         STBI_FREE(output); STBI_FREE(row_y); STBI_FREE(row_cb); STBI_FREE(row_cr);
         stbi__cleanup_jpeg(z);
         return stbi__errpuc("outofmem", "Out of memory");
      }

      // Построчный ресемплинг и конвертация в RGB565 без хранения промежуточных полноразмерных каналов!
      for (j = 0; j < z->s->img_y; ++j) {
         stbi_uc *out_row = output + j * z->s->img_x * 2;
         stbi_uc *rows[3] = { row_y, row_cb, row_cr };

         for (k = 0; k < decode_n; ++k) {
            stbi__resample *r = &res_comp[k];
            int y_bot = r->ystep >= (r->vs >> 1);
            stbi_uc *resampled = r->resample(z->img_comp[k].linebuf,
                                             y_bot ? r->line1 : r->line0,
                                             y_bot ? r->line0 : r->line1,
                                             r->w_lores, r->hs);
            memcpy(rows[k], resampled, z->s->img_x);
            if (++r->ystep >= r->vs) {
               r->ystep = 0;
               r->line0 = r->line1;
               if (++r->ypos < z->img_comp[k].y)
                  r->line1 += z->img_comp[k].w2;
            }
         }

         if (decode_n == 3) {
            if (is_rgb) {
               for (i = 0; i < z->s->img_x; ++i) {
                  uint16_t rgb565 = (uint16_t)(((row_y[i] & 0xF8) << 8) | ((row_cb[i] & 0xFC) << 3) | (row_cr[i] >> 3));
                  out_row[i*2]     = (stbi_uc)(rgb565 & 0xFF);
                  out_row[i*2 + 1] = (stbi_uc)(rgb565 >> 8);
               }
            } else {
               stbi__YCbCr_to_RGB565_row(out_row, row_y, row_cb, row_cr, z->s->img_x, 2);
            }
         } else {
            // Grayscale -> RGB565
            for (i = 0; i < z->s->img_x; ++i) {
               stbi_uc g = row_y[i];
               uint16_t rgb565 = (uint16_t)(((g & 0xF8) << 8) | ((g & 0xFC) << 3) | (g >> 3));
               out_row[i*2]     = (stbi_uc)(rgb565 & 0xFF);
               out_row[i*2 + 1] = (stbi_uc)(rgb565 >> 8);
            }
         }
      }

      STBI_FREE(row_y);
      STBI_FREE(row_cb);
      STBI_FREE(row_cr);
      stbi__cleanup_jpeg(z);

      *out_x = z->s->img_x;
      *out_y = z->s->img_y;
      return output;
   }
}

STBIDEF stbi_uc *stbi_load_rgb565_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int /*desired_channels*/)
{
   stbi__context s;
   stbi__start_mem(&s, buffer, len);
   if (stbi__jpeg_test(&s)) {
      stbi__jpeg *j = (stbi__jpeg *) stbi__malloc(sizeof(stbi__jpeg));
      if (!j) return stbi__errpuc("outofmem", "Out of memory");
      memset(j, 0, sizeof(stbi__jpeg));
      j->s = &s;
      stbi__setup_jpeg(j);
      stbi_uc *res = stbi__load_jpeg_rgb565(j, x, y, channels_in_file);
      STBI_FREE(j);
      return res;
   }
   return stbi__errpuc("not jpeg", "stbi_load_rgb565 supports only JPEG");
}
