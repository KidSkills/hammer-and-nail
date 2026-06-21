# useWebSocket Lifecycle

`useWebSocket` owns exactly one live socket per mounted hook instance. On
unmount, manual disconnect, or remount cleanup, the hook clears reconnect,
ping, and pong timers before detaching socket handlers and closing the socket.

Expected lifecycle harness:

1. Mount a component with `autoConnect: true`, `reconnect: true`, and
   `debug: true`.
2. Force a socket close and confirm only one reconnect timer is scheduled.
3. Unmount before the reconnect delay elapses and confirm no reconnect occurs.
4. Remount the component and confirm the new instance creates one socket and
   does not reuse the previous instance's timers or event handlers.

Cleanup diagnostics are written only when `debug` is enabled, so production
builds do not emit lifecycle noise.
