# `Component deepSleep`

Two times need to be set in `menuconfig`:

## Wake up time:
- `START_HOUR` is the wake up hour
- `START_MINUTE` is the wake up minute.

## Sleep time:
- `FINAL_HOUR` is the sleep hour.
- `FINAL_MINUTE` is the sleep minute.

*The hours are in 24-hour format.*

By default the wake up time is **8:00** and the sleep time is **22:00**.

SIf time is not synchronized in the ESP by default it will be 14 hours awake and 10 hours in deep sleep.
