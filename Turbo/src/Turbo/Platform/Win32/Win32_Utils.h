#pragma once

#include "Turbo/Core/KeyCodes.h"

#include <map>
#include <array>

#include <WinUser.h>

#define TBO_WINKEY(x, y)  y
#define TBO_KEYCODE(x, y)  { x, y }

namespace Turbo {

    using Win32Code = i32;

    namespace Utils {

        static inline Win32Code GetWin32CodeFromKeyCode(const KeyCode keyCode)
        {
            static constexpr std::array<Win32Code, Key::KeyCodeCount> s_WinCodes = {
                // Alphabet
                TBO_WINKEY(Key::A, 65),
                TBO_WINKEY(Key::B, 66),
                TBO_WINKEY(Key::C, 67),
                TBO_WINKEY(Key::D, 68),
                TBO_WINKEY(Key::E, 69),
                TBO_WINKEY(Key::F, 70),
                TBO_WINKEY(Key::G, 71),
                TBO_WINKEY(Key::H, 72),
                TBO_WINKEY(Key::I, 73),
                TBO_WINKEY(Key::J, 74),
                TBO_WINKEY(Key::K, 75),
                TBO_WINKEY(Key::L, 76),
                TBO_WINKEY(Key::M, 77),
                TBO_WINKEY(Key::N, 78),
                TBO_WINKEY(Key::O, 79),
                TBO_WINKEY(Key::P, 80),
                TBO_WINKEY(Key::Q, 81),
                TBO_WINKEY(Key::R, 82),
                TBO_WINKEY(Key::S, 83),
                TBO_WINKEY(Key::T, 84),
                TBO_WINKEY(Key::U, 85),
                TBO_WINKEY(Key::V, 86),
                TBO_WINKEY(Key::W, 87),
                TBO_WINKEY(Key::X, 88),
                TBO_WINKEY(Key::Y, 89),
                TBO_WINKEY(Key::Z, 90),

                // Controls
                TBO_WINKEY(Key::LeftShift, VK_LSHIFT),
                TBO_WINKEY(Key::RightShift, VK_RSHIFT),
                TBO_WINKEY(Key::LeftControl, VK_LCONTROL),
                TBO_WINKEY(Key::RightControl, VK_RCONTROL),
                TBO_WINKEY(Key::LeftAlt, VK_LMENU),
                TBO_WINKEY(Key::RightAlt, VK_RMENU),

                // Arrows
                TBO_WINKEY(Key::Left, VK_LEFT),
                TBO_WINKEY(Key::Up, VK_UP),
                TBO_WINKEY(Key::Right, VK_RIGHT),
                TBO_WINKEY(Key::Down, VK_DOWN),

                // General
                TBO_WINKEY(Key::Space, VK_SPACE),
                TBO_WINKEY(Key::Enter, VK_RETURN),
                TBO_WINKEY(Key::Escape, VK_ESCAPE),
                TBO_WINKEY(Key::Tab, VK_TAB),
            };

            if (keyCode >= Key::KeyCodeCount)
                return keyCode;

            return s_WinCodes[keyCode];
        }

        // TODO: Maybe there is a way to create a lookup map without std::map
        // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
        // Win32 keys are in order so maybe take advantage of that
        static inline KeyCode GetKeyCodeFromWin32Code(const Win32Code win32Code)
        {
            static std::map<Win32Code, KeyCode> s_KeyCodes = {
                // Alphabet
                TBO_KEYCODE(65, Key::A),
                TBO_KEYCODE(66, Key::B),
                TBO_KEYCODE(67, Key::C),
                TBO_KEYCODE(68, Key::D),
                TBO_KEYCODE(69, Key::E),
                TBO_KEYCODE(70, Key::F),
                TBO_KEYCODE(71, Key::G),
                TBO_KEYCODE(72, Key::H),
                TBO_KEYCODE(73, Key::I),
                TBO_KEYCODE(74, Key::J),
                TBO_KEYCODE(75, Key::K),
                TBO_KEYCODE(76, Key::L),
                TBO_KEYCODE(77, Key::M),
                TBO_KEYCODE(78, Key::N),
                TBO_KEYCODE(79, Key::O),
                TBO_KEYCODE(80, Key::P),
                TBO_KEYCODE(81, Key::Q),
                TBO_KEYCODE(82, Key::R),
                TBO_KEYCODE(83, Key::S),
                TBO_KEYCODE(84, Key::T),
                TBO_KEYCODE(85, Key::U),
                TBO_KEYCODE(86, Key::V),
                TBO_KEYCODE(87, Key::W),
                TBO_KEYCODE(88, Key::X),
                TBO_KEYCODE(89, Key::Y),
                TBO_KEYCODE(90, Key::Z),

                // Controls
               TBO_KEYCODE(VK_LSHIFT, Key::LeftShift),
               TBO_KEYCODE(VK_RSHIFT, Key::RightShift),
               TBO_KEYCODE(VK_LCONTROL, Key::LeftControl),
               TBO_KEYCODE(VK_RCONTROL, Key::RightControl),
               TBO_KEYCODE(VK_LMENU, Key::LeftAlt),
               TBO_KEYCODE(VK_RMENU, Key::RightAlt),
               TBO_KEYCODE(VK_MENU, Key::LeftAlt),
               TBO_KEYCODE(VK_CONTROL, Key::LeftControl),

               // Arrows
               TBO_KEYCODE(VK_LEFT, Key::Left),
               TBO_KEYCODE(VK_UP, Key::Up),
               TBO_KEYCODE(VK_RIGHT, Key::Right),
               TBO_KEYCODE(VK_DOWN, Key::Down),

               // General
               TBO_KEYCODE(VK_SPACE, Key::Space),
               TBO_KEYCODE(VK_RETURN, Key::Enter),
               TBO_KEYCODE(VK_ESCAPE, Key::Escape),
               TBO_KEYCODE(VK_TAB, Key::Tab)
            };

            if (s_KeyCodes.find(win32Code) == s_KeyCodes.end())
                return win32Code; // Cannot cover all the different letters etc. so just return a number

            return s_KeyCodes.at(win32Code);
        }
    }

}
