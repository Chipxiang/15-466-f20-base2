#include "TetrisMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "SDL_keycode.h"
#include "SDL_mouse.h"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint cube_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > cube_meshes(LoadTagDefault, []() -> MeshBuffer const* {
	MeshBuffer const* ret = new MeshBuffer(data_path("cube2.pnct"));
	cube_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
	});

Load< Scene > cube_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("cube2.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		std::cout << mesh_name << " " << transform->position.x << " " << transform->position.y << " " << transform->position.z << std::endl;
		Mesh const &mesh = cube_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable& drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = cube_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

		});
	});


Scene::Transform* TetrisMode::add_cube(Scene::Transform* parent, glm::vec3 pos_offset) {
	Mesh const& cube_mesh = cube_meshes->lookup("Cube");

	// create new transform
	scene.transforms.emplace_back();
	Scene::Transform* t = &scene.transforms.back();
	if (parent == nullptr) {
		t->name = "Cube";
	}
	else {
		t->position = parent->position + pos_offset;
		t->name = parent->name + "-child";
		t->parent = parent;
	}

	Scene::Drawable drawable(t);
	drawable.pipeline = lit_color_texture_program_pipeline;
	drawable.pipeline.vao = cube_meshes_for_lit_color_texture_program;
	drawable.pipeline.type = cube_mesh.type;
	drawable.pipeline.start = cube_mesh.start;
	drawable.pipeline.count = cube_mesh.count;
	scene.drawables.emplace_back(drawable);
	return t;
}

glm::vec3 TetrisMode::get_world_position(Scene::Transform* transform) {
	glm::vec3 world_position;
	if (transform->parent) {
		world_position = transform->make_local_to_parent() * glm::vec4(transform->parent->position.x, transform->parent->position.y, transform->parent->position.z, 1);
	}
	else {
		world_position = transform->position;
	}
	return world_position;
}

void TetrisMode::break_down_child_parent_relations(Scene::Transform* transform) {
	if (!transform->parent)
		return;
	glm::vec3 world_position = get_world_position(transform);
	transform->parent = nullptr;
	transform->position.x = world_position.x;
	transform->position.y = world_position.y;
	transform->position.z = world_position.z;
}
bool TetrisMode::is_collide() {
	// Check each of the moving block's position.
	for (int i = 0; i < 4; i++) {
		glm::vec3 world_position = get_world_position(moving_block[i]);

		/*std::cout << "Cube: " << i << " " << "Pos:" << moving_block[i]->position.x << "," << moving_block[i]->position.y << "," << moving_block[i]->position.z << std::endl;
		std::cout << "World Pos:" << world_position.x << "," << world_position.y << "," << world_position.z << std::endl;*/
		// Floor
		if (world_position.z <= GROUND_Z + CUBE_SIZE)
			return true;
		
		/*
		for (int n = 0; n < 3; n++) {
			for (int m = 0; m < 4; m++) {
				std::cout << moving_block[i]->make_local_to_world()[m][n] << " ";
			}
			std::cout << std::endl;
		}*/
		// On the pile
		int x = (int)ceil(world_position.x - MIN_X) / CUBE_SIZE;
		int y = (int)ceil(world_position.y - MIN_Y) / CUBE_SIZE;
		int z = (int)ceil(world_position.z - GROUND_Z) / CUBE_SIZE;
		if (!pile_exists[x][y][z])
			continue;
		else {
			return true;
		}
	}
	return false;
}

void TetrisMode::record_drawables() {
	int i = 0;
	std::list<Scene::Drawable>::iterator it = scene.drawables.end();
	it--;
	for (; i < 4; --it) {
		i++;
		glm::vec3 world_position = get_world_position(it->transform);
		std::cout << world_position.x << "," << world_position.y << "," << world_position.z << std::endl;
		int x = (int)ceil(world_position.x - MIN_X) / CUBE_SIZE;
		int y = (int)ceil(world_position.y - MIN_Y) / CUBE_SIZE;
		int z = (int)ceil(world_position.z - GROUND_Z) / CUBE_SIZE;
		pile_drawables[x][y][z] = it;
		pile_exists[x][y][z] = true;
		std::cout << x << "," << y << "," << z << std::endl;
	}
	/*
	for (int o = 0; o < X_DIM; o++) {
		for (int p = 0; p < Y_DIM; p++) {
			for (int q = 0; q < Z_DIM; q++) {
				if (pile_exists[o][p][q])
					std::cout << pile_drawables[o][p][q]->transform->name;
			}
		}
		std::cout << std::endl;
	}*/
	for (int i = 0; i < 4; i++) {
		
		
	}

}

