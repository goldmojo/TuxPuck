/* tuxpuck.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <SDL.h>
#include "video.h"
#include "audio.h"
#include "tuxpuck.h"

/* defines */
#define SETTINGSFILE _settings_file

/* structs */
typedef struct {
	Uint8 sound;
	Uint8 mouse_speed;
	Uint8 joystick_side;
} Settings;

/* Joystick */
SDL_Joystick* GCW_JOYSTICK;
Sint16 GCW_JOYSTICK_DEADZONE;
int GCW_JOYSTIC2MOUSE_SPEEDSTEP;
int GCW_JOYSTICK_X_MOVE;
int GCW_JOYSTICK_Y_MOVE;

/* statics */
static Settings *_settings = NULL;
static char _settings_file[200];

/* Returns a pointer to the selected AI opponent.
 *
 * @param opponent
 */
AIPlayer* select_opponent(Uint8 opponent) {

	AIPlayer* p2 = NULL;

	switch (opponent) {
	case 1:
		p2 = sam_create(board_get_pad(2), board_get_puck());
		break;
	case 2:
		p2 = tin_create(board_get_pad(2), board_get_puck());
		break;
	case 3:
		p2 = coler_create(board_get_pad(2), board_get_puck());
		break;
	case 4:
		p2 = lex_create(board_get_pad(2), board_get_puck());
		break;
	case 5:
		p2 = smasher_create(board_get_pad(2), board_get_puck());
		break;
	case 6:
		p2 = morth_create(board_get_pad(2), board_get_puck());
		break;
	case 7:
		p2 = arcana_create(board_get_pad(2), board_get_puck());
		break;
	case 8:
		p2 = buff_create(board_get_pad(2), board_get_puck());
		break;
	case 9:
		p2 = a1d2_create(board_get_pad(2), board_get_puck());
		break;
	case 10:
		p2 = tux_create(board_get_pad(2), board_get_puck());
		break;
	}

	return p2;
}

