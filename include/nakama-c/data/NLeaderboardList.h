#pragma once
#include "nakama-c/NTypes.h"
#include "nakama-c/data/NLeaderboard.h"

#ifdef __cplusplus
extern "C" {
#endif

	/// A list of leaderboards
	typedef struct NAKAMA_API NLeaderboardList{
		sNLeaderboard* leaderboards;		///< The list of leaderboards returned.
		uint16_t leaderboardsCount;
		const char* cursor;		///< A pagination cursor (optional).
	}sNLeaderboardList;

#ifdef __cplusplus
}
#endif