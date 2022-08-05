#pragma once

#include "nakama-c/NTypes.h"

#ifdef __cplusplus
extern "C" {
#endif


	/// A leaderboard on the server.
	typedef struct NAKAMA_API NLeaderboard {
		const char* id;		///< The ID of the leaderboard.
		uint32_t sortOrder;		///< ASC(0) or DESC(1) sort mode of scores in the leaderboard.
		NOperator operatorType;		///< BEST, SET, INCREMENT or DECREMENT operator mode of the leaderboard.
		uint32_t prevReset;		///< The UNIX time when the leaderboard was previously reset. A computed value.
		uint32_t nextReset;		///< The UNIX time when the leaderboard is next playable. A computed value.
		const char* metadata;		///< Additional information stored as a JSON object.
		NTimestamp createTime;		///< The UNIX time when the leaderboard was created.
		bool authoritative;		///< Wether the leaderboard was created authoritatively or not.
	}sNLeaderboard;


#ifdef __cplusplus
}
#endif
