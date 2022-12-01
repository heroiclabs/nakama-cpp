/*
 * Copyright 2019 The Nakama Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "nakama-cpp/realtime/NRtClientInterface.h"
#include "rtapi/realtime.pb.h"
#include "NRtClientProtocolInterface.h"
#include <map>

namespace Nakama {

    struct RtRequestContext
    {
        std::function<void(::nakama::realtime::Envelope&)> successCallback;
        RtErrorCallback errorCallback;
    };

    /**
     * A real-time client to interact with Nakama server.
     * Don't use it directly, use `createRtClient` instead.
     */
    class NRtClient : public NRtClientInterface
    {
    public:
        NRtClient(NRtTransportPtr transport, const std::string& host, int32_t port, bool ssl);
        ~NRtClient();

        void tick() override;

        NRtTransportPtr getTransport() const override { return _transport; }
        void setListener(NRtClientListenerInterface* listener) override;

        void setUserData(void* userData) override { _userData = userData; }
        void* getUserData() const override { return _userData; }

        void setHeartbeatIntervalMs(opt::optional<int> ms) override {
            _heartbeatIntervalMs = ms;
        }

        opt::optional<int> getHeartbeatIntervalMs() override {
            return _heartbeatIntervalMs;
        }

        void connect(NSessionPtr session, bool createStatus, NRtClientProtocol protocol) override;

        bool isConnected() const override;

        void disconnect() override;

        void joinChat(
            const std::string& target,
            NChannelType type,
            const opt::optional<bool>& persistence = opt::nullopt,
            const opt::optional<bool>& hidden = opt::nullopt,
            std::function<void (NChannelPtr)> successCallback = nullptr,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void leaveChat(
            const std::string& channelId,
            std::function<void()> successCallback = nullptr,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void writeChatMessage(
            const std::string& channelId,
            const std::string& content,
            std::function<void(const NChannelMessageAck&)> successCallback = nullptr,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void updateChatMessage(
            const std::string& channelId,
            const std::string& messageId,
            const std::string& content,
            std::function<void(const NChannelMessageAck&)> successCallback = nullptr,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void removeChatMessage(
            const std::string& channelId,
            const std::string& messageId,
            std::function<void(const NChannelMessageAck&)> successCallback = nullptr,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void createMatch(
            std::function<void(const NMatch&)> successCallback,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void joinMatch(
            const std::string& matchId,
            const NStringMap& metadata,
            std::function<void(const NMatch&)> successCallback,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void joinMatchByToken(
            const std::string& token,
            std::function<void(const NMatch&)> successCallback,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void leaveMatch(
            const std::string& matchId,
            std::function<void()> successCallback = nullptr,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void addMatchmaker(
            const opt::optional<int32_t>& minCount = opt::nullopt,
            const opt::optional<int32_t>& maxCount = opt::nullopt,
            const opt::optional<std::string>& query = opt::nullopt,
            const NStringMap& stringProperties = {},
            const NStringDoubleMap& numericProperties = {},
            const opt::optional<int32_t>& countMultiple = opt::nullopt,
            std::function<void(const NMatchmakerTicket&)> successCallback = nullptr,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void removeMatchmaker(
            const std::string& ticket,
            std::function<void()> successCallback = nullptr,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void sendMatchData(
            const std::string& matchId,
            int64_t opCode,
            const NBytes& data,
            const std::vector<NUserPresence>& presences = {}
        ) override;

        void followUsers(
            const std::vector<std::string>& userIds,
            std::function<void(const NStatus&)> successCallback = nullptr,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void unfollowUsers(
            const std::vector<std::string>& userIds,
            std::function<void()> successCallback = nullptr,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void updateStatus(
            const std::string& status,
            std::function<void()> successCallback = nullptr,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void rpc(
            const std::string& id,
            const opt::optional<std::string>& payload = opt::nullopt,
            std::function<void(const NRpc&)> successCallback = nullptr,
            RtErrorCallback errorCallback = nullptr
        ) override;

        void acceptPartyMember(const std::string& partyId, NUserPresence& presence, std::function<void()> successCallback = nullptr, RtErrorCallback errorCallback = nullptr) override;

        void addMatchmakerParty(const std::string& partyId, const std::string& query, int minCount, int maxCount,
            const NStringMap& stringProperties = {}, const NStringDoubleMap& numericProperties = {},
            const opt::optional<int32_t>& countMultiple = opt::nullopt,
            std::function<void(const NPartyMatchmakerTicket&)> successCallback = nullptr, RtErrorCallback errorCallback = nullptr) override;

        void closeParty(const std::string& partyId, std::function<void()> successCallback = nullptr, RtErrorCallback errorCallback = nullptr) override;

        void createParty(bool open, int maxSize, std::function<void(const NParty&)> successCallback = nullptr, RtErrorCallback errorCallback = nullptr) override;

        void joinParty(const std::string& partyId, std::function<void()> successCallback = nullptr, RtErrorCallback errorCallback = nullptr) override;

        void leaveParty(const std::string& partyId, std::function<void()> successCallback = nullptr, RtErrorCallback errorCallback = nullptr) override;

        void listPartyJoinRequests(const std::string& partyId, std::function<void(const NPartyJoinRequest&)> successCallback = nullptr, RtErrorCallback errorCallback = nullptr) override;

        void promotePartyMember(const std::string& partyId, NUserPresence& partyMember, std::function<void()> successCallback = nullptr, RtErrorCallback errorCallback = nullptr) override;

        void removeMatchmakerParty(const std::string& partyId, const std::string& ticket, std::function<void()> successCallback = nullptr, RtErrorCallback errorCallback = nullptr) override;

        void removePartyMember(const std::string& partyId, NUserPresence& presence, std::function<void()> successCallback = nullptr, RtErrorCallback errorCallback = nullptr) override;

        void sendPartyData(const std::string& partyId, long opCode, NBytes& data) override;


        protected:
            void onTransportDisconnected(const NRtClientDisconnectInfo& info);
            void onTransportError(const std::string& description);
            void onTransportMessage(const NBytes& data);

            void reqInternalError(int32_t cid, const NRtError& error);

            std::shared_ptr<RtRequestContext> createReqContext(::nakama::realtime::Envelope& msg);
            void send(const ::nakama::realtime::Envelope& msg);

        private:
            // Even though Ping message is in Nakama public API, there is no use case to call it directly
            // other than to implement client-driven heartbeat, which we already have.
            void ping(std::function<void()> successCallback, RtErrorCallback errorCallback = nullptr);
            void heartbeat();
            void cancelAllRequests(RtErrorCode code);
            void disconnect(const NRtClientDisconnectInfo& info);

        private:
            std::string _host;
            int32_t _port = 0;
            bool _ssl = false;
            NRtClientListenerInterface* _listener = nullptr;
            NRtTransportPtr _transport;
            NRtClientProtocolPtr _protocol;
            std::map<int32_t, std::shared_ptr<RtRequestContext>> _reqContexts;
            std::mutex _reqContextsLock; //alow protects _nextCid
            int32_t _nextCid = 0;
            void* _userData = nullptr;

            NTimestamp _lastMessageTs = 0;   // last message sent. Used to decide whether to send heartbeat
            NTimestamp _lastHeartbeatTs = 0;  // last heartbeat. != 0 when we are waiting for the heartbeat response
            bool _heartbeatFailureReported = false;
            opt::optional<int> _heartbeatIntervalMs = 5000;
            std::atomic<bool> _wantDisconnect = false;

    };

}
