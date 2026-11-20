#include "../firmware/vento8266/state.h"
#include <cassert>
#include <cstdint>
int main() {
 VentoState s;
 assert(!s.command("smoke_on",1)); assert(!s.command("upper",1));
 assert(s.command("fans_on",10)); assert(s.command("smoke_on",20));
 assert(s.command("actuation_on",30)); assert(s.command("upper",40));
 assert(s.command("lower",50)); assert(s.direction == VentoState::LOWER);
 assert(!s.command("fans_off",60));
 assert(s.command("actuation_off",70)); assert(s.direction == VentoState::NEUTRAL);
 assert(s.command("actuation_on",80));
 s.tick(5019); assert(!s.coilRequested(5019));
 s.tick(5020); assert(s.coilRequested(5020));
 s.tick(45019); assert(s.smoke && s.actuation);
 s.tick(45020); assert(s.smoke && s.actuation && s.coilRequested(45020));
 s.tick(105020); assert(s.smoke && s.actuation && s.coilRequested(105020));
 assert(s.command("smoke_off",105021)); assert(s.purging && !s.smoke && !s.actuation);
 assert(s.command("fans_off",105022)); assert(s.purging && !s.fans);
 assert(s.command("fans_on",105023)); assert(!s.command("smoke_on",105024));
 s.tick(120020); assert(s.purging);
 s.tick(120021); assert(!s.purging && s.fans); assert(s.command("fans_off",120022));
 assert(s.command("fans_on",100)); assert(s.command("smoke_on",UINT32_MAX-100));
 s.tick(44900); assert(s.smoke && s.coilRequested(44900));
 s.reset(); assert(!s.fans && !s.purging && !s.coilReady);
 s.preheatMs=125000; s.purgeMs=240000;
 assert(s.command("fans_on",0)); assert(s.command("smoke_on",1));
 s.tick(125000); assert(!s.coilRequested(125000));
 s.tick(125001); assert(s.coilRequested(125001));
 assert(s.command("smoke_off",125002)); s.tick(365001); assert(s.purging);
 s.tick(365002); assert(!s.purging);
}
