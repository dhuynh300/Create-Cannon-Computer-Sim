#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include <fstream>

#define RAD2DEG (180.0 / M_PI)
#define DEG2RAD (M_PI / 180.0)

#define DRAG 0.01 // Depends on projectile, shot.json, "drag": 0.01,
#define GRAVITY -0.05 // Depends on projectile, shot.json, "gravity": -0.05,

// Decompiled and edited Vec3.class from CreateBigCannons mod
class Vec3 {
public:
	Vec3(double inputX, double inputY, double inputZ) {
		this->x = inputX;
		this->y = inputY;
		this->z = inputZ;
	}

	Vec3 normalize() const {
		double d = std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
		return d <= (DBL_EPSILON * 10.0) ? Vec3(0.0, 0.0, 0.0) : Vec3(this->x / d, this->y / d, this->z / d);
	}

	double dot(Vec3 vec) const {
		return this->x * vec.x + this->y * vec.y + this->z * vec.z;
	}

	Vec3 cross(Vec3 vec) const {
		return Vec3(this->y * vec.z - this->z * vec.y, this->z * vec.x - this->x * vec.z, this->x * vec.y - this->y * vec.x);
	}

	Vec3 subtract(Vec3 vec) const {
		return this->subtract(vec.x, vec.y, vec.z);
	}

	Vec3 subtract(double x, double y, double z) const {
		return this->add(-x, -y, -z);
	}

	Vec3 add(Vec3 vec) const {
		return this->add(vec.x, vec.y, vec.z);
	}

	Vec3 add(double x, double y, double z) const {
		return Vec3(this->x + x, this->y + y, this->z + z);
	}

	double distanceTo(Vec3 vec) const {
		double d = vec.x - this->x;
		double e = vec.y - this->y;
		double f = vec.z - this->z;
		return std::sqrt(d * d + e * e + f * f);
	}

	double distanceToSqr(Vec3 vec) const {
		double d = vec.x - this->x;
		double e = vec.y - this->y;
		double f = vec.z - this->z;
		return d * d + e * e + f * f;
	}

	double distanceToSqr(double x, double y, double z) const {
		double d = x - this->x;
		double e = y - this->y;
		double f = z - this->z;
		return d * d + e * e + f * f;
	}

	double distanceToXZ(Vec3 vec) const {
		double d = vec.x - this->x;
		double f = vec.z - this->z;
		return std::sqrt(d * d + f * f);
	}

	Vec3 scale(double factor) const {
		return this->multiply(factor, factor, factor);
	}

	Vec3 reverse() const {
		return this->scale(-1.0);
	}

	Vec3 multiply(Vec3 vec) const {
		return this->multiply(vec.x, vec.y, vec.z);
	}

	Vec3 multiply(double factorX, double factorY, double factorZ) const {
		return Vec3(this->x * factorX, this->y * factorY, this->z * factorZ);
	}

	double length() const {
		return std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
	}

	double lengthSqr() const {
		return this->x * this->x + this->y * this->y + this->z * this->z;
	}

	double lengthXZ() const {
		return std::sqrt(this->x * this->x + this->z * this->z);
	}

public:
	bool equals(Vec3 object) {
		return this->x == object.x && this->y == object.y && this->z == object.z;
	}

public:
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
};

enum class Dimension : int {
	OVERWORLD = 0,
	NETHER = -1,
	END = 1
};

// Overworld (1.0), Nether (1.1), End (0.0)
double dimensionDragMultiplier(Dimension dimension) {
	switch (dimension) {
		case Dimension::OVERWORLD: // Overworld
			return 1.0;
		case Dimension::NETHER: // Nether
			return 1.1;
		case Dimension::END: // End
			return 0.0;
	}
	return 1.0;
}

// Overworld (1.0), Nether (1.1), End (0.8)
double dimensionGravityMultiplier(Dimension dimension) {
	switch (dimension) {
		case Dimension::OVERWORLD: // Overworld
			return 1.0;
		case Dimension::NETHER: // Nether
			return 1.1;
		case Dimension::END: // End
			return 0.8;
	}
	return 1.0;
}

// Simplified from line 362 of AbstractCannonProjectile.java
double getDragForce(const Vec3& deltaMovement) {
	double magnitude = deltaMovement.length();
	double drag = DRAG * dimensionDragMultiplier(Dimension::OVERWORLD) * magnitude;
	return std::min(drag, magnitude);
}

// Simplified from line 357 of AbstractCannonProjectile.java
double getGravity() {
	return GRAVITY * dimensionGravityMultiplier(Dimension::OVERWORLD);
}

// Simplified from line 82 of AbstractCannonProjectile.java
void tick(Vec3& deltaMovement, Vec3& position) {
	Vec3 oldVel = deltaMovement;
	Vec3 oldPos = position;
	//Vec3 accel = oldVel.normalize().scale(-getDragForce(deltaMovement)).add(0.0, getGravity(), 0.0);
	Vec3 accel = oldVel.normalize().scale(-getDragForce(deltaMovement)).add(0.0, getGravity(), 0.0);
	Vec3 newPos = oldPos.add(oldVel).add(accel.scale(0.5));

	position = newPos;
	deltaMovement = oldVel.add(accel);
}

