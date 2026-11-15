#include "../firmware/vento8266/state.h"
#include <cassert>
#include <cstdint>
int main() {
 VentoState s;
 assert(!s.command("smoke_on",1)); assert(!s.command("upper",1));
 assert(s.command("fans_on",10)); assert(s.command("smoke_on",20));
 assert(s.command("actuation_on",30)); assert(s.command("upper",40));
 assert(s.command("lower",50)); assert(s.direction == VentoState::LOWER);
 assert(!s.command("smoke_off",60)); assert(!s.command("fans_off",60));
 assert(s.command("actuation_off",70)); assert(s.direction == VentoState::NEUTRAL);
 assert(s.command("actuation_on",80));
 s.tick(40019); assert(s.smoke && s.actuation);
 s.tick(40020); assert(!s.smoke && !s.actuation && s.purging && s.fans);
 assert(!s.command("fans_off",40021)); s.tick(43019); assert(s.purging);
 s.tick(43020); assert(!s.purging && s.fans); assert(s.command("fans_off",43021));
 assert(s.command("fans_on",100)); assert(s.command("smoke_on",UINT32_MAX-100));
 s.tick(39900); assert(s.purging); s.reset(); assert(!s.fans && !s.purging);
}
