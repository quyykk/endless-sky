/* Visual.cpp
Copyright (c) 2017 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "Visual.h"

#include "Audio.h"
#include "Effect.h"
#include "Random.h"

using namespace std;



// Generate a visual based on the given Effect.
Visual::Visual(const Effect &effect, Point pos, Point vel, Angle facing, Point hitVelocity)
	: Body(effect, pos, vel, facing), lifetime(effect.lifetime)
{
	if(effect.randomLifetime > 0)
		lifetime += Random::Int(effect.randomLifetime + 1);

	angle += Angle::Random(effect.randomAngle) - Angle::Random(effect.randomAngle);
	spin = Angle::Random(effect.randomSpin) - Angle::Random(effect.randomSpin);

	velocity *= effect.velocityScale;
	velocity += hitVelocity * (1. - effect.velocityScale);
	if(effect.randomVelocity)
		velocity += angle.Unit() * Random::Real() * effect.randomVelocity;

	if(effect.sound)
		Audio::Play(effect.sound, position);

	if(effect.randomFrameRate)
		AddFrameRate(effect.randomFrameRate * Random::Real());

	positionOffset = position;
}



Visual::Visual(const Effect &effect, Point pos, const Body &follow, Point velocityOffset, Angle facingOffset)
	: Visual(effect, follow.Position() + follow.Velocity() + follow.Facing().Rotate(pos), velocityOffset, follow.Facing())
{
	this->follow = &follow;
	positionOffset = pos;
	this->velocityOffset = velocityOffset;
	this->facingOffset = facingOffset;
}



// Step the effect forward.
void Visual::Move()
{
	if(lifetime-- <= 0 || (follow && follow->ShouldBeRemoved()))
		MarkForRemoval();
	else
	{
		if(follow)
		{
			// Match the position + facing of the body that this visual is centered on.
			position = follow->Position() + follow->Facing().Rotate(positionOffset) + velocityOffset;
			angle = follow->Facing() + facingOffset;

			velocityOffset += velocity;
			facingOffset += spin;
		}
		else
		{
			position += velocity;
			angle += spin;
		}
	}
}
