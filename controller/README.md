# WallE PC simulation

## Structure

- `main.py`: startup, webcam acquisition and application loop.
- `hand.py`: MediaPipe detector and open-hand geometry.
- `control.py`: position-to-command mapping, motor values and stability filter.
- `interface.py`: camera overlay and simulated motor indicators.
- `requirements.txt`: Python dependencies.
- `models/hand_landmarker.task`: local Google MediaPipe model.
- `env/env/`: existing virtual environment; left in place.

## Run on Windows PowerShell

From the `WallE/pc` directory, activate the existing environment:

```powershell
.\env\env\Scripts\Activate.ps1
python -m pip install -r requirements.txt
python main.py --camera 1
```

If activation is blocked, use the environment interpreter directly:

```powershell
.\env\env\Scripts\python.exe main.py --camera 1
```

Camera indices depend on Windows device ordering. Index 0 previously opened
an iVCam waiting screen on this PC. Try 1 or 2 for the physical webcam;
the program does not automatically identify camera names.
Close other camera applications if the device cannot be opened.

Install only `opencv-contrib-python`, not additional OpenCV or headless
packages in the same environment, to avoid conflicting `cv2` installations.

## Controls

With the application window focused:

- A: enable simulated movement.
- Space: disable and stop; press A to enable again.
- Q or Escape: quit and release the camera.
- One open hand in the top/bottom/left/right zone: corresponding direction.
- Center, corners, missing hand, two detected hands or folded fingers: stop.

The yellow circle marks the palm center in the mirrored image. Keep the
whole hand visible, palm facing the camera. All four long fingers must be
extended; the thumb is ignored. This geometric heuristic needs real-camera
validation and can reject tilted or occluded hands.

New directions require 250 ms of stability. While changing direction the
output is STOP. Stop has no intentional delay. Values are simulated PWM,
not measured speed or a physical rover simulation:

| Command | Left | Right |
| --- | ---: | ---: |
| STOP | 0 | 0 |
| FORWARD | 90 | 90 |
| BACKWARD | -90 | -90 |
| LEFT | -90 | 90 |
| RIGHT | 90 | -90 |

No Wi-Fi commands are sent and no video is recorded or transmitted.
The ESP32 sketch is separate and unchanged.

## Model

The model is already stored in `models/hand_landmarker.task`. If missing,
download it from this directory:

```powershell
New-Item -ItemType Directory -Force models
Invoke-WebRequest -Uri 'https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/latest/hand_landmarker.task' -OutFile 'models/hand_landmarker.task'
```

Recognition runs locally after installation and model download.
Reference: https://ai.google.dev/edge/mediapipe/solutions/vision/hand_landmarker/python

## Manual check

1. Start: both motor values must be zero.
2. Press A and hold the palm in each zone for more than 250 ms.
3. Verify motor signs against the table above.
4. Close or hide the hand: both values must become zero.
5. Press Space while moving: remain stopped until A is pressed again.
6. Quit and verify that the camera is released.
