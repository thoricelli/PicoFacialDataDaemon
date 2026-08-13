# PicoFacialDataDaemon

A daemon that exposes an UDP socket that provides eye and face tracking data.  
To use this with VRCFaceTracking, download the module here: https://github.com/thoricelli/PicoFacialDataModule/releases

## Running

### Magisk
1. Download the ZIP file
2. Install via Magisk

### Standalone
This step does not require root.

1. Download the binary
2. `adb push picofacialdatadaemon /data/local/tmp`
3. `adb shell chmod +x /data/local/tmp/picofacialdatadaemon`
4. `adb shell ./data/local/tmp/picofacialdatadaemon`
5. It will notify you what PID it started on.
6. To kill it: `pkill -f picofacialdatadaemon`

## How this works
Face and eye tracking is usually either:
- Polled via the Pico XR API
- or polled via the Unity / Unreal API (these are just bridges to the XR API).

The XR API, then talks to the available running services on the Pico OS.  
These services are documented on my [PICO documentation](https://github.com/thoricelli/PICO-documentation/tree/main/services#pxreyetrackingservice).

This daemon bypasses that, and talks directly to the `pxreyetrackingservice`, this service exposes a transaction: `GetTrackingDataSharedMemory`.  
This transaction returns a file descriptor which can be mapped to use as shared memory.

On startup:
1. The daemon waits for a discovery request on the multicast group: `239.255.255.250` with port `9030`.
2. Once it receives `DISCOVER_DAEMON`, the daemon will call the `pxreyetrackingservice` and start the tracking algorithm.
3. If the device is asleep, it polls until starting the algorithm succeeds.
4. Once established it will start sending eye and face tracking data to the client that sent the discovery request, via UDP on port `9030`.  
The polling interval is 10hz, the rate that the tracking service sends data back is 25hz.
5. The daemon will periodically (every 25 seconds) send pings to the client with the message: `MARCO`, the client must respond with `POLO` within 25 seconds.
6. If the client is dead, the daemon will stop the tracking algorithm (which closes the camera if there are no clients).  
If a `STOP` message is sent, the daemon will immediately stop the tracking algorithm and go back to the discovery process.

The daemon uses about 0.3% - 0.6% of the CPU when sending data, 4.2 MB of memory, and 0% of the CPU when idle.

## Building

1. Install the Android NDK, version: r21e (21.4.7075529).   
https://developer.android.com/ndk/downloads
2. Add the root path `ANDROID_NDK_ROOT` to your environment variables.
3. 
```bash
git submodule update --init --recursive --remote
```
4. Install `make`, `cmake` and `ninja`
5. Run `make`.
