#include "GameMap.h"
#include "Place.h"
#include "Path.h"
#include "AdventureError.h"
#include "GameView.h"
#include "Point.h"
#include <queue>
#include <algorithm>
#include <map>
#include <sstream>
#include <utility>
#include <cmath>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

using namespace std;

// add method add duplicate, reverse path (search through paths)
void GameMap::addPlace(Place* p)
{
	if(find(places.begin(), places.end(), p) == places.end())
	{
		// not found in list, add
		places.push_back(p);
	}
}

void GameMap::addPath(Path* p)
{
	paths.insert(p);
}

string GameMap::toString()
{
	if(places.size() == 0)
		throw AdventureError("NO MAP. The player hasn't moved yet.");

	stringstream ss;
	// loop through places
	list<Place*>::iterator i = places.begin();

	while(i != places.end())
	{
		ss << "  " << (*i)->getName() << "\n";
		// loop through paths
		ss << (*i)->getMapFormattedPaths(paths);
		++i;
	}
	ss << "\n";
	return ss.str();
}

string GameMap::ShortestPathBFS(Place* start, Place* dest)
{

	queue<Place*> bfs;
	map<Place*,int> distMap;
	//populate distMap with -1
	for(list<Place*>::iterator i = places.begin(); i != places.end(); ++i)
	{
		distMap[*i] = -1;
	}

    distMap[dest] = 0;
    bfs.push(dest);
    for(Place* i = bfs.front(); (!(bfs.empty())) && (i != start); i = bfs.front()) 
	{
		for(set<Path*>::iterator iter = paths.begin(); iter!= paths.end(); iter++) 
		{
			try
			{
				// If not explored
				// TODO: fix, infinite loop
				Place* p = i->getDestination((*iter)->getName());
				if(distMap.find(p) != distMap.end())
				{
					if(distMap[p] == -1 || (distMap[p] > (distMap[i]+1))) {
							distMap[p] = distMap[i] + 1;
							bfs.push(p);
					}
				}
			} catch(AdventureError&) {}
		}
		bfs.pop();
    }

	// format the shortest run path
	stringstream ss;
	Place* current = start;
	while(current != dest)
	{
		// loop through paths on destination
		for(set<Path*>::iterator iter = paths.begin(); iter!= paths.end(); iter++) 
		{
			try
			{
				// If not explored
				Place* p = current->getDestination((*iter)->getName());
				if(distMap.find(p) != distMap.end())
				{
					if(distMap[p] == distMap[current]-1)
					{
						// Add to good route
						ss << "  " << (*iter)->getName() << " ---> " << p->getName() << "\n";
						// set current, reset for loop
						current = p;
					}
				}
			} catch(AdventureError&) {}
		}
	}
    return ss.str();
}

Place* GameMap::getMapPlace(string in)
{
	if (places.size() > 0 )
	{
		for(set<Path*>::iterator iter = paths.begin(); iter!= paths.end(); iter++)
		{
			if(StrCmp(in, (*iter)->getDestination()->getName()))
				return (*iter)->getDestination();
		}
	}
	throw AdventureError("Sorry, I am unable to get there.");
	return NULL;
}

string GameMap::toStoreString()
{
	stringstream ss;
	for(list<Place*>::iterator i = places.begin(); i != places.end(); ++i)
	{
		ss << (*i)->getName() << "\n";
	}
	ss << "---paths---\n";
	for(set<Path*>::iterator i = paths.begin(); i != paths.end(); ++i)
	{
		ss << (*i)->getName() << ", " << (*i)->getDestination()->getName() << "\n";
	}
	return ss.str();
}


