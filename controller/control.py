"""Gesture position, motor commands and movement stability filtering."""

PWM = 90
STABILITY_SECONDS = 0.25
MOTOR_COMMANDS = {
    "STOP": (0, 0), "FORWARD": (PWM, PWM), "BACKWARD": (-PWM, -PWM),
    "LEFT": (-PWM, PWM), "RIGHT": (PWM, -PWM),
}


def command_from_position(x, y):
    """Map normalized mirrored coordinates to a direction; center and corners stop."""
    if 0.35 <= x <= 0.65:
        if y < 0.35:
            return "FORWARD"
        if y > 0.65:
            return "BACKWARD"
    if 0.35 <= y <= 0.65:
        if x < 0.35:
            return "LEFT"
        if x > 0.65:
            return "RIGHT"
    return "STOP"


class CommandFilter:
    """Stop immediately; accept movement after 250 ms of stability. Stop while changing direction."""
    def __init__(self):
        self.candidate = "STOP"
        self.since = 0.0

    def update(self, command, now):
        if command == "STOP":
            self.candidate = "STOP"
            self.since = now
            return "STOP"
        if command != self.candidate:
            self.candidate, self.since = command, now
        return command if now - self.since >= STABILITY_SECONDS else "STOP"
