/* Copyright (C) 2018 BARBOTIN Nicolas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once
#include <cstdint>

namespace m
{

    class Color
    {
    public:
        Color()
        {
            m_abgr = 0;
        }

        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        {
            m_r = r;
            m_g = g;
            m_b = b;
            m_a = a;
        }

        Color(float r, float g, float b, float a = 1.0f)
        {
            m_r = static_cast<uint8_t>(r * 255.0f);
            m_g = static_cast<uint8_t>(g * 255.0f);
            m_b = static_cast<uint8_t>(b * 255.0f);
            m_a = static_cast<uint8_t>(a * 255.0f);
        }

        Color &set(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        {
            m_r = r;
            m_g = g;
            m_b = b;
            m_a = a;
        }

        Color &set(float r, float g, float b, float a = 1.0f)
        {
            m_r = static_cast<uint8_t>(r * 255.0f);
            m_g = static_cast<uint8_t>(g * 255.0f);
            m_b = static_cast<uint8_t>(b * 255.0f);
            m_a = static_cast<uint8_t>(a * 255.0f);
        }

        uint8_t r() const
        {
            return m_r;
        }

        uint8_t g() const
        {
            return m_g;
        }

        uint8_t b() const
        {
            return m_b;
        }

        uint8_t a() const
        {
            return m_a;
        }

        //0xRRGGBBAA
        uint32_t rgba() const
        {
            return (m_r << 24) | (m_g << 16) | (m_b << 8) | m_a;
        }

        //0x00RRGGBB
        uint32_t rgb() const
        {
            return (m_r << 16) | (m_g << 8) | m_b;
        }

        //0xAARRGGBB
        uint32_t argb() const
        {
            return (m_a << 24) | (m_r << 16) | (m_g << 8) | m_b;
        }

        //0xAABBGGRR
        uint32_t abgr() const
        {
            return m_abgr;
        }

        //0xBBGGRRAA
        uint32_t bgra() const
        {
            return (m_b << 24) | (m_g << 16) | (m_r << 8) | m_a;
        }

        //0x00BBGGRR
        uint32_t bgr() const
        {
            return m_abgr & 0x00FFFFFF;
        }

        float rf() const
        {
            return static_cast<float>(m_r) / 255.0f;
        }

        float gf() const
        {
            return static_cast<float>(m_g) / 255.0f;
        }

        float bf() const
        {
            return static_cast<float>(m_b) / 255.0f;
        }

        float af() const
        {
            return static_cast<float>(m_a) / 255.0f;
        }

        //h in [0;360], s and v in [0;1]
        void toHSV(float &h, float &s, float &v) const
        {
            float r = static_cast<float>(m_r) / 255.0f;
            float g = static_cast<float>(m_g) / 255.0f;
            float b = static_cast<float>(m_b) / 255.0f;

            float fmin = r < g ? r : g;
            float fmax = r > g ? r : g;

            if(b < fmin)
                fmin = b;

            if(b > fmax)
                fmax = b;

            float delta = fmax - fmin;
            v = fmax;

            if(delta < 0.00001f || fmax <= 0.0f) {
                s = 0.0f;
                h = 0.0f;
                return;
            }

            s = delta / fmax;

            if(r >= fmax)
                h = (g - b) / delta;
            else if(g >= fmax)
                h = 2.0f + (b - r) / delta;
            else //b >= fmax
                h = 4.0f + (r - g) / delta;

            h *= 60.0f;
            while(h < 0.0f)
                h += 360.0f;

            while(h >= 360.0f)
                h -= 360.0f;
        }

        //h in [0;360], s and v in [0;1]
        static Color fromHSV(float h, float s, float v)
        {
            if(s <= 0.0f)
                return Color(v, v, v);

            if(h >= 360.0f)
                h = 0.0f;
            else
                h /= 60.0f;

            int i = static_cast<int>(h);
            float f = h - static_cast<float>(i);
            float p = v * (1.0f - s);
            float q = v * (1.0f - (s * f));
            float t = v * (1.0f - (s * (1.0f - f)));

            switch(i) {
                case 0:
                    return Color(v, t, p);

                case 1:
                    return Color(q, v, p);

                case 2:
                    return Color(p, v, t);

                case 3:
                    return Color(p, q, v);

                case 4:
                    return Color(t, p, v);

                case 5:
                default:
                    return Color(v, p, q);
            }
        }

        static Color fromRGBA(uint32_t color)
        {
            return Color(static_cast<uint8_t>((color & 0xFF000000) >> 24),
                         static_cast<uint8_t>((color & 0x00FF0000) >> 16),
                         static_cast<uint8_t>((color & 0x0000FF00) >> 8),
                         static_cast<uint8_t>( color & 0x000000FF));
        }

        static Color fromARGB(uint32_t color)
        {
            return Color(static_cast<uint8_t>((color & 0x00FF0000) >> 16),
                         static_cast<uint8_t>((color & 0x0000FF00) >> 8),
                         static_cast<uint8_t>( color & 0x000000FF),
                         static_cast<uint8_t>((color & 0xFF000000) >> 24));
        }

        static Color fromRGB(uint32_t color)
        {
            return Color(static_cast<uint8_t>((color & 0x00FF0000) >> 16),
                         static_cast<uint8_t>((color & 0x0000FF00) >> 8),
                         static_cast<uint8_t>( color & 0x000000FF));
        }

        static Color fromABGR(uint32_t color)
        {
            return Color(static_cast<uint8_t>( color & 0x000000FF),
                         static_cast<uint8_t>((color & 0x0000FF00) >> 8),
                         static_cast<uint8_t>((color & 0x00FF0000) >> 16),
                         static_cast<uint8_t>((color & 0xFF000000) >> 24));
        }

        static Color fromBGRA(uint32_t color)
        {
            return Color(static_cast<uint8_t>((color & 0x0000FF00) >> 8),
                         static_cast<uint8_t>((color & 0x00FF0000) >> 16),
                         static_cast<uint8_t>((color & 0xFF000000) >> 24),
                         static_cast<uint8_t>( color & 0x000000FF));
        }

        static Color fromBGR(uint32_t color)
        {
            return Color(static_cast<uint8_t>( color & 0x000000FF),
                         static_cast<uint8_t>((color & 0x0000FF00) >> 8),
                         static_cast<uint8_t>((color & 0x00FF0000) >> 16));
        }

    private:
        union {
            struct {
                uint8_t m_r;
                uint8_t m_g;
                uint8_t m_b;
                uint8_t m_a;
            };

            uint32_t m_abgr;
        };
    };

}
