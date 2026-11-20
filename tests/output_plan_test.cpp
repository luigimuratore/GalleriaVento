#include "../firmware/vento8266/output_plan.h"
#include <cassert>
int main() {
  VentoState s;
  auto p = makeOutputPlan(s, false);
  assert(!p.fans && !p.pump && !p.heater && !p.compressor && !p.upper && !p.lower && !p.led);
  assert(s.command("fans_on", 0));
  p = makeOutputPlan(s, true); assert(p.fans && p.led && !p.pump && !p.heater);
  assert(s.command("smoke_on", 1));
  p = makeOutputPlan(s, true); assert(p.pump && !p.heater);
  assert(s.command("actuation_on", 2));
  p = makeOutputPlan(s, false); assert(p.compressor && !p.upper && !p.lower);
  assert(s.command("upper", 3));
  p = makeOutputPlan(s, false); assert(p.upper && !p.lower && !p.heater);
  assert(s.command("both", 4));
  p = makeOutputPlan(s, false); assert(p.upper && p.lower);
  assert(s.command("lower", 4));
  p = makeOutputPlan(s, false); assert(!p.upper && p.lower);
  assert(s.command("actuation_off", 5));
  assert(s.command("smoke_off", 6));
  p = makeOutputPlan(s, false); assert(p.pump && !p.heater && !p.upper && !p.lower && !p.compressor);
  s.tick(15006); p = makeOutputPlan(s, false); assert(!p.pump && p.fans);
  s.reset(); p = makeOutputPlan(s, false); assert(!p.fans && !p.pump && !p.heater);
}
