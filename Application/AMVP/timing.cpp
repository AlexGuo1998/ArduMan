#include "timing.h"

void initcounter(counter *s, unsigned long now, double framerate) {
	s->perframetime = (unsigned long)1000000 / framerate;
	s->nextframetime = now + s->perframetime;
}

bool nextframe(counter *s, unsigned long now) {
  //unsigned long now = micros();
  if ((signed long)(now - s->nextframetime) > 0) {
	  s->nextframetime += s->perframetime;
	  return true;
  } else {
	  return false;
  }
}

