#pragma once

// Keyboard state, sampled once per game frame (call Input::Update() first thing each tick),
// so every power sees the same "just pressed" edges during that frame.
namespace Input {

void Update();
bool IsDown(int vk);
bool JustPressed(int vk);
bool JustReleased(int vk);

} // namespace Input
