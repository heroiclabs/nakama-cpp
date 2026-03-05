ARG NAKAMA_VERSION=3.37.0

FROM registry.heroiclabs.com/heroiclabs/nakama-pluginbuilder:${NAKAMA_VERSION} AS builder

ENV GO111MODULE=on
ENV CGO_ENABLED=1

WORKDIR /backend
COPY . .

RUN go mod tidy && go build --trimpath --buildmode=plugin -o ./backend.so

FROM registry.heroiclabs.com/heroiclabs/nakama:${NAKAMA_VERSION}

COPY --from=builder /backend/backend.so /nakama/data/modules/
