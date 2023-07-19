#include "pitches.h"

int MELODY[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, REST, NOTE_B3, NOTE_C4
};

int MELODY_DURATIONS[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

int MELODY_SIZE = sizeof(MELODY_DURATIONS) / sizeof(int);
