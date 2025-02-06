#include "std.h"

import application;

auto main(int argc, char *argv[]) -> int {
    auto app = Application("Hello World!", 640, 480);
    app.run();
    return 0;
}
