# Test Plan — DDAWS Prototype

Manual test procedure for the laptop-webcam prototype. Enable **developer mode**
(`d` key or `--dev`) to see EAR/PERCLOS/MAR/pose/confidence/state while testing.
Record results in the table at the bottom. Warning latency = time from the
stimulus to the on-screen/audible warning.

## Setup
- Sit at arm's length, face lit from the front, camera at eye level.
- Run once and complete the 3-second calibration (or press `c` to redo).
- Tune `config/config.json` if thresholds don't match your setup (all documented
  in the README + `Config.hpp`). Thresholds are **prototype values**, not
  official AIS-184 limits.

## Test cases

| # | Scenario | Expected result |
|---|---|---|
| T01 | Normal alert driver | ATTENTIVE / SAFE; EAR steady above threshold |
| T02 | Normal blinking | blinks counted; **no** drowsiness escalation |
| T03 | Single long blink | POSSIBLE_DROWSINESS at most; no CRITICAL |
| T04 | Prolonged eye closure (~1.5 s) | DROWSY; audible warning |
| T05 | Repeated eye closure | PERCLOS rises → DROWSY/CRITICAL |
| T06 | Yawn (mouth open ~1 s) | yawn counter +1; contributes to drowsiness |
| T07 | Repeated yawning | multiple yawns; elevated drowsiness |
| T08 | Look left briefly | no sustained warning (temporal filter) |
| T09 | Look right briefly | no sustained warning |
| T10 | Look away sustained (>2 s) | DISTRACTED / attention warning |
| T11 | Look down (phone posture) | attention warning; head dir = down |
| T12 | Driver leaves frame | "DRIVER NOT DETECTED" after grace period |
| T13 | Momentary face loss (<1 s) | no false "absent" (grace) |
| T14 | Two faces in frame | primary driver tracked; "multiple faces" shown |
| T15 | Low light | "MONITORING QUALITY LOW"; **no** false DROWSY |
| T16 | Glasses | landmarks still track; EAR usable |
| T17 | Sunglasses | expected low confidence / quality-low (eyes occluded) |
| T18 | Different head positions | pose classified; stable |
| T19 | Different users | calibration adapts EAR baseline |
| T20 | Different webcam resolutions | runs; adjust `capture_width/height` |
| T21 | Camera disconnected | clean error message, no crash |
| T22 | Missing/!invalid model file | falls back (Haar) with warning, no crash |
| T23 | Low-confidence landmarks (extreme angle) | quality-low, drowsiness suppressed |

## Record sheet

| # | Expected | Actual | Pass/Fail | False Pos | False Neg | Warning latency (s) |
|---|---|---|---|---|---|---|
| T01 | | | | | | |
| T02 | | | | | | |
| … | | | | | | |

Repeat for T01–T23. Note the camera/lighting/user for reproducibility.
