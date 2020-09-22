#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct TetrisMode : Mode {
	TetrisMode();
	virtual ~TetrisMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//the moving cubes
	Scene::Transform *moving_block[4];
	// the floor
	Scene::Transform *plane[2];
	
	//camera:
	Scene::Camera *camera = nullptr;

	// add cube, return the pointer to the new cube
	Scene::Transform *add_cube(Scene::Transform *parent, glm::vec3 pos_offset);

	// generate new cubes
	void generate_cubes();

	// show the floor tiles
	void create_floor();

	// the position of ground 
	int z = -10;
	// the width of the space
	int width = 10;
};