void TetrisMode::generate_cubes() {
	// randomly generate 5 shapes
	int randint = rand() % 5;

	// TODO put the previously moving block to the stable vector
	// deleting the current block for testing
	//while (scene.drawables.size() > 0)
	//	scene.drawables.pop_back();
	/*bool flag = false;
	for (int o = 0; o < X_DIM; o++) {
		if (flag) break;
		for (int p = 0; p < Y_DIM; p++) {
			for (int q = 0; q < 2; q++) {
				if (pile_exists[o][p][q]) {
					scene.drawables.erase(pile_drawables[o][p][q]);
					pile_exists[o][p][q] = false;
				}
			}
		}
	}*/
	// reset base_cuby by creating a new cube
	moving_block[0] = add_cube(nullptr, glm::vec3(0, 0, 0));

	switch (randint) {
	case 0:
		// triangle
		moving_block[1] = add_cube(moving_block[0], glm::vec3(0, 0, 2));
		moving_block[2] = add_cube(moving_block[0], glm::vec3(2, 0, 0));
		moving_block[3] = add_cube(moving_block[0], glm::vec3(-2, 0, 0));
		break;
	case 1:
		// line
		moving_block[1] = add_cube(moving_block[0], glm::vec3(0, 0, 2));
		moving_block[2] = add_cube(moving_block[0], glm::vec3(0, 0, -2));
		moving_block[3] = add_cube(moving_block[0], glm::vec3(0, 0, 4));
		break;
	case 2:
		// square
		moving_block[1] = add_cube(moving_block[0], glm::vec3(0, 0, 2));
		moving_block[2] = add_cube(moving_block[0], glm::vec3(2, 0, 2));
		moving_block[3] = add_cube(moving_block[0], glm::vec3(2, 0, 0));
		break;
	case 3:
		// Z block
		moving_block[1] = add_cube(moving_block[0], glm::vec3(0, 0, 2));
		moving_block[2] = add_cube(moving_block[0], glm::vec3(-2, 0, 2));
		moving_block[3] = add_cube(moving_block[0], glm::vec3(2, 0, 0));
		break;
	case 4:
		// L block
		moving_block[1] = add_cube(moving_block[0], glm::vec3(0, 0, 2));
		moving_block[2] = add_cube(moving_block[0], glm::vec3(0, 0, 4));
		moving_block[3] = add_cube(moving_block[0], glm::vec3(-2, 0, 0));
		break;
	}
}

void TetrisMode::create_floor() {
	Mesh const &blue = cube_meshes->lookup("blue_plane");
	Mesh const &orange = cube_meshes->lookup("orange_plane");
	Mesh plane[2] = {blue, orange};
	int current_index = 0;
    
	for (int x = -width; x < width; x += 2) {
		for (int y = -width; y < width; y += 2) {
			// create new transform
			scene.transforms.emplace_back();
			Scene::Transform *t = &scene.transforms.back();
			t->position = glm::vec3(x, y, GROUND_Z-1);
			t->name = "plane(" + std::to_string(x) + ", " + std::to_string(y) + ")";

			Scene::Drawable drawable(t);
			drawable.pipeline = lit_color_texture_program_pipeline;
			drawable.pipeline.vao = cube_meshes_for_lit_color_texture_program;
			drawable.pipeline.type = plane[current_index].type;
			drawable.pipeline.start = plane[current_index].start;
			drawable.pipeline.count = plane[current_index].count;
			scene.drawables.emplace_back(drawable);

			current_index = (current_index + 1) % 2;
		}
		current_index = (current_index + 1) % 2;
	}
}

TetrisMode::TetrisMode() : scene(*cube_scene) {
	for (auto& transform : scene.transforms) {
		std::cout << transform.name << " " << transform.position.x << " " << transform.position.y << " " << transform.position.z << std::endl;
		if (transform.name == "Cube") moving_block[0] = &transform;
		if (transform.name == "orange_plane") plane[0] = &transform;
		if (transform.name == "blue_plane") plane[1] = &transform;
	}
	if (moving_block[0] == nullptr) throw std::runtime_error("Cube not found.");
	if (plane[0] == nullptr) throw std::runtime_error("orange_plane not found.");
	if (plane[1] == nullptr) throw std::runtime_error("blue_plane not found.");

	// clear the current drawables
	while(scene.drawables.size() > 0)
	 	scene.drawables.pop_back();


	// create the floor 
	create_floor();
	// create the first moving block
	generate_cubes();

	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
	// reset camera position
	camera->transform->position = glm::vec3(54, -55, 30);
}

TetrisMode::~TetrisMode() {
}

bool TetrisMode::handle_event(SDL_Event const& evt, glm::uvec2 const& window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		}
	}
	else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
		// mouse click for rotation
		// https://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/transforms/examples/index.htm

		if (evt.button.button == SDL_BUTTON_LEFT) {
			moving_block[0]->rotation *= glm::quat(0.7071f, 0.7071f, 0.0f, 0.0f); // 90 degrees by axis
		}
		else if (evt.button.button == SDL_BUTTON_RIGHT) {
			moving_block[0]->rotation *= glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f); // 90 degrees by y axis
		}
		else if (evt.button.button == SDL_BUTTON_MIDDLE) {
			moving_block[0]->rotation *= glm::quat(0.7071f, 0.0f, 0.0f, 0.7071f); // 90 degrees by z axis
		}
	}
	else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void TetrisMode::update(float elapsed) {

	//move camera:
	{
		/*
		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x = -1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y = -1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
		*/

		// for testing
		if (space.pressed) {
			generate_cubes();
			space.pressed = false;
		}
 
		// TODO update the position checking 
		if (left.pressed) {
			if (moving_block[0]->position.x > -width)
				moving_block[0]->position += glm::vec3(-2, 0, 0);
			left.pressed = false;
		}
		else if (right.pressed) {
			if (moving_block[0]->position.x < width) {
				moving_block[0]->position += glm::vec3(2, 0, 0);
			}
			right.pressed = false;
		}
		else if (down.pressed) {
			if (moving_block[0]->position.y > -width) {
				moving_block[0]->position += glm::vec3(0, -2, 0);
			}
			down.pressed = false;
		}
		else if (up.pressed) {
			if (moving_block[0]->position.y < width) {
				moving_block[0]->position += glm::vec3(0, 2, 0);
			}
			up.pressed = false;
		}
	}

	if (is_collide()) {
		for (int i = 0; i < 4; i++) {
			break_down_child_parent_relations(moving_block[i]);
		}
		record_drawables();
		generate_cubes();
	}
		
	else {
		moving_block[0]->position += glm::vec3(0, 0, -0.1);
	}
	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void TetrisMode::draw(glm::uvec2 const& drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	GL_ERRORS();
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, -1.0f)));
	GL_ERRORS();
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	GL_ERRORS();
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + +0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));

	}
}
