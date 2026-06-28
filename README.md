# Pi System Monitor

A system monitor for the Raspberry Pi, written in C from scratch, using the standard library and POSIX sockets. It reads live metrics from `/proc` and `/sys`, and serves them as JSON over a hand-built HTTP server.

The goal is to understand what's actually happening at the OS and protocol level: how the kernel exposes hardware state, how HTTP works as a raw byte protocol, and how a long-running C process manages itself.

## What it does

- Reads CPU usage, memory usage, CPU core count, max CPU frequency, temperature, system uptime, and my Minecraft server status directly from kernel interfaces (`/proc`, `/sys`) and process checks
- Serves these metrics as JSON over a tiny HTTP API, built directly on top of BSD sockets
- Designed to be exposed publicly via a Cloudflare tunnel, so the Pi's stats are checkable from anywhere

## Architecture

```
Browser / curl  --HTTP GET-->  C server (this project)  -->  /proc, /sys, docker ps
                <--JSON--------
```

```
main.c        — socket setup (socket/bind/listen), accept loop, HTTP response writing
metrics.c/h   — pure functions that read /proc, /sys, and process state and return values
```

## Metrics

| Field | Source | Notes |
|---|---|---|
| CPU usage (%) | `/proc/stat` | Computed from the delta between two snapshots of cumulative CPU time (`(busy_delta / total_delta) * 100`), not a single instantaneous read |
| Memory usage | `/proc/meminfo` | Total minus available, reported in kB by the kernel |
| CPU core count | `/proc/cpuinfo` | Counted from CPU entries |
| Max CPU frequency | `/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq` | In kHz, as reported by the kernel |
| Temperature | `/sys/class/thermal/thermal_zone0/temp` | Reported in millidegrees Celsius, divided by 1000 |
| Uptime | `/proc/uptime` | Seconds since boot |
| Minecraft server status | `docker ps` via `popen` | Not kernel-exposed data — checked as a running process/container, not a `/proc` read |

## Building

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

Starts the server listening on port `8080`. Hit it with:

```
curl http://localhost:8080
```

## API

**`GET /`**

Returns a JSON object with the current system metrics, e.g.:

```json
{
  "cpu_usage": 23.4,
  "memory_used_kb": 871200,
  "memory_total_kb": 3942180,
  "cores": 4,
  "max_freq_khz": 1800000,
  "temperature_c": 51.2,
  "uptime_s": 184221,
  "minecraft_running": true
}
```

Raw values are sent as-is; unit conversion and formatting (percentages, GB display, uptime as `Xd Yh`) is left to the frontend, not the C server.

## Exposing it publicly

The server only binds locally — it doesn't handle TLS or public exposure itself. A [Cloudflare Tunnel](https://developers.cloudflare.com/cloudflare-one/connections/connect-networks/) run alongside it maps a public domain to `localhost:8080`, so the Pi's HTTPS termination is handled by Cloudflare.

## Status / roadmap

- [x] Core metrics: CPU, memory, cores, max frequency, temperature
- [x] Extended metrics: uptime, Minecraft server status
- [x] HTTP server serving JSON over raw sockets
- [ ] Daemonize (background process, clean signal handling for `SIGTERM`/`SIGINT`)
- [ ] Cloudflare tunnel deployment
- [ ] Browser-based frontend dashboard (plain HTML/JS polling the API)

## Tools & references used

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) — socket programming
- [RFC 7230](https://datatracker.ietf.org/doc/html/rfc7230) / MDN — HTTP/1.1 message format
- Stevens, *Advanced Programming in the UNIX Environment* — daemon architecture (upcoming)