FROM debian:bookworm-slim AS build

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        libasio-dev \
        cmake \
        g++ \
        libpqxx-dev \
        libssl-dev \
        make \
        pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel 2

FROM debian:bookworm-slim AS runtime

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        libpqxx-6.4 \
        libssl3 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/build/BankingSystem /usr/local/bin/BankingSystem

EXPOSE 8080
CMD ["BankingSystem"]
