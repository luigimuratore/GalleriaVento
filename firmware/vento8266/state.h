#pragma once
#include <stdint.h>
#include <string.h>
struct VentoState {
  bool fans = false, smoke = false, actuation = false, purging = false;
  enum Direction { NEUTRAL, UPPER, LOWER } direction = NEUTRAL;
  uint32_t heatStart = 0, purgeStart = 0;
  static constexpr uint32_t HEAT_MS = 40000, PURGE_MS = 3000;
  void reset() { fans = smoke = actuation = purging = false; direction = NEUTRAL; }
  void startPurge(uint32_t now) { smoke = actuation = false; direction = NEUTRAL; purging = true; purgeStart = now; }
  void tick(uint32_t now) {
    if (smoke && uint32_t(now - heatStart) >= HEAT_MS) startPurge(now);
    if (purging && uint32_t(now - purgeStart) >= PURGE_MS) purging = false;
  }
  bool command(const char* c, uint32_t now) {
    tick(now);
    if (!strcmp(c,"reset")) { reset(); return true; }
    if (!strcmp(c,"fans_on") && !fans) { fans = true; return true; }
    if (!strcmp(c,"fans_off") && fans && !smoke && !actuation) { fans = false; return true; }
    if (!strcmp(c,"smoke_on") && fans && !smoke && !purging) { smoke = true; heatStart = now; return true; }
    if (!strcmp(c,"smoke_off") && smoke && !actuation) { startPurge(now); return true; }
    if (!strcmp(c,"actuation_on") && smoke && !actuation) { actuation = true; direction = NEUTRAL; return true; }
    if (!strcmp(c,"actuation_off") && actuation) { actuation = false; direction = NEUTRAL; return true; }
    if (actuation && !strcmp(c,"upper")) { direction = UPPER; return true; }
    if (actuation && !strcmp(c,"lower")) { direction = LOWER; return true; }
    if (actuation && !strcmp(c,"neutral")) { direction = NEUTRAL; return true; }
    return false;
  }
};
