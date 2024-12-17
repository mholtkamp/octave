# TimerManager

A system that can be used to set timers which will invoke a given callback function after their duration has elapsed.

---
### SetTimer
Set a timer that will invoke a callback function after its duration has elapsed. This function will return an ID that can be used to pause/resume/clear the timer later.

Sig: `id = TimerManager.SetTimer(func, time, loop=false)`
 - Arg: `function func` Callback function
 - Arg: `number time` Time in seconds
 - Arg: `boolean loop` Loop timer
 - Ret: `integer id` Timer ID
---
### ClearAllTimers
Clear all timers.

Sig: `TimerManager.ClearAllTimers()`

---
### ClearTimer
Clear a specific timer.

Sig: `TimerManager.ClearTimer(id)`
 - Arg: `integer id` Timer ID
---
### PauseTimer
Pause a timer.

Sig: `TimerManager.PauseTimer(id)`
 - Arg: `integer id` Timer ID
---
### ResumeTimer
Resume a timer that was previously paused.

Sig: `TimerManager.ResumeTimer(id)`
 - Arg: `integer id` Timer ID
---
### ResetTimer
Reset a timer, restoring its time remaining to the initial time given.

Sig: `TimerManager.ResetTimer(id)`
 - Arg: `integer id` Timer ID
---
### GetTimeRemaining
Get the time remaining on a timer.

Sig: `time = TimerManager.GetTimeRemaining(id)`
 - Arg: `integer id` Timer ID
 - Ret: `number time` Time remaining in seconds
---
