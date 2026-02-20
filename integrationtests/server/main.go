package main

import (
	"context"
	"database/sql"
	"encoding/json"
	"fmt"
	"time"

	"github.com/heroiclabs/nakama-common/runtime"
)

func InitModule(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, initializer runtime.Initializer) error {
	logger.Info("nakama-cpp test module loaded")

	// Register match handler for authoritative matches
	if err := initializer.RegisterMatch("test_match", newTestMatch); err != nil {
		return fmt.Errorf("register match: %w", err)
	}

	// clientrpc.rpc — Echo: return payload as-is, empty if no payload
	if err := initializer.RegisterRpc("clientrpc.rpc", rpcEcho); err != nil {
		return fmt.Errorf("register rpc echo: %w", err)
	}

	// clientrpc.rpc_error — Always return an error
	if err := initializer.RegisterRpc("clientrpc.rpc_error", rpcError); err != nil {
		return fmt.Errorf("register rpc error: %w", err)
	}

	// clientrpc.rpc_get — Return non-empty JSON (callable via HTTP GET)
	if err := initializer.RegisterRpc("clientrpc.rpc_get", rpcGet); err != nil {
		return fmt.Errorf("register rpc get: %w", err)
	}

	// clientrpc.send_notification — Send a notification to the user specified in payload
	if err := initializer.RegisterRpc("clientrpc.send_notification", rpcSendNotification); err != nil {
		return fmt.Errorf("register rpc send_notification: %w", err)
	}

	// clientrpc.create_authoritative_match — Create an authoritative match
	if err := initializer.RegisterRpc("clientrpc.create_authoritative_match", rpcCreateAuthoritativeMatch); err != nil {
		return fmt.Errorf("register rpc create_authoritative_match: %w", err)
	}

	// clientrpc.create_tournament — Create a tournament from params
	if err := initializer.RegisterRpc("clientrpc.create_tournament", rpcCreateTournament); err != nil {
		return fmt.Errorf("register rpc create_tournament: %w", err)
	}

	// clientrpc.delete_tournament — Delete a tournament by ID
	if err := initializer.RegisterRpc("clientrpc.delete_tournament", rpcDeleteTournament); err != nil {
		return fmt.Errorf("register rpc delete_tournament: %w", err)
	}

	return nil
}

// rpcEcho returns the payload as-is. If no payload, returns empty.
func rpcEcho(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, payload string) (string, error) {
	return payload, nil
}

// rpcError always returns an error.
func rpcError(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, payload string) (string, error) {
	return "", runtime.NewError("rpc_error: intentional error for testing", 13) // INTERNAL
}

// rpcGet returns a non-empty JSON response.
func rpcGet(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, payload string) (string, error) {
	resp := map[string]interface{}{
		"message":   "ok",
		"timestamp": time.Now().Unix(),
	}
	data, err := json.Marshal(resp)
	if err != nil {
		return "", runtime.NewError("failed to marshal response", 13)
	}
	return string(data), nil
}

// rpcSendNotification sends a notification to the user_id in the payload.
func rpcSendNotification(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, payload string) (string, error) {
	var input map[string]string
	if err := json.Unmarshal([]byte(payload), &input); err != nil {
		return "", runtime.NewError("invalid payload", 3) // INVALID_ARGUMENT
	}

	userID, ok := input["user_id"]
	if !ok || userID == "" {
		return "", runtime.NewError("missing user_id", 3)
	}

	senderID, _ := ctx.Value(runtime.RUNTIME_CTX_USER_ID).(string)

	content := map[string]interface{}{
		"message": "test notification",
	}

	if err := nk.NotificationSend(ctx, userID, "Test Notification", content, 1, senderID, true); err != nil {
		return "", runtime.NewError("failed to send notification: "+err.Error(), 13)
	}

	return "", nil
}

// rpcCreateAuthoritativeMatch creates an authoritative match and returns its ID.
func rpcCreateAuthoritativeMatch(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, payload string) (string, error) {
	var params map[string]interface{}
	if payload != "" {
		if err := json.Unmarshal([]byte(payload), &params); err != nil {
			return "", runtime.NewError("invalid payload", 3)
		}
	}
	if params == nil {
		params = map[string]interface{}{}
	}

	matchID, err := nk.MatchCreate(ctx, "test_match", params)
	if err != nil {
		return "", runtime.NewError("failed to create match: "+err.Error(), 13)
	}

	resp, _ := json.Marshal(map[string]string{"match_id": matchID})
	return string(resp), nil
}

