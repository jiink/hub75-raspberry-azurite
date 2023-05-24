#include "led-matrix.h"
#include "graphics.h"
#include <wiringPi.h>

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>

// Pin defines
#define SW 29   // wPi assignment

using namespace rgb_matrix;

RGBMatrix *matrix;
FrameCanvas *canvas;
Font font;

enum ProgramState
{
    MENU,
    APPLET
};
ProgramState state = MENU;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
    interrupt_received = true;
}

void menuLoop()
{
    canvas->Fill(0, 10, 20);

    Color textColor(255, 100, 0);
    static int xPos = 8;
    if(digitalRead(SW) == LOW){
        xPos = (xPos + 1) % 50;
    }
    DrawText(canvas, font, xPos, 8, textColor, "qwerty");

    canvas = matrix->SwapOnVSync(canvas);
}

void loop()
{
    switch (state)
    {
        case MENU:
            menuLoop();
            break;
        case APPLET:
            // if (buttonPressed)
            // {
            //     buttonPressed = false;
            //     state = MENU;
            // }
            // applets[appletSelectedIndex].appletLoop();
            break;
    }
}

int main(int argc, char *argv[]) {
    RGBMatrix::Options matrix_options;
    RuntimeOptions runtime_opt;
    if (!ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt)) {
        return 1;
    }
  
    matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (matrix == NULL)
    {
        return 1;
    }
    canvas = matrix->CreateFrameCanvas();

    // GPIO setup
    if (wiringPiSetup () == -1)
    {
        return 1;
    }
    pinMode(SW, INPUT);
    pullUpDnControl(SW, PUD_UP);

    // It is always good to set up a signal handler to cleanly exit when we
    // receive a CTRL-C for instance. The DrawOnCanvas() routine is looking
    // for that.
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    const char bdf_font_file[64] = "5x7.bdf";
    if (!font.LoadFont(bdf_font_file)) {
        fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
        return 1;
    }

    while (!interrupt_received)
    {
        loop();
    }

    delete matrix;

    return 0;
}