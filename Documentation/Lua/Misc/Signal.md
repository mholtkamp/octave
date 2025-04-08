# Signal

A signal is an object that provides a sort of event-driven behavior. When a signal is emitted, any nodes that are connected to the signal will be able to react.

---
### Create
Create a new signal.

Sig: `signal = Signal.Create()`
 - Ret: `Signal signal` Newly created Signal
 ---
### Emit
Emit the signal so that connected listeners can react.

Sig: `Signal:Emit(args...)`
 - Arg: `args...` Any number of arguments that will be passed to connected handler functions.
---
### Connect 
Connect a node to the signal. The function passed in should be a member function of the node's table.
For instance:

```lua
function HealthBar:OnHealthChanged(newHp)
    self.quad:SetXRatio(newHp / Player.MaxHp)
end

function HealthBar:Start()
    GetPlayer().healthSignal:Connect(self, HealthBar.OnHealthChanged)
end
```

Sig: `Signal:Connect(listener, func)`
 - Arg: `Node listener` The node that will be notified when the signal is emit
 - Arg: `function func` The function on the node that will be invoked when the signal is emit
 ---
 ### Disconnect
 Sig: `Signal:Disconnect(listener)`
  - Arg: `Node listener` The node that will be disconnected from the signal
---
