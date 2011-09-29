#pragma once
#include "Foe.h"
#include "BowserFoeGrav.h"

// Philip Bjorge
// The Whole Class
// A bit specialized for this game.

#include <iostream>
#include <string>
using namespace std;


class BowserFoe :
	public Foe
{
public:
	BowserFoe();
	bool startFight();
private:
	bool offScreen( BowserFoeGrav* b);
};