- Feature Name: Outfit Types
- Affected audience: Content Creators
- RFC PR: [#57](https://github.com/EndlessSkyCommunity/endless-sky/pull/57)

# Summary

Add the ability to specify a `type` to an outfit. This is used to group different outfits together and can be used by missions for query/removal.

```
outfit "Hyperdrive"
	type "Hyperdrive"
outfit "Scram Drive"
	type "Hyperdrive"
```

# Motivation

Something very often requested is the ability to have species-specific hyperdrives. This RFC solves this problem using `type` on outfits. This type could then be used by missions to check/remove a outfit that matches the given type, which IIUC is the only blocker for species-specific hyperdrives.

More generally, `type` enables us to group outfits together which is useful for other outfits than hyperdrives too.

# Detailed Design

Adds an option node to outfits specifying a type like so:

```
outfit "Hyperdrive"
	type "Hyperdrive"
```

Now the outfit `Hyperdrive` belongs to the type (or is a) `Hyperdrive`. The type name can be anything, it has no relation at all with the outfit. This type is also not shown in the UI, it's for internal use only.

Missions have their `outfit` node expanded to allow an optional `type` like this:

```
on offer
	outfit type Hyperdrive -1
```

which will remove the first outfit that has a `type` equal to `Hyperdrive`.


# Drawbacks

Any outfit having a `type <value>` attribute will now be an error, although it is possible to mitigate this if wanted. I can't really see any drawback, but I'm probably wrong.

# Alternatives

The obvious alternative to this RFC is outfit variants: The ability to specify a base outfit whose attributes are inherited.

Outfit variants aren't a nice solution to solving the motivation mentioned above. Namely, having missions be able to remove any outfit from a set of outfits. For outfit variants, this set would be the base model and any other variants. This means that species specific hyperdrives would all need to inherit from the base Hyperdrive model, while overwriting all of its attributes. It's hardly a variant at that point and more like a completely separate outfit.

Additionally, what if there is no base model? All species specific hyperdrives inheriting from the Hyperdrive make it seem like they are all related in some way, which isn't true: The Hyperdrive is the human model for hyperspace travel. Makes no sense why the Korath Hyperdrive would be based on the Human Hyperdrive.

# Unresolved Questions
> Are there parts of this RFC that still need work before it can be considered complete?

Not really, this RFC is not very complicated.

> Identify related issues/concerns that pertain to this RFC, but do not depend on this particular RFC in order to be addressed

Outfit variants are related, but they are orthogonal to this RFC (they solve a different class of problems).
