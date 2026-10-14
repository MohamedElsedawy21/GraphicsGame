#pragma once

// Draws a 60 x 8 px health bar. (centerX, y) = the middle of the bar's BOTTOM edge.
// One small section per life, so players can count the lives at a glance.
void drawHealthBar(float centerX, float y, int current, int max);
