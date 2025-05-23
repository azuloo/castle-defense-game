#ifndef _PLAYER_H
#define _PLAYER_H

#include "global_decl.h"

typedef struct CurrencyDef
{
	DrawableDef* currency_drawable;
	int amount;
} CurrencyDef;

int init_player_currency();
void change_player_currency_amount(int amount);

#endif // _PLAYER_H
