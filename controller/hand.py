"""Hand geometry used by the MediaPipe recognition pipeline."""

import math

HAND_CONNECTIONS = (
    (0, 1), (1, 2), (2, 3), (3, 4), (0, 5), (5, 6), (6, 7), (7, 8),
    (5, 9), (9, 10), (10, 11), (11, 12), (9, 13), (13, 14),
    (14, 15), (15, 16), (13, 17), (0, 17), (17, 18), (18, 19), (19, 20),
)


def is_hand_open(landmarks, width, height):
    """Require four extended fingers, excluding the thumb. This geometric heuristic can reject tilted or occluded hands. Pixel distances account for image aspect ratio."""
    def distance(a, b):
        return math.hypot((landmarks[a].x - landmarks[b].x) * width,
                          (landmarks[a].y - landmarks[b].y) * height)

    for base, joint, tip in ((5, 6, 8), (9, 10, 12), (13, 14, 16), (17, 18, 20)):
        sides = distance(base, joint) + distance(joint, tip)
        if sides < 1 or distance(base, tip) / sides < 0.90:
            return False
        if distance(0, tip) < distance(0, joint) * 1.15:
            return False
    return True


def create_detector(model_path):
    """Create a video hand detector; two detected hands cause the caller to stop."""
    from mediapipe.tasks import python
    from mediapipe.tasks.python import vision

    options = vision.HandLandmarkerOptions(
        base_options=python.BaseOptions(model_asset_path=str(model_path)),
        running_mode=vision.RunningMode.VIDEO,
        num_hands=2,
        min_hand_detection_confidence=0.7,
        min_hand_presence_confidence=0.7,
        min_tracking_confidence=0.7,
    )
    return vision.HandLandmarker.create_from_options(options)


def detect_frame(detector, rgb_frame, timestamp_ms):
    """Recognize landmarks in an RGB frame using an increasing timestamp."""
    import mediapipe as mp

    image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb_frame)
    return detector.detect_for_video(image, timestamp_ms)
