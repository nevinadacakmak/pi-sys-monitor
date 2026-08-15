# Pi System Monitor

A system monitor for the Raspberry Pi, written in C from scratch, using the standard library and POSIX sockets. It reads live metrics from `/proc` and `/sys`, serves them as JSON over a hand-built HTTP server, and runs as a Linux daemon.

The goal is to understand what's actually happening at the OS and protocol level: how the kernel exposes hardware state, how HTTP works as a raw byte protocol, and how a long-running C process manages its own lifecycle.

## What it does

- Reads CPU usage, memory usage, CPU core count, max CPU frequency, temperature, system uptime, and Minecraft server status directly from kernel interfaces (`/proc`, `/sys`) and process checks
- Serves these metrics as JSON over a tiny HTTP API, built directly on top of BSD sockets
- Runs as a daemon, detached from any terminal, surviving logout, logging via `syslog`
- Exposed publicly at [pi.nevinadacakmak.com](https://pi.nevinadacakmak.com) via Caddy and port forwarding

## Design

```
main.c        — daemonizing sequence, socket setup, accept loop, HTTP response writing
metrics.c/h   — pure functions that read /proc, /sys, and process state into a buffer
```

## Metrics

| Field | Source | Notes |
| --- | --- | --- |
| CPU usage (%) | `/proc/stat` | Delta between two snapshots: `(busy_delta / total_delta) * 100` |
| Memory usage | `/proc/meminfo` | Total minus available, in kB |
| CPU core count | `/proc/cpuinfo` | Counts `processor` entries — ARM-compatible, unlike x86 `cpu cores` field |
| Max CPU frequency | `/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq` | In kHz |
| Temperature | `/sys/class/thermal/thermal_zone0/temp` | Millidegrees Celsius ÷ 1000 |
| Uptime | `/proc/uptime` | Seconds since boot |
| Minecraft server status | `docker ps` via `popen` | Container check, not a kernel interface |

## Building

Requires `gcc` and `make`.

```
make sysmon
```

Produces a `sysmon` binary from `main.c` and `metrics.c`.

```
make clean
```

Removes build artifacts.

## Running

```
./sysmon
```

The process forks, daemonizes, and detaches from the terminal immediately. It listens on port `8080`.

To verify it's running:

```
curl http://localhost:8080/metrics
```

To follow logs:

```
journalctl -f -t pi-sys-monitor
```

To stop it:

```
kill $(pgrep sysmon)
```

## API

**`GET /metrics`**

Returns a JSON object with current metrics:

```json
{
  "delay": 1.00,
  "numOfCores": 4,
  "memoUtil": 7592204.0,
  "overallValue": 16351168.0,
  "maxFreq": 2400000,
  "cpuValue": 1.82,
  "temperature": 48.00,
  "upTime": 2332800.0,
  "McServer": "Up 4 months (healthy)"
}
```

Raw values, unit conversion and display formatting is left to the frontend.

**`GET /`** (anything else)

Returns `404 Not Found`.

## Daemon behaviour

On startup, `sysmon`:

1. `fork()` — parent exits, child is re-parented to `init`/`systemd`
2. `setsid()` — child creates a new session, detaches from controlling terminal
3. Redirects `stdin`/`stdout`/`stderr` to `/dev/null`
4. `openlog()` — all logging goes to `syslog` from here
5. `chdir("/")` — releases reference to launch directory
6. Registers `SIGTERM` and `SIGINT` handlers
7. Sets up and enters the socket accept loop


## Status

- [x] Core metrics: CPU, memory, cores, max frequency, temperature
- [x] Extended metrics: uptime, Minecraft server status
- [x] HTTP server serving JSON over raw sockets
- [x] Daemon: fork/setsid, signal handling, syslog
- [x] Self-hosted on Pi with Caddy, publicly accessible over HTTPS
- [x] Browser dashboard at pi.nevinadacakmak.com

## References

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) — socket programming
- [RFC 7230](https://datatracker.ietf.org/doc/html/rfc7230) / MDN — HTTP/1.1 message format
- Stevens, *Advanced Programming in the UNIX Environment* — daemon architecture
- `man 2 fork`, `man 2 setsid`, `man 2 sigaction`, `man 3 syslog`