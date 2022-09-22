#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("hexapod.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("hexapod.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

GLuint blocks_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > blocks_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("blocks.pnct"));
	blocks_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > blocks_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("blocks.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = blocks_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = blocks_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;
	});
});

Load< Sound::Sample > alien_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("alien.opus"));
});
Load< Sound::Sample > beach_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("beach.opus"));
});
Load< Sound::Sample > beep_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("beep.opus"));
});
Load< Sound::Sample > blip_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("blip.opus"));
});
Load< Sound::Sample > guns_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("guns.opus"));
});
Load< Sound::Sample > phone_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("phone.opus"));
});
Load< Sound::Sample > spring_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("spring.opus"));
});
Load< Sound::Sample > static_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("static.opus"));
});


PlayMode::PlayMode() : scene(*blocks_scene) {
	// make randomized pairs of blocks
	std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(shuffled_blocks.begin(), shuffled_blocks.end(), g);

	for (size_t i = 0; i < shuffled_blocks.size(); i++) {
		block_pairs[shuffled_blocks[i]] = i % num_pairs;
	}

	//get transformation pointers for convenience:
	for (auto &transform : scene.transforms) {
		for (int i = 0; i < num_blocks; i++) {
			if (transform.name == block_names[i]) blocks[i] = &transform;
			if (transform.name == letter_names[i]) letters[i] = &transform;
		}
	}

	// for (int i = 0; i < shuffled_blocks.size(); i++) {
	// 	std::cout << block_pairs[i] << "  ";
	// }
	// std::cout << std::endl;

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

PlayMode::~PlayMode() {
}

void PlayMode::play_block_sound(int pair_idx) {
	switch (pair_idx) {
		case 0:
			Sound::play(*alien_sample);
			break;
		case 1:
			Sound::play(*beach_sample);
			break;
		case 2:
			Sound::play(*beep_sample);
			break;
		case 3:
			Sound::play(*blip_sample);
			break;
		case 4:
			Sound::play(*guns_sample);
			break;
		case 5:
			Sound::play(*phone_sample);
			break;
		case 6:
			Sound::play(*spring_sample);
			break;
		case 7:
			Sound::play(*static_sample);
			break;
		default:
			break;
	}
}

void PlayMode::found_match(int first, int second) {
	// set pair to 8 so music is no longer played and keys are no longer clickable
	block_pairs[first] = -1;
	block_pairs[second] = -1;

	// adds pair to total
	pairs_found++;

	// move blocks out of camera view
	block_matches_found.push_back(first);
	block_matches_found.push_back(second);
}

void PlayMode::reset_pair(int first, int second) {
	// stop block rotations

}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		}

		for (int i = 0; i < num_blocks; i++) {
			if (evt.key.keysym.sym == block_keycodes[i]) {
				if (!first_pressed) {
					if (block_pairs[i] >= 0) {
						first_block = i;
						play_block_sound(block_pairs[i]);
						first_pressed = true;
					}
				} else {
					if (block_pairs[i] >= 0 && i != first_block) {
						second_block = i;
						play_block_sound(block_pairs[i]);
						if (block_pairs[first_block] == block_pairs[second_block]) {
							found_match(first_block, second_block);
						} else {
							reset_pair(first_block, second_block);
						}
						first_pressed = false;
					}
				}
			}
		}

	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
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

void PlayMode::update(float elapsed) {

	// std::cout << first_block << " " << second_block << std::endl;
	
	rotation_speed += elapsed * 2.0f;

	for (size_t i = 0; i < block_matches_found.size(); i++) {
		int idx = block_matches_found[i];
		blocks[idx]->position = blocks[idx]->position + glm::vec3(0.0f, 0.0f, rotation_speed);
		letters[idx]->position = letters[idx]->position + glm::vec3(0.0f, 0.0f, rotation_speed);
	}

	
	// blocks[0]->rotation = blocks[0]->rotation * glm::vec3(0.0f, 0.0f, rotation_speed);
	// letters[0]->rotation = letters[0]->rotation * glm::vec3(0.0f, 0.0f, rotation_speed);

}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
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
		lines.draw_text("SCORE: " + std::to_string(pairs_found),
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("SCORE: " + std::to_string(pairs_found),
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}

// glm::vec3 PlayMode::get_leg_tip_position() {
// 	//the vertex position here was read from the model in blender:
// 	return lower_leg->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
// }