#pragma once


/*
Common types shared between the client and server projects
*/


enum class PlayerTeam
{
	None,
	Red,
	Blue
};


enum class GameState
{
	FightMode,
	BuildMode
};