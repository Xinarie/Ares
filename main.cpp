#include <cstdint>
#include <iostream>
#include <Windows.h>
#include "Memory.h"

namespace Offsets {
    constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDEB99C;
    constexpr ::std::ptrdiff_t m_fFlags = 0x104;
    constexpr ::std::ptrdiff_t dwForceJump = 0x52BCD88;
    constexpr ::std::ptrdiff_t dwEntityList = 0x4E0102C;
    constexpr ::std::ptrdiff_t dwGlowObjectManager = 0x535BAD0;
    constexpr ::std::ptrdiff_t m_iTeamNum = 0xF4;
    constexpr ::std::ptrdiff_t m_iGlowIndex = 0x10488;
}

struct alignas(16) Color {
    constexpr Color(const float r, const float g, const float b, const float a = 1.f) noexcept :
            r(r), g(g), b(b), a(a) {}
    float r, g, b, a;
};

int main() {
    const char* targetProcessName = "csgo.exe";
    const char* moduleName = "client.dll";
    Memory memory(targetProcessName);

    uintptr_t moduleBaseAddress = memory.GetModule(moduleName);
    if (moduleBaseAddress != 0) {
        while (true) {
            const auto localPlayer = memory.Read<std::uintptr_t>(moduleBaseAddress + Offsets::dwLocalPlayer);

            const auto localPlayerTeam = memory.Read<std::uintptr_t>(localPlayer + Offsets::m_iTeamNum);
            const auto localPlayerFlags = memory.Read<std::uintptr_t>(localPlayer + Offsets::m_fFlags);

            // Bunny Hopping (Bhop)
            if (GetAsyncKeyState(VK_SPACE)) {
                memory.Write(moduleBaseAddress + Offsets::dwForceJump, (localPlayerFlags & (1 << 0)) ? 6 : 4);
            }

            // GlowESP
            const auto glowObjectManager = memory.Read<std::uintptr_t>(moduleBaseAddress + Offsets::dwGlowObjectManager);

            for (int i = 1; i <= 32; ++i) {
                const auto entity = memory.Read<std::uintptr_t>(moduleBaseAddress + Offsets::dwEntityList + i * 0x10);

                if (!entity)
                    continue;

                if (memory.Read<std::uintptr_t>(entity + Offsets::m_iTeamNum) == localPlayerTeam)
                    continue;

                const auto glowIndex = memory.Read<std::int32_t>(entity + Offsets::m_iGlowIndex);

                memory.Write(glowObjectManager + (glowIndex * 0x38) + 0x8, Color(1.f, 0.f, 1.f, 1.f));

                memory.Write<bool>(glowObjectManager + (glowIndex * 0x38) + 0x28, true);
                memory.Write<bool>(glowObjectManager + (glowIndex * 0x38) + 0x29, false);
            }
        }
    } else {
        std::cerr << "Failed to find module: " << moduleName << std::endl;
    }

    return 0;
}
