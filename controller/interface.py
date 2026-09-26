"""Webcam overlay and simulated motor display."""

from control import MOTOR_COMMANDS
from hand import HAND_CONNECTIONS


def draw_interface(cv2, frame, landmarks, command, reason, enabled):
    """Draw hand landmarks, control zones and two simulated PWM indicators."""
    h, w = frame.shape[:2]

    def text(img, caption, x, y, color=(230, 230, 230), scale=0.55):
        cv2.putText(img, caption, (x, y), cv2.FONT_HERSHEY_SIMPLEX,
                    scale, color, 1, cv2.LINE_AA)

    for fraction in (0.35, 0.65):
        cv2.line(frame, (int(w * fraction), 0), (int(w * fraction), h), (130, 130, 130), 1)
        cv2.line(frame, (0, int(h * fraction)), (w, int(h * fraction)), (130, 130, 130), 1)
    for label, x, y in (("FORWARD", .40, .12), ("BACKWARD", .40, .90),
                        ("LEFT", .03, .50), ("RIGHT", .72, .50), ("STOP", .44, .50)):
        text(frame, label, int(x * w), int(y * h), (0, 240, 255))
    if landmarks:
        pixels = [(int(p.x * w), int(p.y * h)) for p in landmarks]
        for a, b in HAND_CONNECTIONS:
            cv2.line(frame, pixels[a], pixels[b], (100, 230, 100), 2)
        for p in pixels:
            cv2.circle(frame, p, 3, (255, 180, 30), -1)
        cx = int(sum(landmarks[i].x for i in (0, 5, 9, 13, 17)) / 5 * w)
        cy = int(sum(landmarks[i].y for i in (0, 5, 9, 13, 17)) / 5 * h)
        cv2.circle(frame, (cx, cy), 9, (0, 240, 255), 2)

    canvas = cv2.copyMakeBorder(frame, 0, 190, 0, 0, cv2.BORDER_CONSTANT, value=(25, 25, 25))
    status = "ENABLED" if enabled else "DISABLED"
    text(canvas, f"SIMULATION | {status} | {command}", 15, h + 27)
    text(canvas, reason, 15, h + 52)
    for index, (name, value) in enumerate(zip(("LEFT", "RIGHT"), MOTOR_COMMANDS[command])):
        x = 15 + index * (w // 2)
        text(canvas, f"{name}: {value:+d} / 255", x, h + 82)
        center = x + (w // 2 - 35) // 2
        half = (w // 2 - 35) // 2
        cv2.line(canvas, (center - half, h + 108), (center + half, h + 108), (90, 90, 90), 8)
        if value:
            cv2.line(canvas, (center, h + 108), (center + int(half * value / 255), h + 108), (100, 230, 100), 8)
        cv2.line(canvas, (center, h + 98), (center, h + 118), (255, 255, 255), 1)
    text(canvas, "A: enable | SPACE: stop | Q / ESC: exit", 15, h + 148)
    text(canvas, "Simulated PWM: motors are not connected", 15, h + 173)
    return canvas
