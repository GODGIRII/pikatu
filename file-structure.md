# audio-bridge — file structure

Monorepo with three packages: C pc-client, Kotlin android-app, Node.js server.

---

```
audio-bridge/
│
├── pc-client/                          ← C application (Windows & Mac)
│   ├── src/
│   │   ├── main.c                      ← entry point, CLI args, main loop
│   │   │
│   │   ├── audio/
│   │   │   ├── capture.c               ← WASAPI loopback (Win) / CoreAudio (Mac)
│   │   │   └── capture.h
│   │   │
│   │   ├── codec/
│   │   │   ├── encoder.c               ← Opus encode wrapper (10ms frames, 160kbps)
│   │   │   └── encoder.h
│   │   │
│   │   ├── network/
│   │   │   ├── udp_sender.c            ← non-blocking UDP socket, SO_SNDBUF tuning
│   │   │   ├── udp_sender.h
│   │   │   ├── sequencer.c             ← stamps each packet: seq num + timestamp
│   │   │   └── sequencer.h
│   │   │
│   │   └── session/
│   │       ├── signaling.c             ← HTTP client for pairing handshake
│   │       ├── signaling.h
│   │       ├── peer.c                  ← holds peer IP, port, session token
│   │       └── peer.h
│   │
│   ├── include/
│   │   └── audio_bridge.h              ← shared constants (packet format, magic bytes, ports)
│   │
│   ├── lib/
│   │   └── opus/                       ← libopus (git submodule or prebuilt .a)
│   │
│   ├── tests/
│   │   ├── test_encoder.c              ← encode → decode round-trip check
│   │   └── test_sequencer.c            ← seq number rollover, timestamp drift
│   │
│   ├── CMakeLists.txt                  ← links libopus, platform audio libs
│   └── README.md
│
│
├── android-app/                        ← Kotlin Android app
│   ├── app/
│   │   ├── src/main/
│   │   │   │
│   │   │   ├── java/com/audiobridge/
│   │   │   │   ├── MainActivity.kt             ← QR scan / code entry, start/stop
│   │   │   │   │
│   │   │   │   ├── audio/
│   │   │   │   │   ├── AudioPlayer.kt          ← AudioTrack in streaming mode
│   │   │   │   │   └── JitterBuffer.kt         ← reorder packets, smooth playback
│   │   │   │   │
│   │   │   │   ├── codec/
│   │   │   │   │   └── OpusDecoder.kt          ← JNI bridge to native libopus
│   │   │   │   │
│   │   │   │   ├── network/
│   │   │   │   │   ├── UdpReceiver.kt          ← coroutine-based UDP receive loop
│   │   │   │   │   └── PacketParser.kt         ← strips header, validates seq num
│   │   │   │   │
│   │   │   │   └── session/
│   │   │   │       ├── SignalingClient.kt      ← WebSocket pairing with server
│   │   │   │       └── SessionManager.kt       ← peer IP, token, connection state
│   │   │   │
│   │   │   ├── cpp/
│   │   │   │   ├── CMakeLists.txt              ← NDK build: links libopus.a
│   │   │   │   └── opus_jni.cpp                ← JNI wrapper (decode, init, destroy)
│   │   │   │
│   │   │   ├── res/
│   │   │   │   ├── layout/
│   │   │   │   │   └── activity_main.xml       ← connect button, status, volume
│   │   │   │   └── values/
│   │   │   │       └── strings.xml
│   │   │   │
│   │   │   └── AndroidManifest.xml             ← INTERNET + CHANGE_WIFI_MULTICAST_STATE
│   │   │
│   │   └── build.gradle                        ← externalNativeBuild, abiFilters
│   │
│   ├── build.gradle
│   └── gradle.properties
│
│
├── server/                             ← Node.js (signaling + UDP relay)
│   ├── src/
│   │   ├── index.js                    ← starts both servers, shared config
│   │   │
│   │   ├── signaling/
│   │   │   ├── server.js               ← WebSocket server (ws library)
│   │   │   ├── pairingCodes.js         ← generates & validates 6-digit codes
│   │   │   └── sessionStore.js         ← in-memory session map (or Redis)
│   │   │
│   │   └── relay/
│   │       ├── udpRelay.js             ← dgram UDP relay, routes by session token
│   │       └── natTraversal.js         ← UDP hole-punching logic
│   │
│   ├── config/
│   │   └── default.json                ← ports, session TTL, max concurrent sessions
│   │
│   ├── tests/
│   │   ├── signaling.test.js
│   │   └── relay.test.js
│   │
│   ├── package.json
│   └── Dockerfile                      ← for deploying relay to a VPS
│
│
├── shared/
│   └── protocol.md                     ← packet format spec (the truth for all three sides)
│
├── .gitignore
├── .gitmodules                         ← libopus as a submodule
└── README.md
```

---

## Key design notes

### `shared/protocol.md` — the most important file
All three codebases must agree on the packet binary format.
Define it once here. Example layout:

```
| Offset | Size | Field         |
|--------|------|---------------|
| 0      | 4    | Magic bytes   |  0xAB 0xCD 0xEF 0x01
| 4      | 4    | Sequence num  |  uint32, big-endian
| 8      | 8    | Timestamp µs  |  uint64, big-endian
| 16     | 1    | Flags         |  bit 0 = FEC, bit 1 = last fragment
| 17     | 2    | Payload len   |  uint16
| 19     | N    | Opus payload  |  encoded audio frame
```

### Platform split in `pc-client/audio/`
`capture.c` should use `#ifdef _WIN32` to switch between WASAPI and CoreAudio.
The encoder, sequencer, and network layers are fully platform-agnostic.

### JNI in `android-app/cpp/`
`opus_jni.cpp` is the only C++ file in the Android project.
Keep it thin — just allocate/free the decoder and call `opus_decode`.
All buffer management and threading stays in Kotlin (`OpusDecoder.kt`).

### Server separation
The signaling server (WebSocket, port 443) and relay server (UDP, port 9000)
are separate modules but share the same Node.js process via `index.js`.
For scale-out, split them into separate processes behind a load balancer.
