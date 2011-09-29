#include "BowserFoe.h"
#include "BowserFoeGrav.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
using namespace std;

// Philip Bjorge
// The Whole Class
// A bit specialized for this game.

BowserFoe::BowserFoe():
Foe("Super Bowser Boss", "Not your ordinary Foe", 1000, 1000)
{
}

bool BowserFoe::startFight()
{
	srand((unsigned)time(0));

	bool playing = true;
	bool win = false;
	sf::RenderWindow App(sf::VideoMode(800, 600, 32), "Bowser Boss Game");

	sf::Image bg;
	bg.LoadFromFile("bowserBattle.jpg");

	sf::Image marioSprite;
	marioSprite.LoadFromFile("mario.png");
	sf::Image marioJumpSprite;
	marioJumpSprite.LoadFromFile("marioJump.png");
	sf::Image bowserSprite;
	bowserSprite.LoadFromFile("bowser.png");
	sf::Image bulletSprite;
	bulletSprite.LoadFromFile("bullet.png");

	BowserFoeGrav mario = BowserFoeGrav(0,1.2,6.,5., 0., 2.2, 16.,0.,356., 356.);
	BowserFoeGrav bowser = BowserFoeGrav(0, 1.2, 6., 5., 0., .8, 2.4, 520., 334., 334.);

	vector<BowserFoeGrav*> bullets;

	double lastJumpTime = 0;
	double lastThrowTime = 0;
	int nextJump = (rand()%2 + 1);
	int nextThrow = (rand()%2 + 1);

	while(playing)
	{
		// Update input
		sf::Event e;
		App.GetEvent(e);

		// Draw BG
		App.Draw(sf::Sprite(bg));

		//
		// CONTROLS
		//
		// Left/Right
		if(App.GetInput().IsKeyDown(sf::Key::Left))
		{
			mario.move(true, true, App.GetFrameTime());
		}
		else if(App.GetInput().IsKeyDown(sf::Key::Right))
		{
			mario.move(true, false, App.GetFrameTime());
		}
		else
		{
			mario.move(false, false, App.GetFrameTime());
		}
		// Jump
		if(App.GetInput().IsKeyDown(sf::Key::Space) || App.GetInput().IsKeyDown(sf::Key::Up))
		{
			mario.jump(App.GetFrameTime());
		}


		//
		// PLAYER MOVEMENT
		//

		mario.update(App.GetFrameTime());
		if(mario.isGrounded())
			App.Draw( sf::Sprite(marioSprite, sf::Vector2f(mario.getX(), mario.getY())) );
		else
			App.Draw( sf::Sprite(marioJumpSprite, sf::Vector2f(mario.getX(), mario.getY())) );


		//
		// BOWSER MOVEMENT
		// HAMMER MOVEMENT
		//

		// Bowser random Jump
		if( lastJumpTime >= nextJump)
		{
			bowser.jump(App.GetFrameTime());
			nextJump = (rand()%2 + 1);
			lastJumpTime = 0;
		}
		bowser.update(App.GetFrameTime());

		if( lastThrowTime >= nextThrow)
		{
			int bulletNumber = rand()%5 +1;
			for(int i=0; i < bulletNumber; i++)
			{
				// These for loops are run because the random number generator is being called
				// so quickly that its returning very, very similar numbers
				for(int j = 0; j <= (rand()%9); j++){ rand(); }
				double xV = ( (rand()%8) +5)/10.; // 0.5->1.0
				for(int j = 0; j <= (rand()%9); j++){ rand(); }
				double yV = ( (rand()%15) + 20)/20;
				bullets.push_back(new BowserFoeGrav(-xV,.5,1.,5., yV, yV, 5., bowser.getX(),bowser.getY(), 800));
			}
			nextThrow = (rand()%2 + 1);
			lastThrowTime = 0;
		}

		// Bullet Throw
		// Add new gravFoes to bullets vector with random velocities
		// for loop, draw bullets, removing and deleting bullets that move off screen
		vector<BowserFoeGrav*> tempB;
		for(vector<BowserFoeGrav*>::iterator b = bullets.begin(); b != bullets.end(); ++b)
		{
			if((*b)->getY() >= 600)
			{
				delete (*b);
			}
			else
			{
				tempB.push_back(*b);
			}
		}
		bullets = tempB;

		for(int b = 0; b < bullets.size(); ++b)
		{
			bullets[b]->update(App.GetFrameTime());
			App.Draw( sf::Sprite( bulletSprite, sf::Vector2f( bullets[b]->getX(), bullets[b]->getY() ) ) );
		}
		App.Draw( sf::Sprite(bowserSprite, sf::Vector2f(bowser.getX(), bowser.getY())) );

		//
		// COLLISION DETECTION
		// Bowser, Hammers, Button
		//
		sf::Rect<double> marioBounding =  sf::Rect<double>(mario.getX(), mario.getY(), mario.getX()+marioSprite.GetWidth(), mario.getY()+marioSprite.GetHeight());
		sf::Rect<double> bowserBounding = sf::Rect<double>(bowser.getX(), bowser.getY(), bowser.getX()+bowserSprite.GetWidth(), bowser.getY()+bowserSprite.GetHeight());
		
		if(marioBounding.Intersects(bowserBounding))
		{
			// Mario Dead
			win = false;
			playing = false;
			sf::String Text("You Lose");
			Text.SetStyle(sf::String::Bold);
			Text.SetCenter(Text.GetRect().GetWidth()/2, Text.GetRect().GetHeight()/2);
			Text.Move(400, 300);
			App.Draw(Text);
		}

		// Bullet collisions
		for(int b = 0; b < bullets.size(); ++b)
		{
			sf::Rect<double> bulletBounding = sf::Rect<double>(bullets[b]->getX(), bullets[b]->getY(), bullets[b]->getX()+bulletSprite.GetWidth(), bullets[b]->getY()+bulletSprite.GetHeight());
			if(marioBounding.Intersects(bulletBounding))
			{
				// Mario Dead
				win = false;
				playing = false;
				sf::String Text("You Lose");
				Text.SetStyle(sf::String::Bold);
				Text.SetCenter(Text.GetRect().GetWidth()/2, Text.GetRect().GetHeight()/2);
				Text.Move(400, 300);
				App.Draw(Text);
			}
		}

		if(mario.getX() > 680. && mario.isGrounded())
		{
			// Bowser Dead
			win = true;
			playing = false;
			sf::String Text("You Win");
			Text.SetStyle(sf::String::Bold);
			Text.SetCenter(Text.GetRect().GetWidth()/2, Text.GetRect().GetHeight()/2);
			Text.Move(400, 300);
			App.Draw(Text);
		}
			
		lastJumpTime += App.GetFrameTime();
		lastThrowTime += App.GetFrameTime();
		App.Display();
	}
	// Destroy bullets
	for(vector<BowserFoeGrav*>::iterator b = bullets.begin(); b != bullets.end(); ++b)
	{
		delete *b;
	}

	sf::Clock timer;
	while(true)
	{
		if(timer.GetElapsedTime() >= 3)
		{
			App.Close();
			return win;
		}
	}
}