// powder_charge.json "strength": 2
int getChargePower() {
	return 2;
}

// Rewritten from lines 410-433 of AbstractCannonProjectile.java
// Minecraft's "coordinate system"
void FireShot(const Vec3& cannonPivot, Vec3& projVel, Vec3& projPosition, double cannonPitch, double cannonYaw, int barrelLength, int chargesUsed) {
	// Projectile centered position
	projPosition = cannonPivot;

	double radPitch = cannonPitch * DEG2RAD;
	double radYaw = (cannonYaw + 90.0) * DEG2RAD; // +90 degrees because Mojang is special
	double pitchCosResult = std::cos(radPitch);
	double pitchSinResult = std::sin(radPitch);
	double yawCosResult = std::cos(radYaw);
	double yawSinResult = std::sin(radYaw);

	barrelLength += 2; // Can hide behind a wall lol
	projPosition.x += pitchCosResult * yawCosResult * barrelLength;
	projPosition.y += pitchSinResult * barrelLength;
	projPosition.z += pitchCosResult * yawSinResult * barrelLength;

	// Projectile velocity (no spread)
	double projPower = chargesUsed * getChargePower();
	projVel.x = pitchCosResult * yawCosResult * projPower;
	projVel.y = pitchSinResult * projPower;
	projVel.z = pitchCosResult * yawSinResult * projPower;
}

double calculateYaw(const Vec3& cannonPivot, const Vec3& targetPos) {
	Vec3 deltaPos = targetPos.subtract(cannonPivot);
	return std::atan2(-deltaPos.x, deltaPos.z) * RAD2DEG;
}

int main() {
	// Cannon data
	Vec3 cannonPivot(39.5, 60.5, 41.5); // Center
	int barrelLength = 24;

	// Modifiable Projectile data
	Vec3 projVel(0.0, 0.0, 0.0); // Blocks per tick
	Vec3 projPosition(0.0, 0.0, 0.0); // Blocks or meters

	// Target data
	Vec3 targetPos(-48.5, 57.5, 853.5);
	targetPos.y -= 0.75; // Hitbox offset

	// Calculations
	double cannonPitch = 60.0;
	double cannonYaw = calculateYaw(cannonPivot, targetPos);

	double bestPitch = cannonPitch;
	double bestDistance = cannonPivot.distanceToSqr(targetPos);

	double minPitch = -30.0;
	double pitchPrecision = 1.0;

	while (pitchPrecision > (DBL_EPSILON * 10.0)) {
		// Setup projectile
		FireShot(cannonPivot, projVel, projPosition, cannonPitch, cannonYaw, barrelLength, 8);

		// Use this method instead of time to target due to drag
		double currentDist = projPosition.distanceToSqr(targetPos);
		double previousDist = currentDist;
		while (currentDist <= previousDist) {
			// Simulate 1 tick
			tick(projVel, projPosition);

			// Find closest point on projectile's path
			previousDist = currentDist;
			currentDist = projPosition.distanceToSqr(targetPos);
			if (currentDist < bestDistance) {
				bestPitch = cannonPitch;
				bestDistance = currentDist;
			}
		}

		if (cannonPitch <= minPitch) {
			cannonPitch = bestPitch + pitchPrecision;
			minPitch = bestPitch - pitchPrecision;
			pitchPrecision /= 10.0;
		}

		cannonPitch -= pitchPrecision;
	}

	// Tracking https://www.desmos.com/calculator/tec02khlm5
	int currentTick = 0;
	std::ofstream outFile("projectileData.csv", std::ios_base::trunc);

	// Simulation for integrals
	projPosition.x = 0.0;
	projPosition.y = 0.0;
	projPosition.z = 0.0;
	projVel.x = 16.0;
	projVel.y = 0.0;
	projVel.z = 0.0;
	outFile << std::setprecision(10) << "Tick, X-Pos, Y-Pos, Z-Pos, X-Vel, Y-Vel, Z-Vel, Speed" << std::endl;

	while (currentTick < (10 * 20)) {
		// Log data
		outFile << currentTick << ", "
			<< projPosition.x << ", " << projPosition.y << ", " << projPosition.z << ", "
			<< projVel.x << ", " << projVel.y << ", " << projVel.z << ", " << projVel.length()
			<< std::endl;
		currentTick++;

		// Simulate 1 tick
		tick(projVel, projPosition);
	}

	// Close file handle
	outFile.close();

	// Final result
	std::cout << std::setprecision(10) << "Cannon pitch = " << bestPitch << ", Cannon yaw = " << cannonYaw << ", Best distance = " << bestDistance << std::endl;
}