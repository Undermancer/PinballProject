#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"
#include "PhysBody.h"

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	
}

ModuleGame::~ModuleGame()
{}

// Load assets
bool ModuleGame::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;
	// Bottom wall
	PhysBody* bottomWall = App->physics->CreateRectangle(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 10, SCREEN_WIDTH, 20, b2_staticBody);

	
	// Create ball
	ball = App->physics->CreateCircle(660, 650, 15, b2_dynamicBody);
	ball->listener = this;
	
	// TODO: Load ball texture from assets/balls folder
	// ballTexture = LoadTexture("assets/balls/ball1.png");
	int points1[] = {
		0, 0,
		0, -650,
		-5, -680,
		-10, -685,
		-20, -695,
		-45, -700,
		-460, -700,
		-480, -690,
		-490, -680,
		-495, -670,
		-500, -650,
		-500, 0,
		-499, 0,
		-499, -649,
		-494, -669,
		-489, -679,
		-479, -689,
		-459, -699,
		-46, -699,
		-21, -694,
		-11, -684,
		-6, -679,
		-1, -649,
		-1,0
	};
	int points2[] = {
	0, 0,
	0, -620,
	-1, -620,
	-1, 0
	};
	

	PhysBody* wall1 = App->physics->CreateChain(680, 760, points1, sizeof(points1) / sizeof(int), b2_staticBody, 0);
	PhysBody* wall2 = App->physics->CreateChain(640, 760, points2, sizeof(points2) / sizeof(int), b2_staticBody, 0);


	
	// Create launcher components
	// Create static pivot point at the bottom of movement range - make it a sensor
	launcherPivot = App->physics->CreateCircle(660, 680, 5, b2_staticBody);
	// Make the pivot a sensor so it doesn't collide with the ball
	launcherPivot->body->GetFixtureList()->SetSensor(true);
	
	// Create launcher rectangle aligned with pivot
	launcher = App->physics->CreateRectangle(660, 680, 20, 30, b2_dynamicBody);
	
	// Create prismatic joint for vertical-only movement
	b2PrismaticJointDef jointDef;
	jointDef.bodyA = launcherPivot->body;
	jointDef.bodyB = launcher->body;
	jointDef.collideConnected = false;
	jointDef.localAxisA.Set(0.0f, -1.0f);
	jointDef.localAnchorA.Set(0, 0);
	jointDef.localAnchorB.Set(0, 0);
	jointDef.enableLimit = true;
	jointDef.lowerTranslation = 0.0f;
	jointDef.upperTranslation = 50.0f;
	jointDef.enableMotor = true;
	jointDef.maxMotorForce = 5000.0f;
	jointDef.motorSpeed = 0.0f;

	launcherJoint = (b2PrismaticJoint*)App->physics->GetWorld()->CreateJoint(&jointDef);

	// Initialize launcher position
	launcher->body->SetAwake(true);  // Ensure the body is awake

// Small map with 1-pixel width walls using chains
    int leftRamp[] = {
        0, 0,
        0, -200,    // Vertical section
        50, -250,   // Angled section
        51, -250,   // 1 pixel width
        1, -200,    // Return parallel to first section
        1, 0        // Close the chain
    };

    int rightRamp[] = {
        0, 0,
        0, -200,
        -50, -250,
        -51, -250,
        -1, -200,
        -1, 0
    };

    int bumperWall[] = {
        0, 0,          
        0, -50,        
        50, -50,       
        50, 0,         
        0, 1,         
        51, 1,       
        51, -51,        
        -1, -51,          
        -1, 0           
    };

    // Create the chain bodies - positioned strategically in the map
    PhysBody* leftRampBody = App->physics->CreateChain(250, 400, leftRamp, sizeof(leftRamp) / sizeof(int), b2_staticBody, 0);
    PhysBody* rightRampBody = App->physics->CreateChain(580, 400, rightRamp, sizeof(rightRamp) / sizeof(int), b2_staticBody, 0);
    
    // Add some bumper walls
    PhysBody* bumper1 = App->physics->CreateChain(280, 600, bumperWall, sizeof(bumperWall) / sizeof(int), b2_staticBody, 50);
    PhysBody* bumper2 = App->physics->CreateChain(500, 600, bumperWall, sizeof(bumperWall) / sizeof(int), b2_staticBody, 50);

    // Add a funnel shape to guide the ball
    int funnel[] = {
        0, 0,
        150, -150,
        151, -150,
        1, 0
    };
    
    PhysBody* funnelLeft = App->physics->CreateChain(489, 750, funnel, sizeof(funnel) / sizeof(int), b2_staticBody, 0);
    
    // Mirror the funnel for right side
    int funnel2[] = {
        0, 0,
        -150, -150,
        -151, -150,
        -1, 0
    };
    
    PhysBody* funnelRight = App->physics->CreateChain(331, 750, funnel2, sizeof(funnel2) / sizeof(int), b2_staticBody, 0);
	
	return ret;
}

