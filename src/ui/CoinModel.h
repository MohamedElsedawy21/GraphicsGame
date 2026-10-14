#pragma once

// The collectible: an Egyptian 1-pound coin (brass ring, silver center,
// a pharaoh's-mask emblem). (cx, cy) = center, radius in pixels.
// spinTime: pass a growing time to make the coin turn; pass 0 for a flat coin.
void drawCoin(float cx, float cy, float radius, float spinTime = 0.0f);