/* functions */
static int _play_match(Uint8 opponent) {
	int next_opponent;
	SDL_Event event;
	Uint8 loop = 1, scorer = 0;
	Uint32 elapsed_time = 0;
	char buffer[50];

	/* Human player.*/
	HumanPlayer *p1 = NULL;

	/* Computer opponent. */
	AIPlayer *p2 = NULL;

	Timer *timer = NULL;
	float alpha = 0.0;

	/* Exit menu. */
	Menu *exit_menu;

	/* "Play-again" menu.*/
	Menu *again_menu;

	memset(buffer, 0, 50);
	board_init();
	scoreboard_init();
	video_save();

	p1 = human_create(board_get_pad(1), "Human");
	p2 = select_opponent(opponent);

	exit_menu = menu_create(2);
	menu_add_field(exit_menu, 0, 1, "Continue");
	menu_add_field(exit_menu, 1, 1, "Surrender");

	again_menu = menu_create(2);
	menu_add_field(again_menu, 0, 1, "Play Again");
	menu_add_field(again_menu, 1, 1, "Main Menu");

	timer = timer_create2();
	timer_reset(timer);
	
	while (loop) {
		while (SDL_PollEvent(&event))
			if (event.type == SDL_KEYDOWN) {
				loop = 0;
				alpha = 1.0;
			}
		SDL_Delay(SLEEP);
		timer_update(timer);
		timer_reset(timer);
		elapsed_time = timer_elapsed(timer);
		alpha += elapsed_time * 0.001;
		if (alpha > 1.0) {
			loop = 0;
			alpha = 1.0;
		}
		board_clean_up();
		scoreboard_erase();
		aiplayer_erase(p2);
		aiplayer_set_alpha(p2, (Uint8) (alpha * 255));
		scoreboard_set_alpha((Uint8) (alpha * 255));
		entity_set_alpha((Entity *) board_get_puck(), (Uint8) (alpha * 255));
		entity_set_alpha(board_get_pad(1), (Uint8) (alpha * 255.0 / 2.0));
		entity_set_alpha(board_get_pad(2), (Uint8) (alpha * 255.0 / 2.0));
		aiplayer_blit(p2);
		board_reblit();
		scoreboard_blit();
		video_update();
	}
	loop = 1;
	board_clean_up();
	aiplayer_blit(p2);
	video_save();
	board_reblit();
	video_update();
	SDL_PumpEvents();
	SDL_GetRelativeMouseState(NULL, NULL);
#ifndef _DEBUG
	SDL_WM_GrabInput(SDL_GRAB_ON);
#endif
	human_set_speed(p1, _settings->mouse_speed);
	timer_reset(timer);
	while (loop) {
		while (SDL_PollEvent(&event))
			switch (event.type) {
			/* Joystick */
			case SDL_JOYAXISMOTION:
				/* Map stick to mouse */
				/* Left */
				if(SDL_JoystickGetAxis(GCW_JOYSTICK,_settings->joystick_side)<-GCW_JOYSTICK_DEADZONE) {
					GCW_JOYSTICK_X_MOVE = (SDL_JoystickGetAxis(GCW_JOYSTICK,_settings->joystick_side) / GCW_JOYSTIC2MOUSE_SPEEDSTEP) - 1;
				}
				/* Right */
				if(SDL_JoystickGetAxis(GCW_JOYSTICK,_settings->joystick_side)>GCW_JOYSTICK_DEADZONE) {
					GCW_JOYSTICK_X_MOVE = (SDL_JoystickGetAxis(GCW_JOYSTICK,_settings->joystick_side) / GCW_JOYSTIC2MOUSE_SPEEDSTEP) + 1;
				}
				/* Up */
				if(SDL_JoystickGetAxis(GCW_JOYSTICK,_settings->joystick_side+1)<-GCW_JOYSTICK_DEADZONE) {
					GCW_JOYSTICK_Y_MOVE = (SDL_JoystickGetAxis(GCW_JOYSTICK,_settings->joystick_side+1) / GCW_JOYSTIC2MOUSE_SPEEDSTEP) - 1;
				}
				/* Down */
				if(SDL_JoystickGetAxis(GCW_JOYSTICK,_settings->joystick_side+1)>GCW_JOYSTICK_DEADZONE) {
					GCW_JOYSTICK_Y_MOVE = (SDL_JoystickGetAxis(GCW_JOYSTICK,_settings->joystick_side+1) / GCW_JOYSTIC2MOUSE_SPEEDSTEP) + 1;
				}
				/* Release movement if necessary */
				if((SDL_JoystickGetAxis(GCW_JOYSTICK,_settings->joystick_side)>-GCW_JOYSTICK_DEADZONE) && (SDL_JoystickGetAxis(GCW_JOYSTICK,_settings->joystick_side)<GCW_JOYSTICK_DEADZONE)) {
					GCW_JOYSTICK_X_MOVE = 0;
				}
				if((SDL_JoystickGetAxis(GCW_JOYSTICK,_settings->joystick_side+1)>-GCW_JOYSTICK_DEADZONE) && (SDL_JoystickGetAxis(GCW_JOYSTICK,_settings->joystick_side+1)<GCW_JOYSTICK_DEADZONE)) {
					GCW_JOYSTICK_Y_MOVE = 0;
				}
				break;
			/* Key down */
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				/* SELECT */
				case SDLK_ESCAPE:
					if (menu_get_selected(exit_menu) == 1)
						loop = 0;
					timer_reset(timer);
					break;
				/* START */
				case SDLK_RETURN:
					_settings->sound = !_settings->sound;
					audio_set_mute(!_settings->sound);
					break;
				/* B */
				case SDLK_LALT:
					if (_settings->mouse_speed > 1)
						_settings->mouse_speed--;
					human_set_speed(p1, _settings->mouse_speed);
					scoreboard_set_mousebar(_settings->mouse_speed);
					break;
				/* A */
				case SDLK_LCTRL:
					if (_settings->mouse_speed < 10)
						_settings->mouse_speed++;
					human_set_speed(p1, _settings->mouse_speed);
					scoreboard_set_mousebar(_settings->mouse_speed);
					break;
				/* L2 */
				case SDLK_PAGEUP:
					_settings->joystick_side = 0;
					break;
				/* R2 */
				case SDLK_PAGEDOWN:
					_settings->joystick_side = 2;
					break;
				default:
					break;
				}
				break;
			case SDL_QUIT:
				loop = 0;
				break;
			}
		SDL_Delay(SLEEP);
		timer_update(timer);
		timer_reset(timer);
		elapsed_time = timer_elapsed(timer);
		human_update(p1, elapsed_time, GCW_JOYSTICK_X_MOVE, GCW_JOYSTICK_Y_MOVE);
		aiplayer_update(p2, elapsed_time);
		scoreboard_update(elapsed_time);
		if ((scorer = board_update(elapsed_time)) != 0) {
			scoreboard_add_point(scorer);
			if (scorer == 1) {
				human_give_point(p1);
				p2->set_state(p2, PLAYER_STATE_LOOSE_POINT);
			} else {
				p2->points++;
				p2->set_state(p2, PLAYER_STATE_WIN_POINT);
			}
			if (human_get_points(p1) >= 15 || p2->points >= 15) {
				if (human_get_points(p1) == 15)
					p2->set_state(p2, PLAYER_STATE_LOOSE_GAME);
				else
					p2->set_state(p2, PLAYER_STATE_WIN_GAME);
			}
		}
		board_clean_up();
		scoreboard_clean_up();
		scoreboard_reblit();
		if (p2->reblit(p2, elapsed_time) == 0)
			loop = 0;
		board_reblit();
		video_update();
	}
