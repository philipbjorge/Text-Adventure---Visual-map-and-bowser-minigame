#include "BowserFoeGrav.h"
#include <algorithm>
using namespace std;

// Philip Bjorge
// The Whole Class
// A bit specialized for this game.

BowserFoeGrav::BowserFoeGrav(double s, double a, double mSpeed, double f,
	double yV, double maxYV, double grav, double x, double y, double groundedY)
{
	speed = s;
	accel = a;
	maxSpeed = mSpeed;
	friction = f;
	posX = x;
	posY = y;

	yVel = yV;
	maxYVel = maxYV;
	gravity = grav;
	groundY = groundedY;
}

void BowserFoeGrav::move(bool moving, bool isLeft, float frameTime)
{
	if(moving)
	{
		if(isLeft)
			speed -= accel * frameTime;
		else
			speed += accel * frameTime;
	}
	else
	{
		if(speed > 0)
		{
			speed -= friction * frameTime;
			speed = max(speed,0.);
		}
		else
		{
			speed += friction * frameTime;
			speed = min(speed, 0.);
		}
	}

	speed = max(-maxSpeed, speed);
	speed = min(maxSpeed, speed);
}

void BowserFoeGrav::jump(float frameTime)
{
	if(isGrounded())
	{
		// adjust ySpeed
		yVel = maxYVel;
		posY--;
		// move 1 pixel off ground
	}
}

bool BowserFoeGrav::isGrounded()
{
	// second part is mario specific (hammers+bowser don't go up on platform)
	return ((posX >= 0. && posX <= 680. && posY >= groundY) || (posX > 680. && posY >= 288));
}

void BowserFoeGrav::update(float frameTime)
{
	if(!isGrounded())
	{
		// update posY (gravity)
		yVel -= gravity*frameTime;
		posY -= yVel;
	}

	// Super basic collision detection with pillar
		if( (posX+speed) > 680 && posY <= 289)
		{
			posX += speed;
			posX = min(776., posX);
		}
		else
		{
			posX += speed;
			posX = max(0.,posX);
			posX = min(680., posX);
			if( (posX == 680 && posY >= 289) || (posX == 0)) // stops velocity on wall hit
				speed = 0;
		}
	// update posX (speed)
}

double BowserFoeGrav::getX()
{
	return posX;
}

int BowserFoeGrav::getY()
{
	return posY;
}