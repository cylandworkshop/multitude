#include <cstdio>

#include <FixedPoints.h>
#include <FixedPointsCommon.h>

#include <cmath>
#include <stdexcept>
#include <iostream>
#include <thread>
#include <chrono>

// #include "sin.hpp"
#include "math/sqrt.hpp"
#include "math/vec.hpp"
#include "font.hpp"
#include "scene.hpp"
#include "encoder.hpp"

#include <sdlpp.hpp>

using fnum = SQ7x8;
using namespace std::chrono_literals;

using Vec2 = Vec2Base<fp_t>;

constexpr size_t FRAC_BITS = 8;
constexpr size_t FRAC_RANGE = 256;

float round_to_frac_range(float v)
{
    float step = 1.0 / FRAC_RANGE;
    float ipart;
    auto fpart = modff(v, &ipart);
    fpart = roundf(fpart * FRAC_RANGE);

    return ipart + fpart * step;
}

void print_sine()
{
    printf("constexpr fp_t SIN_LOOKUP_TABLE[] {\n");

    constexpr float step = 1.0 / FRAC_RANGE;
    for (size_t i = 0; i <= FRAC_RANGE / 4; ++i) {
        float const v = step * i;
        auto const fv = sinf(v * 2 * M_PI);
        auto const rfv = round_to_frac_range(fv);
        printf("\t%.8f,\n", rfv);
    }

    printf("};\n");
}

// void print_graph()
// {
//     constexpr fp_t step = 1.0 / FRAC_RANGE;
//     for (size_t i = 0; i <= FRAC_RANGE; ++i)
//     {
//         float sx = static_cast<float>(Sin(i * step));
//         float rsx = sinf(float(i * step) * 2 * M_PI);
//         printf("\t%.8f, %.8f,\n", rsx, sx);
//     }
// }

bool SampleFont(char c, fp_t x, fp_t y)
{
    uint8_t const width = pgm_read_byte(FONT + FONT_WIDTH_OFFSET);
    uint8_t const height = pgm_read_byte(FONT + FONT_HEIGHT_OFFSET);

    return SampleFont(c, roundFixed(x * width).getInteger(), roundFixed(y * height).getInteger());
}

constexpr size_t SCREEN_WIDTH = 16;
constexpr size_t SCREEN_HEIGHT = 32;
constexpr size_t SCREEN_NORM = SCREEN_HEIGHT;

constexpr fp_t LENS_RADIUS = 0.25;
constexpr fp_t LENS_SQUARE_RADIUS = LENS_RADIUS * LENS_RADIUS;
constexpr fp_t FONT_SCALE = 0.5;

constexpr size_t WINDOW_WIDTH = 800;
constexpr size_t WINDOW_HEIGHT = 800;

Vec2Base<size_t> CoordToScreen(Vec2 p)
{
    auto x = ((float(p.x) + 1) / 2) * WINDOW_WIDTH;
    auto y = WINDOW_HEIGHT - ((float(p.y) + 1) / 2) * WINDOW_HEIGHT;
    return Vec2Base<size_t>{size_t(x), size_t(y)};
}

void DrawPoint(sdl::Renderer& renderer, Vec2 p)
{
    auto [x, y] = CoordToScreen(p);
    SDL_Rect rect{x - 2, y - 2, 4 , 4};
    renderer.fillRect(&rect);
}

uint32_t millis()
{
    static const auto start = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
}

int main()
{
    EncoderState ENCODER = {};
    float ENCODER_PREV_ANGLE = 0;

    std::cout << "Deltas: " << (float(PIXEL_HALF_STEP) / float(fp_t::Epsilon)) << std::endl;
    sdl::Init init{SDL_INIT_EVERYTHING};
    sdl::Window w{"Ledder", 63, 126, 800, 800, SDL_WINDOW_BORDERLESS};
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    sdl::Renderer renderer(w.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    sdl::EventHandler e;
    bool done = false;
    e.quit = [&done](const SDL_QuitEvent &)
    {
      done = true;
    };

    uint32_t prevT = millis();

    while(!done)
    {
        while(e.poll()) {}

        auto const t = millis();

        if (Duration(prevT, t) > 100)
        {
            ENCODER.Update(t, ENCODER.state + 2);
            prevT = t;
        }

        auto const angle = ENCODER.GetAngle(t);

        if (angle == ENCODER_PREV_ANGLE)
        {
            continue;
        }

        ENCODER_PREV_ANGLE = angle;


        renderer.setDrawColor(0xff, 0xff, 0xff, 0xff);
        renderer.clear();
        renderer.setDrawColor(0x00, 0x00, 0x00, 0xff);

        /*for (size_t x = 0; x < WINDOW_WIDTH; ++x)
        {
            for (size_t y = 0; y < WINDOW_HEIGHT; ++y)
            {
                fp_t x_norm = (0.5 - float(x) / (WINDOW_WIDTH + 1 )) * 2;
                fp_t y_norm = (0.5 - float(y) / (WINDOW_HEIGHT + 1)) * 2;

                auto const r = Sqrt(x_norm * x_norm + y_norm * y_norm);
                if (r >= SCENE.R_IN && r <= SCENE.R_IN + 1.5 * PIXEL_STEP || r <= SCENE.R_OUT && r >= SCENE.R_OUT - 1.5 * PIXEL_STEP)
                {
                    renderer.drawPoint(x, y);
                }
            }
        }*/

        renderer.setDrawColor(0xff, 0x00, 0xff, 0xff);

        ForEachDisplayPixel(angle, [&](Vec2 p, auto, auto)
        {
            auto const r = Sqrt(p.x * p.x + p.y * p.y);
            auto const midpoint = (SCENE.R_IN + SCENE.R_OUT) / 2;
            if (absFixed(r - SCENE.R_IN) <= 2 * PIXEL_STEP || absFixed(r - SCENE.R_OUT) <= 2 * PIXEL_STEP)
            {
                DrawPoint(renderer, p);
            }
        });

        // // DrawPoint(renderer, DisplayCenter(0.25));
        // renderer.setDrawColor(0x00, 0x00, 0xff, 0xff);
        // DrawPoint(renderer, DisplayPixel(0.5, 0, 0));
        // // DrawPoint(renderer, DisplayPixel(0.75, 0, 0));
        // DrawPoint(renderer, Vec2{-0.5, -0.5});
        renderer.present();
        // std::this_thread::sleep_for(16ms);
    }
    // auto const c = 'A';

    // for (size_t j = 0; j < 32; ++j)
    // {
    //     for (size_t i = 0; i < 24; ++i)
    //     {
    //         auto pixel = SampleFont(c, i, j);
    //         if (pixel == 0)
    //         {
    //             std::cout << "  ";
    //         }
    //         else{
    //             std::cout << "**";
    //         }
    //     }

    //     std::cout << std::endl;
    // }

    // return 0;
}