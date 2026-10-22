#include "core/Input.h"

#include "plugin.h" // plugin::KeyPressed

namespace {
bool g_now[256] = {};
bool g_before[256] = {};

bool Valid(int vk) {
    return vk > 0 && vk < 256;
}
} // namespace

namespace Input {

void Update() {
    for (int vk = 1; vk < 256; ++vk) {
        g_before[vk] = g_now[vk];
        g_now[vk] = plugin::KeyPressed(vk);
    }
}

bool IsDown(int vk) {
    return Valid(vk) && g_now[vk];
}

bool JustPressed(int vk) {
    return Valid(vk) && g_now[vk] && !g_before[vk];
}

bool JustReleased(int vk) {
    return Valid(vk) && !g_now[vk] && g_before[vk];
}

} // namespace Input
