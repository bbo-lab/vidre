# VidRe microservices — Docker / WSL testing

Run the camera and storage microservices in containers (WSL2 or Linux), then connect a
native GUI (or another container) to the camera service.

## Quick start

```bash
cd vidre/docker
docker compose up --build
```

- `storage` — `vidre_storage_service` on port 50053, writes CCV files into `./data`.
- `camera` — `vidre_camera_service` with a dummy camera, control on port 50051, preview
  on port 50052, autostarts recording and streams frames to the storage service.

The camera service config is `docker/config/camera.yaml` (mounted read-only).

## Connect the GUI

The camera service ports are published to the host:

```bash
vidre_app --connect localhost:50051
```

Open the camera window (double-click in the device list) to request preview; Record/Stop
are sent to the camera service over the control channel.

## Checking output

The storage service writes timestamped CCV files into `./data` (on the host):

```bash
ls docker/data
```

The storage service container logs "Storage session opened/closed" with frame counts.

## Stop / clean

```bash
docker compose down
```

## Notes

- Only the dummy camera is available in the container build; Pylon/Aptina are optional in
  CMake (`find_package(pylon ... QUIET)`), so the Linux build skips them.
- The full pipeline is three independent processes: camera service (cameras + ring),
  storage service (CCV writing), and the app (control + preview). Each can run on a
  different machine; the endpoints are configurable via CLI flags.
