"""WallE webcam and simulated motors. No commands are sent to the rover.

Run: python main.py --camera 0
A enables; SPACE disables; Q or ESC exits.
"""

import argparse
from pathlib import Path
import time

from control import CommandFilter, command_from_position
from hand import create_detector, detect_frame, is_hand_open
from interface import draw_interface

MODEL_PATH = Path(__file__).resolve().parent / "models" / "hand_landmarker.task"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--camera", type=int, default=0, help="Camera index (default: 0)")
    args = parser.parse_args()
    # Lazy imports keep --help available without third-party dependencies.
    try:
        import cv2
        import mediapipe  # Check the dependency before opening the camera.
    except ImportError as error:
        print(f"Missing dependency: {error}\nRun: python -m pip install -r requirements.txt")
        return 1
    if not MODEL_PATH.is_file():
        print(f"Missing model: {MODEL_PATH}\nSee the Model section in README.md.")
        return 1

    camera = cv2.VideoCapture(args.camera)
    window = "WallE - simulation"
    try:
        if not camera.isOpened():
            raise RuntimeError("Camera unavailable. Close other apps or try --camera 1.")
        camera.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
        command_filter = CommandFilter()
        enabled = False
        timestamp = -1
        cv2.namedWindow(window, cv2.WINDOW_AUTOSIZE)
        with create_detector(MODEL_PATH) as detector:
            while True:
                ok, frame = camera.read()
                if not ok:
                    raise RuntimeError("Camera stream interrupted: simulation stopped.")
                frame = cv2.flip(frame, 1)
                rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                # MediaPipe VIDEO requires strictly increasing timestamps.
                now = time.monotonic()
                timestamp = max(timestamp + 1, int(now * 1000))
                result = detect_frame(detector, rgb, timestamp)
                landmarks = None
                requested = "STOP"
                reason = "Show exactly one open hand to the camera"
                if len(result.hand_landmarks) == 1:
                    landmarks = result.hand_landmarks[0]
                    h, w = frame.shape[:2]
                    if is_hand_open(landmarks, w, h):
                        x = sum(landmarks[i].x for i in (0, 5, 9, 13, 17)) / 5
                        y = sum(landmarks[i].y for i in (0, 5, 9, 13, 17)) / 5
                        requested = command_from_position(x, y)
                        reason = f"Open hand | zone: {requested}"
                    else:
                        reason = "Hand not open: STOP"
                if not enabled:
                    requested = "STOP"
                    reason = "Press A to enable the simulation"
                command = command_filter.update(requested, now)
                cv2.imshow(window, draw_interface(cv2, frame, landmarks, command, reason, enabled))
                key = cv2.waitKey(1) & 0xFF
                if key in (27, ord("q"), ord("Q")):
                    break
                if cv2.getWindowProperty(window, cv2.WND_PROP_VISIBLE) < 1:
                    break
                if key == 32:
                    enabled = False
                    command_filter.update("STOP", time.monotonic())
                elif key in (ord("a"), ord("A")):
                    enabled = True
    except (RuntimeError, ValueError, OSError, cv2.error) as error:
        print(f"Error: {error}")
        return 1
    except KeyboardInterrupt:
        pass
    finally:
        camera.release()
        cv2.destroyAllWindows()
        print("STOP: simulation closed, camera released.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
