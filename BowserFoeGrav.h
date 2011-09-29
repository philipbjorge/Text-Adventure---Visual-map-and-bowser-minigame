#pragma once
// Philip Bjorge
// The Whole Class
// A bit specialized for this game.

class BowserFoeGrav
{
public:
	BowserFoeGrav(double s, double a, double mSpeed, double f,
		double yV, double maxYV, double grav, double x, double y, double groundedY);
	void move(bool moving, bool isLeft, float frameTime);
	void jump(float frameTime);
	bool isGrounded();
	void update(float frameTime);
	double getX();
	int getY();
private:
	double speed;
	double accel;
	double maxSpeed;
	double friction;

	double yVel;
	double maxYVel;
	double gravity;

	double posX;
	int posY;
	int groundY;
};