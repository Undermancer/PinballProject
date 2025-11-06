#pragma once

#include "Globals.h"
#include "Module.h"

#include "p2Point.h"

#include "raylib.h"
#include <vector>

#include "box2d/box2d.h" // Убедитесь, что путь к box2d корректен

class PhysBody;
class PhysicEntity;
class b2RevoluteJoint; // Forward declaration, если не включён полный заголовок


class ModuleGame : public Module
{
public:
	ModuleGame(Application* app, bool start_enabled = true);
	~ModuleGame();

	bool Start();
	update_status Update();
	bool CleanUp();
	void OnCollision(PhysBody* bodyA, PhysBody* bodyB) override;

public:
	// Ball
	PhysBody* ball = nullptr;
	Texture2D ballTexture;
	Texture2D background;
	
	// Flippers
	PhysBody* leftFlipper = nullptr;
	PhysBody* rightFlipper = nullptr;
	
	// Score tracking
	int currentScore = 0;
	int previousScore = 0;
	int highestScore = 0;
	int ballsRemaining = 3;
	
	// Game state
	bool gameStarted = false;
	bool roundActive = false;
	
	// Fonts for UI
	Font font;

	// Launcher components
	PhysBody* launcher = nullptr;         // The rectangle that pushes the ball
	PhysBody* launcherPivot = nullptr;    // The static point to attach the joint
	b2PrismaticJoint* launcherJoint = nullptr; // The joint connecting launcher to pivot
	bool isLauncherPressed = false;      // Track if space is being held
};
