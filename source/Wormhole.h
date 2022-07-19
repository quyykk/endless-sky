/* Wormhole.h
Copyright (c) 2021 quyykk

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef WORMHOLE_H_
#define WORMHOLE_H_

#include <string>
#include <unordered_map>

class DataNode;
class Planet;
class System;



// Class representing a wormhole in a planet.
class Wormhole {
public:
	// Load a wormhole's description from a file.
	void Load(const DataNode &node);
	// Load a wormhole from a given planet.
	void LoadFromPlanet(const Planet &planet);
	// Check if this wormhole has been defined.
	bool IsValid() const;

	// Returns the planet corresponding to this wormhole.
	const Planet *GetPlanet() const;
	// Returns this wormhole's name.
	const std::string &Name() const;
	// Whether this wormhole's link appears on the map.
	bool IsMappable() const;
	// Whether this wormhole was auto-generated. This is used to
	// update this wormhole in game events that update the corresponding
	// planet.
	bool IsAutogenerated() const;

	const System &WormholeSource(const System &to) const;
	const System &WormholeDestination(const System &from) const;
	const std::unordered_map<const System *, const System *> &Links() const;

	void SetPlanet(const Planet &planet);


private:
	// Generate any links from the planet.
	void GenerateLinks();


private:
	bool isDefined = false;
	bool isAutogenerated = false;

	const Planet *planet = nullptr;
	std::string name = "???";
	bool mappable = false;
	std::unordered_map<const System *, const System *> links;
};



#endif
