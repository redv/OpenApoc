#pragma once

#include "framework/logger.h"
#include "game/state/gametime.h"
#include "game/state/stateobject.h"
#include "game/state/tilemap/tile.h"
#include "game/state/tilemap/tileobject.h"
#include "library/colour.h"
#include "library/rect.h"
#include "library/sp.h"
#include <map>
#include <set>
#include <vector>

#define VELOCITY_SCALE_CITY (Vec3<float>{32, 32, 16})
#define VELOCITY_SCALE_BATTLE (Vec3<float>{24, 24, 20})

namespace OpenApoc
{

static const Colour COLOUR_BLACK = {0, 0, 0, 255};

// FIXME: Alexey Andronov: Does anyone know why we divide by 4 here?
static const unsigned TICK_SCALE = TICKS_PER_SECOND / 4;

class Image;
class TileMap;
class Collision;
class VoxelMap;
class Renderer;
class TileView;
class Projectile;
class TileObjectProjectile;
class Vehicle;
class TileObjectVehicle;
class Scenery;
class TileObjectScenery;
class Doodad;
class TileObjectDoodad;
class BattleMapPart;
class TileObjectBattleMapPart;
class BattleUnit;
class TileObjectBattleUnit;
class BattleItem;
class TileObjectBattleItem;
class BattleHazard;
class TileObjectBattleHazard;
class Sample;
class Organisation;
class Surface;

class TileTransform
{
  public:
	virtual Vec2<float> tileToScreenCoords(Vec3<float> coords) const = 0;
	virtual Vec3<float> screenToTileCoords(Vec2<float> screenPos, float z) const = 0;
};

enum class TileViewMode
{
	Isometric,
	Strategy,
};

enum class MapDirection
{
	North = 0,
	East = 1,
	South = 2,
	West = 3,
	Up = 4,
	Down = 5
};

class TileMap
{
  private:
	std::vector<Tile> tiles;
	std::vector<std::set<TileObject::Type>> layerMap;
	// Cache for the strategy view mode.
	std::vector<sp<Surface>> strategyViewCache;

  public:
	const Tile *getTile(int x, int y, int z) const
	{

		if (!((x >= 0) && (x < size.x) && (y >= 0) && (y < size.y) && (z >= 0) && (z < size.z)))
		{
			LogError("Incorrect tile coordinates (const) %d,%d,%d", x, y, z);
			return nullptr;
		}
		return &this->tiles[z * size.x * size.y + y * size.x + x];
	}
	Tile *getTile(int x, int y, int z)
	{
		if (!((x >= 0) && (x < size.x) && (y >= 0) && (y < size.y) && (z >= 0) && (z < size.z)))
		{
			LogError("Incorrect tile coordinates %d,%d,%d", x, y, z);
			return nullptr;
		}
		return &this->tiles[z * size.x * size.y + y * size.x + x];
	}
	Tile *getTile(Vec3<int> pos) { return this->getTile(pos.x, pos.y, pos.z); }
	const Tile *getTile(Vec3<int> pos) const { return this->getTile(pos.x, pos.y, pos.z); }
	// Returns the tile this point is 'within'
	Tile *getTile(Vec3<float> pos)
	{
		return this->getTile(static_cast<int>(pos.x), static_cast<int>(pos.y),
		                     static_cast<int>(pos.z));
	}
	const Tile *getTile(Vec3<float> pos) const
	{
		return this->getTile(static_cast<int>(pos.x), static_cast<int>(pos.y),
		                     static_cast<int>(pos.z));
	}
	Vec3<int> size;
	Vec3<int> voxelMapSize;
	Vec3<float> velocityScale;
	bool ceaseUpdates = false;

	TileMap(Vec3<int> size, Vec3<float> velocityScale, Vec3<int> voxelMapSize,
	        std::vector<std::set<TileObject::Type>> layerMap);
	~TileMap();

	// Path to target area (bounds are exclusive)
	std::list<Vec3<int>> findShortestPath(Vec3<int> origin, Vec3<int> destinationStart,
	                                      Vec3<int> destinationEnd, int iterationLimit,
	                                      const CanEnterTileHelper &canEnterTile,
	                                      bool approachOnly = false, bool ignoreStaticUnits = false,
	                                      bool ignoreMovingUnits = true,
	                                      bool ignoreAllUnits = false, float *cost = nullptr,
	                                      float maxCost = 0.0f);
	// Path to target position
	std::list<Vec3<int>>
	findShortestPath(Vec3<int> origin, Vec3<int> destination, unsigned int iterationLimit,
	                 const CanEnterTileHelper &canEnterTile, bool approachOnly = false,
	                 bool ignoreStaticUnits = false, bool ignoreMovingUnits = true,
	                 bool ignoreAllUnits = false, float *cost = nullptr, float maxCost = 0.0f)
	{
		return findShortestPath(origin, destination, destination + Vec3<int>{1, 1, 1},
		                        iterationLimit, canEnterTile, approachOnly, ignoreStaticUnits,
		                        ignoreMovingUnits, ignoreAllUnits, cost, maxCost);
	}

	Collision findCollision(Vec3<float> lineSegmentStart, Vec3<float> lineSegmentEnd,
	                        const std::set<TileObject::Type> validTypes = {},
	                        sp<TileObject> ignoredObject = nullptr, bool useLOS = false,
	                        bool check_full_path = false, unsigned maxRange = 0,
	                        bool recordPassedTiles = false,
	                        StateRef<Organisation> ignoreOwnedProjectiles = nullptr) const;

	bool checkThrowTrajectory(const sp<TileObject> thrower, Vec3<float> start, Vec3<int> end,
	                          Vec3<float> targetVectorXY, float velocityXY, float velocityZ) const;

	void addObjectToMap(sp<Projectile>);
	void addObjectToMap(sp<Vehicle>);
	void addObjectToMap(sp<Scenery>);
	void addObjectToMap(sp<Doodad>);
	void addObjectToMap(sp<BattleMapPart>);
	void addObjectToMap(sp<BattleItem>);
	void addObjectToMap(sp<BattleUnit>);
	void addObjectToMap(sp<BattleHazard>);

	unsigned int getLayer(TileObject::Type type) const;
	unsigned int getLayerCount() const;
	bool tileIsValid(int x, int y, int z) const;
	bool tileIsValid(Vec3<int> tile) const;

	sp<Image> dumpVoxelView(const Rect<int> viewRect, const TileTransform &transform, float maxZ,
	                        bool fast = false, bool los = false) const;

	void updateAllBattlescapeInfo();
	void updateAllCityInfo();

	std::map<Vec3<int>, std::list<Vec3<int>>> agentPathCache;
	void clearPathCaches();

	// Operations with the strategy view cache.
	sp<Surface> getViewSurface(size_t i) const;
	void setViewSurface(size_t i, sp<Surface> surface);
	void setViewSurfaceDirty(size_t i, bool dirty = true);
	bool isViewSurfaceDirty(size_t i) const;
};
}; // namespace OpenApoc
