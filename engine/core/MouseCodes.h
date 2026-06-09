#pragma once

#include <cstdint>

using MouseCode = uint16_t;

namespace Mouse
{
    enum : MouseCode
    {
        Button0 = 0,   // 左键
        Button1 = 1,   // 右键
        Button2 = 2,   // 中键
        Button3 = 3,
        Button4 = 4,
        Button5 = 5,
        Button6 = 6,
        Button7 = 7,

        ButtonLeft   = Button0,
        ButtonRight  = Button1,
        ButtonMiddle = Button2,
    };
}