// rpcCreateTournament creates a tournament from the provided parameters.
func rpcCreateTournament(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, payload string) (string, error) {
	var input map[string]interface{}
	if err := json.Unmarshal([]byte(payload), &input); err != nil {
		return "", runtime.NewError("invalid payload", 3)
	}

	getString := func(key, fallback string) string {
		if v, ok := input[key]; ok {
			if s, ok := v.(string); ok {
				return s
			}
		}
		return fallback
	}
	getInt := func(key string, fallback int) int {
		if v, ok := input[key]; ok {
			if f, ok := v.(float64); ok {
				return int(f)
			}
		}
		return fallback
	}
	getBool := func(key string, fallback bool) bool {
		if v, ok := input[key]; ok {
			if b, ok := v.(bool); ok {
				return b
			}
		}
		return fallback
	}

	id := fmt.Sprintf("tournament_%d", time.Now().UnixNano())
	authoritative := getBool("authoritative", true)
	sortOrder := getString("sort_order", "desc")
	operator := getString("operator", "best")
	resetSchedule := getString("reset_schedule", "")
	metadata := map[string]interface{}{}
	title := getString("title", "Test Tournament")
	description := getString("description", "")
	category := getInt("category", 1)
	startTime := getInt("start_time", int(time.Now().Unix()))
	endTime := getInt("end_time", 0)
	duration := getInt("duration", 3600)
	maxSize := getInt("max_size", 10000)
	maxNumScore := getInt("max_num_score", 3)
	joinRequired := getBool("join_required", true)

	enableRanks := true
	if err := nk.TournamentCreate(ctx, id, authoritative, sortOrder, operator, resetSchedule, metadata, title, description, category, startTime, endTime, duration, maxSize, maxNumScore, joinRequired, enableRanks); err != nil {
		return "", runtime.NewError("failed to create tournament: "+err.Error(), 13)
	}

	resp, _ := json.Marshal(map[string]string{"tournament_id": id})
	return string(resp), nil
}

// rpcDeleteTournament deletes a tournament by its ID.
func rpcDeleteTournament(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, payload string) (string, error) {
	var input map[string]string
	if err := json.Unmarshal([]byte(payload), &input); err != nil {
		return "", runtime.NewError("invalid payload", 3)
	}

	tournamentID, ok := input["tournament_id"]
	if !ok || tournamentID == "" {
		return "", runtime.NewError("missing tournament_id", 3)
	}

	if err := nk.TournamentDelete(ctx, tournamentID); err != nil {
		return "", runtime.NewError("failed to delete tournament: "+err.Error(), 13)
	}

	return "", nil
}

// --- Authoritative Match Handler ---

type testMatchState struct {
	presences map[string]runtime.Presence
	debug     bool
	label     string
}

type testMatch struct{}

func newTestMatch(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule) (runtime.Match, error) {
	return &testMatch{}, nil
}

func (m *testMatch) MatchInit(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, params map[string]interface{}) (interface{}, int, string) {
	debug := false
	if v, ok := params["debug"]; ok {
		if b, ok := v.(bool); ok {
			debug = b
		}
	}

	label := "TestAuthoritativeMatch"
	if v, ok := params["label"]; ok {
		if s, ok := v.(string); ok {
			label = s
		}
	}

	state := &testMatchState{
		presences: make(map[string]runtime.Presence),
		debug:     debug,
		label:     label,
	}

	tickRate := 1
	return state, tickRate, label
}

func (m *testMatch) MatchJoinAttempt(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, dispatcher runtime.MatchDispatcher, tick int64, state interface{}, presence runtime.Presence, metadata map[string]string) (interface{}, bool, string) {
	return state, true, ""
}

func (m *testMatch) MatchJoin(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, dispatcher runtime.MatchDispatcher, tick int64, state interface{}, presences []runtime.Presence) interface{} {
	s := state.(*testMatchState)
	for _, p := range presences {
		s.presences[p.GetUserId()] = p
		if s.debug {
			logger.Info("Player joined: %s", p.GetUserId())
		}
	}
	return s
}

func (m *testMatch) MatchLeave(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, dispatcher runtime.MatchDispatcher, tick int64, state interface{}, presences []runtime.Presence) interface{} {
	s := state.(*testMatchState)
	for _, p := range presences {
		delete(s.presences, p.GetUserId())
	}
	// Terminate if everyone left
	if len(s.presences) == 0 {
		return nil
	}
	return s
}

func (m *testMatch) MatchLoop(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, dispatcher runtime.MatchDispatcher, tick int64, state interface{}, messages []runtime.MatchData) interface{} {
	s := state.(*testMatchState)
	// Echo messages back to sender
	for _, msg := range messages {
		_ = dispatcher.BroadcastMessage(msg.GetOpCode(), msg.GetData(), []runtime.Presence{msg}, nil, true)
	}
	return s
}

func (m *testMatch) MatchTerminate(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, dispatcher runtime.MatchDispatcher, tick int64, state interface{}, graceSeconds int) interface{} {
	return state
}

func (m *testMatch) MatchSignal(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, dispatcher runtime.MatchDispatcher, tick int64, state interface{}, data string) (interface{}, string) {
	return state, "signal received: " + data
}
