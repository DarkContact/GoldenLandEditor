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

static void stbi__YCbCr_to_RGB565_row(stbi_uc *out, const stbi_uc *y, const stbi_uc *pcb, const stbi_uc *pcr, int count, int step)
{
   int i = 0;

#ifdef STBI_SSE2
   if (count >= 8) {
      __m128i signflip  = _mm_set1_epi8(-0x80);
      __m128i cr_const0 = _mm_set1_epi16((short)(1.40200f * 4096.0f + 0.5f));
      __m128i cr_const1 = _mm_set1_epi16(-(short)(0.71414f * 4096.0f + 0.5f));
      __m128i cb_const0 = _mm_set1_epi16(-(short)(0.34414f * 4096.0f + 0.5f));
      __m128i cb_const1 = _mm_set1_epi16((short)(1.77200f * 4096.0f + 0.5f));
      __m128i y_bias    = _mm_set1_epi8((char)128);

      for (; i + 7 < count; i += 8) {
         __m128i y_bytes  = _mm_loadl_epi64((const __m128i *)(y + i));
         __m128i cr_bytes = _mm_loadl_epi64((const __m128i *)(pcr + i));
         __m128i cb_bytes = _mm_loadl_epi64((const __m128i *)(pcb + i));

         __m128i cr_biased = _mm_xor_si128(cr_bytes, signflip);
         __m128i cb_biased = _mm_xor_si128(cb_bytes, signflip);

         __m128i yw  = _mm_unpacklo_epi8(y_bias, y_bytes);
         __m128i crw = _mm_unpacklo_epi8(_mm_setzero_si128(), cr_biased);
         __m128i cbw = _mm_unpacklo_epi8(_mm_setzero_si128(), cb_biased);

         __m128i yws = _mm_srli_epi16(yw, 4);
         __m128i cr0 = _mm_mulhi_epi16(cr_const0, crw);
         __m128i cb0 = _mm_mulhi_epi16(cb_const0, cbw);
         __m128i cb1 = _mm_mulhi_epi16(cbw, cb_const1);
         __m128i cr1 = _mm_mulhi_epi16(crw, cr_const1);

         __m128i rws = _mm_add_epi16(cr0, yws);
         __m128i gwt = _mm_add_epi16(cb0, yws);
         __m128i bws = _mm_add_epi16(yws, cb1);
         __m128i gws = _mm_add_epi16(gwt, cr1);

         __m128i rw = _mm_srai_epi16(rws, 4);
         __m128i bw = _mm_srai_epi16(bws, 4);
         __m128i gw = _mm_srai_epi16(gws, 4);

         __m128i rw_clamped = _mm_packus_epi16(rw, _mm_setzero_si128());
         __m128i gw_clamped = _mm_packus_epi16(gw, _mm_setzero_si128());
         __m128i bw_clamped = _mm_packus_epi16(bw, _mm_setzero_si128());

         // _mm_packus_epi16 пакует 8 элементов из первого аргумента в младшие 8 байт, а из второго (у нас ноль) в старшие 8 байт.
         // Значит, байты лежат в младших 8 байтах каждого из векторов _clamped.
         // Распакуем байты правильно в 16-битные слова без сдвига байт местами:
         __m128i r16 = _mm_unpacklo_epi8(rw_clamped, _mm_setzero_si128());
         __m128i g16 = _mm_unpacklo_epi8(gw_clamped, _mm_setzero_si128());
         __m128i b16 = _mm_unpacklo_epi8(bw_clamped, _mm_setzero_si128());

         // Упаковка в RGB565:
         // R: (r & 0xF8) -> сдвиг влево на 8 (получаем биты в позициях 11-15)
         // G: (g & 0xFC) -> сдвиг влево на 3 (позиции 5-10)
         // B: b -> сдвиг вправо на 3 (позиции 0-4)
         __m128i r_masked = _mm_slli_epi16(_mm_and_si128(r16, _mm_set1_epi16(0xF8)), 8);
         __m128i g_masked = _mm_slli_epi16(_mm_and_si128(g16, _mm_set1_epi16(0xFC)), 3);
         __m128i b_masked = _mm_srli_epi16(_mm_and_si128(b16, _mm_set1_epi16(0xFF)), 3);

         __m128i rgb565_vec = _mm_or_si128(_mm_or_si128(r_masked, g_masked), b_masked);

         // Сохраняем 8 шестнадцатибитных значений (16 байт) в выходной буфер
         if (step == 2) {
            _mm_storeu_si128((__m128i *)(out + i * 2), rgb565_vec);
         } else {
            // Если step != 2 (например, пишем в плотный буфер побайтово с шагом)
            uint16_t temp[8];
            _mm_storeu_si128((__m128i *)temp, rgb565_vec);
            for (int j = 0; j < 8; ++j) {
               uint16_t val = temp[j];
               out[(i + j) * step]     = (stbi_uc)(val & 0xFF);
               out[(i + j) * step + 1] = (stbi_uc)(val >> 8);
            }
         }
      }
   }
#endif

   for (; i < count; ++i) {
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
      out[i * step]     = (stbi_uc)(rgb565 & 0xFF);
      out[i * step + 1] = (stbi_uc)(rgb565 >> 8);
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
