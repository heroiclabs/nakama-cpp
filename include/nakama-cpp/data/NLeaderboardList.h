#pragma once
#include "nakama-cpp/NTypes.h"
#include "nakama-cpp/data/NLeaderboard.h"
#include <string>
#include <memory>

NAKAMA_NAMESPACE_BEGIN

	/// A list of leaderboards
	struct NAKAMA_API NLeaderboardList {
		std::vector<NLeaderboard> leaderboards;	///< The list of leaderboards returned.
		std::string cursor;	///< A pagination cursor (optional).
	};

NAKAMA_NAMESPACE_END