// Load assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading Intro scene");
	
	// TODO: Unload textures
	// UnloadTexture(ballTexture);
	UnloadTexture(background);
	
	// Clean up launcher components
	if (launcherJoint)
	{
		App->physics->GetWorld()->DestroyJoint(launcherJoint);
		launcherJoint = nullptr;
	}
	
	return true;
}

// Update: draw background
update_status ModuleGame::Update()
{
	// Draw the background at position 0,0
	/*App->renderer->Draw(background, 0, 0);*/
	
	// Handle input
	if(IsKeyPressed(KEY_SPACE))
	{
		// Launch ball or start game
		if(!roundActive)
		{
			roundActive = true;
			gameStarted = true;
		}
	}
	
	// Draw score
	DrawText(TextFormat("Score: %d", currentScore), 20, 30, 18, BLACK);
	DrawText(TextFormat("Previous: %d", previousScore), 20, 50, 18, DARKGRAY);
	DrawText(TextFormat("High Score: %d", highestScore), 20, 80, 18, DARKGREEN);
	DrawText(TextFormat("Balls: %d", ballsRemaining), 20, 110, 18, RED);
	
	// Draw ball
	if(ball)
	{
		int x, y;
		ball->GetPosition(x, y);
		DrawCircle(x, y, 15, BLUE);
	}
	
	// Handle launcher input
	if (IsKeyDown(KEY_SPACE))
	{
		if (!isLauncherPressed)
		{
			isLauncherPressed = true;
		}	
		
		// Move down with constant speed
		launcherJoint->SetMotorSpeed(15.0f);  // Slightly increased downward speed
		LOG("Launcher moving down - Translation: %f", launcherJoint->GetJointTranslation());
	}
	else
	{
		if (isLauncherPressed)
		{
			isLauncherPressed = false;
		}
		
		float translation = launcherJoint->GetJointTranslation();
		LOG("Launcher returning - Translation: %f", translation);
		
		// Return to starting position if not already there
		if (translation > 0.1f)  // If we're away from the starting position
		{
			launcherJoint->SetMotorSpeed(-25.0f);  // Increased return speed significantly
		}
		else
		{
			launcherJoint->SetMotorSpeed(0.0f);  // Stop at starting position
		}
	}

	// Draw launcher
	if (launcher)
	{
		int x, y;
		launcher->GetPosition(x, y);
		DrawRectanglePro(
			Rectangle{ (float)x, (float)y, 15, 40 },  // Rectangle
			Vector2{ 7.5f, 20 },                      // Origin at center
			0.0f,                                     // No rotation
			RED                                       // Color
		);
	}
	// Draw map
	
	DrawLine(680, 760, 680, 110, BLACK);
	DrawLine(680, 110, 675, 80, BLACK);
	DrawLine(675, 80, 670, 75, BLACK);
	DrawLine(670, 75, 650, 65, BLACK);
	DrawLine(650, 65, 625, 60, BLACK);
	DrawLine(625, 60, 220, 60, BLACK);
	DrawLine(220, 60, 200, 70, BLACK);
	DrawLine(200, 70, 190, 80, BLACK);
	DrawLine(190, 80, 185, 90, BLACK);
	DrawLine(185, 90, 180, 110, BLACK);
	DrawLine(180, 110, 180, 760, BLACK);
	DrawLine(640, 760, 640, 140, BLACK);


    // Draw bumpers as green rectangles
    DrawRectangle(280, 550, 50, 50, GREEN);  // bumper1
    DrawRectangle(500, 550, 50, 50, GREEN);  // bumper2

    // Draw chains as black lines
    // Left ramp
    DrawLine(250, 400, 250, 200, BLACK);            // Vertical line
    DrawLine(250, 200, 300, 150, BLACK);            // Angled line
    DrawLine(251, 200, 301, 150, BLACK);            // Parallel line 1 pixel offset

    // Right ramp
    DrawLine(580, 400, 580, 200, BLACK);            // Vertical line
    DrawLine(580, 200, 530, 150, BLACK);            // Angled line
    DrawLine(579, 200, 529, 150, BLACK);            // Parallel line 1 pixel offset

    // Left funnel
    DrawLine(489, 750, 639, 600, BLACK);            // Angled line
    DrawLine(490, 750, 640, 600, BLACK);            // Parallel line 1 pixel offset

    // Right funnel
    DrawLine(331, 750, 181, 600, BLACK);            // Angled line
    DrawLine(330, 750, 180, 600, BLACK);            // Parallel line 1 pixel offset
	return UPDATE_CONTINUE;
}

void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
    // Assuming bodyA is always the ball (since we set the listener on the ball)
    if (bodyB != nullptr)
    {
        // Add the score value of the chain that was hit
        currentScore += bodyB->scoreValue;
        
        // Only play sound if it was a scoring collision
        if (bodyB->scoreValue > 0)
        {
            // TODO: Play scoring sound effect
            // App->audio->PlayFx(scoreFx);
        }
    }
}
