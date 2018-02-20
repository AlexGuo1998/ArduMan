#pragma once

typedef struct {
	unsigned long perframetime;
	unsigned long nextframetime;
} counter;

void initcounter(counter *s, unsigned long now, double framerate);
bool nextframe(counter *s, unsigned long now);