// Philip Bjorge
// Originally, I implemented the Fruchterman and Reingold algorithm
// This provided unsatisfying results and it was extremely convoluted (in my opinion)
// So, I rewrote it with one-sided springs, while maintaining F&R's force equations
// http://www.cs.brown.edu/~rt/gdhandbook/chapters/force-directed.pdf
void GameMap::spring(Place* current)
{
	if(places.size() != 0)
	{
		srand((unsigned)time(0));

		int w = 700;
		int h = 750;
		sf::RenderWindow App(sf::VideoMode(800,800), "Map", sf::Style::Close, sf::WindowSettings::WindowSettings(24,8,4));

		vector<Place*> drawNodes(places.begin(), places.end());
		map<Place*, vector<Place*>> tempEdges;
		map<Place*, Point> dXY;
		map<Place*, Point> XY;
		map<pair<Place*,Place*>, string> tempEdgesNames;

		for(list<Place*>::iterator i = places.begin(); i != places.end(); ++i)
		{
			vector<Path*> p;
			(*i)->getPaths(p);
			for(vector<Path*>::iterator j = p.begin(); j != p.end(); ++j)
			{
				// unexplored path
				if(paths.find(*j) == paths.end())
				{
					Place* newP = new Place("","");

					tempEdgesNames[make_pair(*i,newP)] = (*j)->getName();
					tempEdges[(*i)].push_back(newP); // temporary edges
					drawNodes.push_back(newP); // Remember to destroy these newP
				}
			}
		}

		// Populate map
		for(vector<Place*>::iterator i = drawNodes.begin(); i != drawNodes.end(); ++i)
		{
			dXY[*i] = Point(0,0);
			//for(int j = 0; j <= (rand()%9); j++){ rand(); }
			XY[*i] = Point(w/2-300+(rand()%600),h/2-200+(rand()%400));
		}


		int area = w*h;
		int k = sqrt(double(area/drawNodes.size()));


		for(int iteration = 150; iteration > 0; iteration--)
		{
			// 4 was empirically one of the better constants
			// I found that randomizing the order I applied the forces improved the results
			if(iteration%4 == 0)
				random_shuffle(drawNodes.begin(), drawNodes.end());
			for(vector<Place*>::iterator node = drawNodes.begin(); node != drawNodes.end(); ++node)
			{
				for(vector<Place*>::iterator other = drawNodes.begin(); other != drawNodes.end(); ++other)
				{
					if((*node) != (*other))
					{
						Point d = XY[*other] - XY[*node];

						double magnitude = d.getMagnitude();
						Point force = Point(0,0);

						// Repulsion
						//force = force - ((d*(1/magnitude)) * .2 *(k*k)/magnitude);
						// Spring
						bool isConnected = false;

						// node = ???, other = something
						if(tempEdges.find(*other) != tempEdges.end())
						{
							vector<Place*> t = tempEdges[(*other)];
							for(vector<Place*>::iterator k = t.begin(); k != t.end(); ++k)
							{
								if(*k == *node)
									isConnected = true;
							}
						}

						// node = something, other = ???
						if(!isConnected)
						{
							if(tempEdges.find(*node) != tempEdges.end())
							{
								vector<Place*> t = tempEdges[(*node)];
								for(vector<Place*>::iterator k = t.begin(); k != t.end(); ++k)
								{
									if(*k == *other)
										isConnected = true;
								}
							}
						}

						// node = something, other = something
						if(!isConnected)
						{
							vector<Path*> p;
							(*other)->getPaths(p);
							for(vector<Path*>::iterator j = p.begin(); j != p.end(); ++j)
							{
								if((*j)->getDestination() == *node)
									isConnected = true;
							}
						}

						// node = something, other = something
						if(!isConnected)
						{
							vector<Path*> p;
							(*node)->getPaths(p);
							for(vector<Path*>::iterator j = p.begin(); j != p.end(); ++j)
							{
								if((*j)->getDestination() == *other)
									isConnected = true;
							}
						}

						Point seg = XY[*other] - XY[*node];
						
						// Bug fix
						// When no repelling forces are nearby, nodes will spring on top of eachother
						// Manual repositioning in the last iteration
						if( (seg.getMagnitude() < 10) && iteration <= 1)
						{
							force.setXY(0,0);
							force = force - ((d*(1/magnitude)) * .001 *(k*k)/magnitude);
						}
						else
						{
							// .32 and .25 are arbitary constants that I found work well
							if(isConnected)
							{
								// exerts a little more pull on the question marks nodes, which by default have only one connection
								if( (*node)->getName() == "" && (*node)->getDescription() == "")
									force = force + ((d*(1/magnitude)) * .35 * (magnitude*magnitude/k));
								else
									force = force + ((d*(1/magnitude)) * .25 * (magnitude*magnitude/k));
							}
							else
							{
								// repulsion
								force = force - ((d*(1/magnitude)) * .25 *(k*k)/magnitude);
							}
						}

						dXY[*node] = dXY[*node] + (force*( ceil( (iteration/12.) )/25)); // dampen?
					}
				}
				XY[*node] = XY[*node] + dXY[*node];
				dXY[*node] = Point(0,0);

				// Keep in drawing bounds
				double cX = min((double)w, max(50., XY[*node].getX()));
				double cY = min((double)h, max(25., XY[*node].getY()));

				Point p = XY[*node];
				p.setXY(cX,cY);
				XY[*node] = p;
			}

			App.Clear();

			vector<pair<Point, Point>> lineSegments;

			for(list<Place*>::iterator i = places.begin(); i != places.end(); ++i)
			{
				if(tempEdges.find((*i)) != tempEdges.end())
				{
					// draw the temp edges in the vector
					vector<Place*> t = tempEdges[(*i)];
					for(vector<Place*>::iterator k = t.begin(); k != t.end(); ++k)
					{

						sf::String Text(tempEdgesNames[make_pair(*i, *k)]);
						Text.Scale(.5f,.5f);
						Text.SetStyle(sf::String::Bold);
						Text.SetColor(sf::Color::Black);

						sf::FloatRect r = Text.GetRect();
						double offX = r.GetWidth()/2;
						double offY = r.GetHeight()/2;
						double newX = ((XY[(*i)].getX()+XY[(*k)].getX())/2)-offX;
						double newY = ((XY[(*i)].getY()+XY[(*k)].getY())/2)-offY;
						Text.Move(newX, newY);

						r = Text.GetRect();


						lineSegments.push_back(make_pair(Point(XY[(*i)].getX(),XY[(*i)].getY()), Point(XY[(*k)].getX(),XY[(*k)].getY())));

						App.Draw(sf::Shape::Line(XY[(*i)].getX(),XY[(*i)].getY(), XY[(*k)].getX(),XY[(*k)].getY(), 2, sf::Color::Yellow));
						App.Draw(sf::Shape::Rectangle( (newX-3), (newY-3), (newX+r.GetWidth()+3), (newY+r.GetHeight()+3), sf::Color::Yellow));
						App.Draw(Text);
					}
				}

				// Draw the other edges
				vector<Path*> p;
				(*i)->getPaths(p);
				for(vector<Path*>::iterator j = p.begin(); j != p.end(); ++j)
				{
					// Edge between visited
					if(find(drawNodes.begin(), drawNodes.end(), (*j)->getDestination()) != drawNodes.end())
					{
						sf::String Text((*j)->getName());

						// Check reverse path to see if it has a different name
						vector<Path*> jOutPaths;
						(*j)->getDestination()->getPaths(jOutPaths);
						for(vector<Path*>::iterator jPath = jOutPaths.begin(); jPath != jOutPaths.end(); ++jPath)
						{
							if( (*jPath)->getDestination() == *i)
								if( (*j)->getName() != (*jPath)->getName() )
									Text.SetText( sf::Unicode::Text( (*j)->getName() + "\n" + (*jPath)->getName() ) );
						}
						

						Text.Scale(.5f,.5f);
						Text.SetStyle(sf::String::Bold);
						Text.SetColor(sf::Color::Black);

						sf::FloatRect r = Text.GetRect();
						double offX = r.GetWidth()/2;
						double offY = r.GetHeight()/2;
						double newX = ((XY[(*i)].getX()+XY[(*j)->getDestination()].getX())/2)-offX;
						double newY = ((XY[(*i)].getY()+XY[(*j)->getDestination()].getY())/2)-offY;
						Text.Move(newX, newY);

						r = Text.GetRect();

						lineSegments.push_back(make_pair(Point(XY[(*i)].getX(),XY[(*i)].getY()), Point(XY[(*j)->getDestination()].getX(),XY[(*j)->getDestination()].getY())));
						App.Draw(sf::Shape::Line(XY[(*i)].getX(),XY[(*i)].getY(), XY[(*j)->getDestination()].getX(),XY[(*j)->getDestination()].getY(), 2, sf::Color::Yellow));
						App.Draw(sf::Shape::Rectangle( (newX-3), (newY-3), (newX+r.GetWidth()+3), (newY+r.GetHeight()+3), sf::Color::Yellow));
						App.Draw(Text);
					}
				}


			for(map<Place*, Point>::iterator i = XY.begin(); i != XY.end(); ++i)
			{
				sf::String Text("?????");
				if(find(places.begin(),places.end(), (*i).first) != places.end())
					Text.SetText((*i).first->getName());
				Text.Scale(.5f,.5f);
				Text.SetStyle(sf::String::Bold);
				Text.SetColor(sf::Color::Black);
				sf::FloatRect r = Text.GetRect();
				double offX = r.GetWidth()/2;
				double offY = r.GetHeight()/2;
				double newX = (*i).second.getX()-offX;
				double newY = (*i).second.getY()-offY;
				Text.Move(newX, newY);
				r = Text.GetRect();
				if((*i).first == current)
					App.Draw(sf::Shape::Rectangle( (newX-3), (newY-3), (newX+r.GetWidth()+3), (newY+r.GetHeight()+3), sf::Color::Green));
				else
					App.Draw(sf::Shape::Rectangle( (newX-3), (newY-3), (newX+r.GetWidth()+3), (newY+r.GetHeight()+3), sf::Color::Red));

				App.Draw(Text);
			}


				// On last iteration, check for line intersections
				// If yes, reset the map locations and start loop over
				if(iteration == 80 && linesIntersect(lineSegments))
				{
					// Randomize map
					for(vector<Place*>::iterator i = drawNodes.begin(); i != drawNodes.end(); ++i)
					{
						dXY[*i] = Point(0,0);
						//for(int j = 0; j <= (rand()%9); j++){ rand(); }
						XY[*i] = Point(275+(rand()%150),275+(rand()%150));
					}

					iteration = 150;
				}

			}

			App.Display();
		}

		sf::Event e;
		while(App.IsOpened())
		{
			while(App.GetEvent(e))
			{
				if(e.Type == sf::Event::Closed)
				{
					// Destory dummy drawNodes
					for(vector<Place*>::iterator n = drawNodes.begin(); n != drawNodes.end(); ++n)
					{
						if( (*n)->getName() == "" && (*n)->getDescription() == "")
							delete *n;
					}
					App.Close();
					break; // fix a bug where you click close multiple times
				}
			}
		}
	}
}


