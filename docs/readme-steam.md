## Steam

To build the game inside the Steam Runtime, you can use the provided Docker Compose file to do so. It will build the game inside the latest Sniper runtime.

**Note:** Using rootful Docker will create a lot of permission errors if you switch back to using local builds, so using rootless Podman is recommended.


```bash
$ docker-compose run steam-x64  # Build 64-bit binary
$ docker-compose run steam-x86  # Build 32-bit binary
```

The binary is located in `build/steam-xAA/endless-sky`, where `AA` is the architecture from above.
