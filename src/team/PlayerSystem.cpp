// =====================================================================
//  OWNER: Person 2.  EMPTY STUBS - replace with the real logic.
//
//  Useful pieces from Person 1:
//    Input::mouseX(), Input::mouseY(), Input::mouseClicked(),
//    Input::keyPressed(GLFW_KEY_W)               (engine/Input.h)
//    Layout::laneAt(y), Layout::laneBottom(lane)  (shared/Sizes.h)
//    world.selectedCard, cardCost(world.selectedCard), world.coins.spend(cost)
//    Audio::play(Sfx::LaneSelect), Audio::play(Sfx::Place)   (engine/Audio.h)
//    Entity (shared/Entity.h): make MeleePlayer, RangedPlayer, DefensePlayer from it
//    When a player dies: Audio::play(Sfx::Death); player->removeMe = true;
// =====================================================================
#include "team/PlayerSystem.h"

void PlayerSystem::reset(World&) {}
void PlayerSystem::handleInput(World&) {}
void PlayerSystem::update(World&, float) {}
void PlayerSystem::drawUnderCharacters(const World&) {}
void PlayerSystem::drawOverCharacters(const World&) {}