#ifndef _DEBUG
	SDL_WM_GrabInput(SDL_GRAB_OFF);
#endif
	menu_free(exit_menu);
	timer_free(timer);
	human_free(p1);
	p2->free(p2);
	board_deinit();
	scoreboard_deinit();
	if (menu_get_selected(again_menu) == 0)
		next_opponent = opponent;
	else
		next_opponent = -1;
	menu_free(again_menu);
	return next_opponent;
}

static void _read_settings(void) {
	FILE *file = NULL;
	char buffer[100], buffer2[100];
	Uint32 uint32 = 0;

	if ((file = fopen(SETTINGSFILE, "r")) == NULL)
		return;
	while (fgets(buffer, 100, file) != 0) {
		if (sscanf(buffer, "%s %d\n", buffer2, &uint32) != 2) {
			fclose(file);
			return;
		} else if (strcmp(buffer2, "SOUND") == 0)
			_settings->sound = (Uint8) uint32;
		else if (strcmp(buffer2, "MOUSESPEED") == 0)
			_settings->mouse_speed = (Uint8) uint32;
		else if (strcmp(buffer2, "JOYSIDE") == 0)
			_settings->joystick_side = (Uint8) uint32;
	}
	fclose(file);
}

static void _save_settings(void) {
	FILE *file = NULL;

	if ((file = fopen(SETTINGSFILE, "w")) == NULL)
		return;
	fprintf(file, "SOUND %d\n", _settings->sound);
	fprintf(file, "MOUSESPEED %d\n", _settings->mouse_speed);
	fprintf(file, "JOYSIDE %d\n", _settings->joystick_side);
	fclose(file);
}

static void _tuxpuck_init(void) {

	char *homeDir = NULL;

	srand(time(NULL));

	SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	SDL_JoystickEventState(SDL_ENABLE);
	GCW_JOYSTICK = SDL_JoystickOpen(0);
	GCW_JOYSTICK_DEADZONE = 500;
	/* Maximum joystick value is 32768
	   Will change mouse speed every 4000 */
	GCW_JOYSTIC2MOUSE_SPEEDSTEP = 4000;
	GCW_JOYSTICK_X_MOVE = 0;
	GCW_JOYSTICK_Y_MOVE = 0;
	
	audio_init();
	video_init();

	video_save();
	_settings = (Settings *) malloc(sizeof(Settings));
	memset(_settings, 0, sizeof(Settings));
	_settings->sound = 1;
	_settings->mouse_speed = 5;
	/* 0 = left - 2 = right */
	_settings->joystick_side = 0;

	homeDir = getenv("HOME");
	sprintf(_settings_file, "%s/.tuxpuckrc", homeDir);

	_read_settings();
	audio_set_mute(!_settings->sound);

	run_intro();
	video_save();
}

static void _tuxpuck_deinit(void) {
	audio_deinit();
	video_deinit();
	SDL_Quit();
	_save_settings();
	free(_settings);
}

Menu* create_game_menu() {
	Menu* main_menu = menu_create(2);
	menu_add_field(main_menu, 0, 1, "Play");
	menu_add_field(main_menu, 1, 1, "Exit");
	return main_menu;
}

Menu* create_opponent_menu() {
	Menu* op_menu = menu_create(11);
	menu_add_field(op_menu, 0, 0, "Opponent");
	menu_add_field(op_menu, 1, 1, "Sam");
	menu_add_field(op_menu, 2, 1, "Tin");
	menu_add_field(op_menu, 3, 1, "Coler");
	menu_add_field(op_menu, 4, 1, "Lex");
	menu_add_field(op_menu, 5, 1, "Smasher");
	menu_add_field(op_menu, 6, 1, "Morth");
	menu_add_field(op_menu, 7, 1, "Arcana");
	menu_add_field(op_menu, 8, 1, "Buff");
	menu_add_field(op_menu, 9, 1, "A1d2");
	menu_add_field(op_menu, 10, 1, "Tux");
	return op_menu;
}

/* main method*/
int main(int argc, char *argv[]) {
	
	printf("Here we go!\n");

	int next_opponent;
	Menu *main_menu, *opponent_menu;

	_tuxpuck_init();
	main_menu = create_game_menu();
	opponent_menu = create_opponent_menu();
	int loop = 1;
	while (loop) {
		int selection = menu_get_selected(main_menu);

		printf("Main menu selection is %i\n", selection);
		switch (selection) {

		/* Escape was pressed.*/
		case -1:

			loop = 0;
			break;
		case 0:
			next_opponent = menu_get_selected(opponent_menu);
			while (next_opponent != -1)
				next_opponent = _play_match(next_opponent);
			break;
		case 1:
			loop = 0;
			break;
		default:
			printf("This case should not exist\n");
			break;
		}
	}
	menu_free(opponent_menu);
	menu_free(main_menu);
	_tuxpuck_deinit();
	return 0;
}
