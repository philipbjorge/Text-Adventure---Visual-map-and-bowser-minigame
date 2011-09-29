#pragma once
#include "Point.h"
#include <vector>
#include <string>
#include <set>
#include <list>
#include <utility>
#include <map>

using namespace std;

class Place;
class Path;

class GameMap {
public:
	void addPlace(Place* p); // add method add duplicate, reverse path (search through paths)
	void addPath(Path* p);
	string toString();
	string ShortestPathBFS(Place* start, Place* dest);
	Place* getMapPlace(string in);
	string toStoreString();

	// Philip Bjorge
	void spring(Place* current);
private:
	list<Place*> places; // places visited
	set<Path*> paths; //
	bool GameMap::linesIntersect(vector<pair<Point, Point>> s);
};