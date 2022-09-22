#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----
	void play_block_sound(int pair_idx);
	void found_match(int first, int second);
	void reset_pair(int first, int second);

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	const static int num_blocks = 16;
	const static int num_pairs = num_blocks / 2;
	bool first_pressed = false;
	int first_block = 0;
	int second_block = 0;
	std::vector<int> selected_blocks;
	int block_pairs[num_blocks];
	int pairs_found = 0;
	std::vector<int> block_matches_found;
	SDL_KeyCode block_keycodes[num_blocks] = { SDLK_1, SDLK_2, SDLK_3, SDLK_4,
											   SDLK_q, SDLK_w, SDLK_e, SDLK_r,
											   SDLK_a, SDLK_s, SDLK_d, SDLK_f,
											   SDLK_z, SDLK_x, SDLK_c, SDLK_v };

	std::string block_names[num_blocks] = { "block1", "block2", "block3", "block4",
											"blockQ", "blockW", "blockE", "blockR",
											"blockA", "blockS", "blockD", "blockF",
											"blockZ", "blockX", "blockC", "blockV" };

	std::string letter_names[num_blocks] = { "letter1", "letter2", "letter3", "letter4",
											 "letterQ", "letterW", "letterE", "letterR",
											 "letterA", "letterS", "letterD", "letterF",
											 "letterZ", "letterX", "letterC", "letterV" };

	// for randomizing pairs
	std::vector<int> shuffled_blocks = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	Scene::Transform *blocks[num_blocks];
	Scene::Transform *letters[num_blocks];

	float rotation_speed = 0.0f;

	// glm::vec3 get_leg_tip_position();

	// //music coming from the tip of the leg (as a demonstration):
	// std::shared_ptr< Sound::PlayingSample > leg_tip_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;

};