// Found this bad-boy from stackoverflow
// 1:1 Copy
// Original algorithm in http://www.amazon.com/dp/0672323699/
bool GameMap::linesIntersect(vector<pair<Point, Point>> s)
{
	for(vector<pair<Point, Point>>::iterator i = s.begin(); i != s.end(); ++i)
	{
		for(vector<pair<Point, Point>>::iterator j = s.begin(); j != s.end(); ++j)
		{
			// If not a path from the same vertex (most of the time)
			if( !((*j).first == (*i).first || (*j).first == (*i).second || (*j).second == (*i).first || (*j).second == (*i).second))
			{
			    float s1_x, s1_y, s2_x, s2_y;
				s1_x = (*i).second.getX() - (*i).first.getX();
				s1_y = (*i).second.getY() - (*i).first.getY();
				s2_x = (*j).second.getX() - (*j).first.getX();
				s2_y = (*j).second.getY() - (*j).first.getY();

				float s = (-s1_y * ((*i).first.getX() - (*j).first.getX()) + s1_x * ((*i).first.getY() - (*j).first.getY())) / (-s2_x * s1_y + s1_x * s2_y);
				float t = ( s2_x * ((*i).first.getY() - (*j).first.getY()) - s2_y * ((*i).first.getX() - (*j).first.getX())) / (-s2_x * s1_y + s1_x * s2_y);
				
				if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
				{
					return true;
				}
			}
		}
	}
	return false;
}