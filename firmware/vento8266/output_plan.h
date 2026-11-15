#pragma once
#include "state.h"

// Bozza logica. Non include Arduino e non accede ai pin o ai carichi.
// Indica richieste, non condizioni misurate né comando abilitato.
struct OutputPlan {
  bool fans = false;
  bool pump = false;
  bool heater = false; // Blocco permanente: assenza di consensi fisici verificati.
  bool compressor = false;
  bool upper = false;
  bool lower = false;
  bool led = false;
};

inline OutputPlan makeOutputPlan(const VentoState &s, bool ledRequested) {
  OutputPlan p;
  p.fans = s.fans;
  p.pump = s.smoke || s.purging;
  // Non esiste ancora la lettura dei consensi di flusso e temperatura.
  p.heater = false;
  p.compressor = s.actuation;
  p.upper = s.actuation && s.direction == VentoState::UPPER;
  p.lower = s.actuation && s.direction == VentoState::LOWER;
  p.led = ledRequested;
  return p;
}
