#ifndef _HUD_H
#define _HUD_H

typedef enum GameOverHUDState
{
	GameOverHUDState_Hidden,
	GameOverHUDState_Rendered
};

void render_player_controls();
void render_game_over();

#endif // _HUD_H
