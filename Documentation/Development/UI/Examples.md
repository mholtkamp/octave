# XML UI Examples

## Example 1: Simple HUD

**`UI/HUD.xml`:**
```xml
<ui>
  <style>
    .hud-root { anchor: full-stretch; }
    .health-bar {
      anchor: top-left;
      width: 200px;
      height: 24px;
      left: 16px;
      top: 16px;
      background-color: #cc2222;
    }
    .health-bg {
      anchor: full-stretch;
      background-color: #333333;
    }
    .score-text {
      anchor: top-right;
      color: #ffffff;
      font-size: 20px;
      left: -16px;
      top: 16px;
      text-align: right;
    }
    .ammo-text {
      anchor: bottom-right;
      color: #ffcc00;
      font-size: 28px;
      left: -16px;
      top: -16px;
    }
  </style>

  <div id="hud" class="hud-root">
    <!-- Health bar (bg + fill) -->
    <div class="health-bar">
      <img class="health-bg" />
      <img id="health-fill" style="anchor: left-stretch; background-color: #22cc22" />
    </div>

    <text id="score" class="score-text">Score: {{score}}</text>
    <text id="ammo" class="ammo-text">{{ammo}} / {{maxAmmo}}</text>
  </div>
</ui>
```

**Lua (attach to a Canvas widget in the scene):**
```lua
SimpleHUD = {}

function SimpleHUD:Start()
    self.hud = UI.Load("UI/HUD.xml")
    self.hud:Mount(self)
end

function SimpleHUD:Tick(deltaTime)
    self.hud:Tick()

    self.hud:SetData("score", tostring(playerScore))
    self.hud:SetData("ammo", tostring(currentAmmo))
    self.hud:SetData("maxAmmo", tostring(maxAmmo))

    -- Update health bar width (0-100%)
    local healthFill = self.hud:FindById("health-fill")
    if healthFill then
        local pct = currentHealth / maxHealth
        healthFill:SetWidthRatio(pct)
    end
end

function SimpleHUD:Stop()
    self.hud:Unmount()
    self.hud:Destroy()
end
```

---

## Example 2: Main Menu with Navigation

**`UI/MainMenu.xml`:**
```xml
<ui>
  <link href="UI/theme.css" />

  <div id="menu-root" class="fullscreen">
    <img class="bg-image" src="T_MenuBackground" />

    <text class="title-text">My Game</text>

    <flex class="menu-buttons">
      <button id="btn-play" class="menu-btn" on-click="OnPlay">Play</button>
      <button id="btn-options" class="menu-btn" on-click="OnOptions">Options</button>
      <button id="btn-credits" class="menu-btn" on-click="OnCredits">Credits</button>
      <button id="btn-quit" class="menu-btn quit-btn" on-click="OnQuit">Quit</button>
    </flex>

    <text class="footer-text">v{{version}}</text>
  </div>
</ui>
```

**`UI/theme.css`:**
```css
.fullscreen {
  anchor: full-stretch;
}

.bg-image {
  anchor: full-stretch;
}

.title-text {
  anchor: top-center;
  color: #ffffff;
  font-size: 48px;
  top: 60px;
  text-align: center;
}

.menu-buttons {
  anchor: center;
  flex-direction: column;
  gap: 8px;
  width: 220px;
  height: 260px;
}

.menu-btn {
  width: 200px;
  height: 50px;
  background-color: #334466;
  --hovered-color: #445588;
  --pressed-color: #223355;
  color: #ffffff;
  font-size: 20px;
  text-align: center;
}

.quit-btn {
  background-color: #663333;
  --hovered-color: #884444;
  --pressed-color: #552222;
}

.footer-text {
  anchor: bottom-center;
  color: #666666;
  font-size: 12px;
  top: -8px;
}
```

---

## Example 3: Inventory Grid

```xml
<ui>
  <style>
    .inventory-panel {
      anchor: center;
      width: 400px;
      height: 300px;
      background-color: rgba(0, 0, 0, 0.8);
      overflow: hidden;
    }
    .inv-title {
      anchor: top-center;
      color: #cccccc;
      font-size: 20px;
      top: 8px;
    }
    .inv-grid {
      anchor: top-center;
      top: 40px;
      width: 380px;
      height: 240px;
      flex-direction: row;
      gap: 4px;
      align-items: center;
    }
    .inv-slot {
      width: 56px;
      height: 56px;
      background-color: #222244;
      --hovered-color: #334466;
    }
  </style>

  <div class="inventory-panel">
    <text class="inv-title">Inventory</text>
    <flex class="inv-grid">
      <button id="slot-0" class="inv-slot" on-click="OnSlotClick"></button>
      <button id="slot-1" class="inv-slot" on-click="OnSlotClick"></button>
      <button id="slot-2" class="inv-slot" on-click="OnSlotClick"></button>
      <button id="slot-3" class="inv-slot" on-click="OnSlotClick"></button>
      <button id="slot-4" class="inv-slot" on-click="OnSlotClick"></button>
      <button id="slot-5" class="inv-slot" on-click="OnSlotClick"></button>
    </flex>
  </div>
</ui>
```

---

## Example 4: Dialog Box

```xml
<ui>
  <style>
    .dialog-overlay {
      anchor: full-stretch;
      background-color: rgba(0, 0, 0, 0.5);
    }
    .dialog-box {
      anchor: center;
      width: 320px;
      height: 180px;
      background-color: #222222;
    }
    .dialog-title {
      anchor: top-center;
      color: #ffffff;
      font-size: 22px;
      top: 12px;
    }
    .dialog-message {
      anchor: center;
      color: #cccccc;
      font-size: 16px;
      text-align: center;
      word-wrap: break-word;
      width: 280px;
    }
    .dialog-buttons {
      anchor: bottom-center;
      top: -12px;
      width: 280px;
      height: 40px;
      flex-direction: row;
      gap: 12px;
    }
    .btn-ok {
      width: 100px;
      height: 36px;
      background-color: #2a6db0;
      --hovered-color: #3a7dc0;
      color: #ffffff;
      font-size: 16px;
      text-align: center;
    }
    .btn-cancel {
      width: 100px;
      height: 36px;
      background-color: #555555;
      --hovered-color: #666666;
      color: #ffffff;
      font-size: 16px;
      text-align: center;
    }
  </style>

  <div class="dialog-overlay">
    <div class="dialog-box">
      <text class="dialog-title">{{title}}</text>
      <text class="dialog-message">{{message}}</text>
      <flex class="dialog-buttons">
        <button id="btn-ok" class="btn-ok" on-click="OnDialogOK">OK</button>
        <button id="btn-cancel" class="btn-cancel" on-click="OnDialogCancel">Cancel</button>
      </flex>
    </div>
  </div>
</ui>
```

**Lua:**
```lua
-- parentWidget: the Canvas widget to mount the dialog onto
function ShowDialog(parentWidget, title, message, onOK, onCancel)
    local dialog = UI.Load("UI/Dialog.xml")
    dialog:Mount(parentWidget)
    dialog:SetData("title", title)
    dialog:SetData("message", message)

    dialog:SetCallback("btn-ok", "click", function()
        dialog:Unmount()
        dialog:Destroy()
        if onOK then onOK() end
    end)

    dialog:SetCallback("btn-cancel", "click", function()
        dialog:Unmount()
        dialog:Destroy()
        if onCancel then onCancel() end
    end)

    return dialog
end

-- Usage (from a script attached to a Canvas):
ShowDialog(self, "Quit Game?", "Are you sure you want to quit?",
    function() QuitGame() end,
    function() end
)
```

---

## Example 5: Composing with Includes

**`UI/Components/StatBar.xml`:**
```xml
<ui>
  <div class="stat-bar-container">
    <text class="stat-label">{{label}}</text>
    <div class="stat-bar-bg">
      <img id="stat-fill" class="stat-bar-fill" />
    </div>
    <text class="stat-value">{{value}}</text>
  </div>
</ui>
```

**`UI/CharacterSheet.xml`:**
```xml
<ui>
  <link href="UI/rpg-theme.css" />

  <div class="char-panel">
    <text class="char-name">{{name}}</text>
    <img class="char-portrait" src="T_DefaultPortrait" />

    <include src="UI/Components/StatBar.xml" />
  </div>
</ui>
```

---

## Example 6: Animated UI

Octave has no built-in tween library, but you can build smooth animations using `TimerManager.SetTimer` combined with `Maths.Lerp`, `Maths.Damp`, and `Maths.Approach`.

### Reusable Tween Helper

**Lua:**
```lua
-- Simple tween helper using TimerManager
-- Returns a timer handle that can be cancelled
function Tween(duration, onUpdate, onComplete)
    local elapsed = 0.0
    local handle = nil

    handle = TimerManager.SetTimer(function(dt)
        elapsed = elapsed + dt
        local t = math.min(elapsed / duration, 1.0)

        -- Smooth ease-in-out curve
        local eased = t * t * (3.0 - 2.0 * t)
        onUpdate(eased)

        if t >= 1.0 then
            TimerManager.ClearTimer(handle)
            if onComplete then onComplete() end
        end
    end, 0.0, true)

    return handle
end
```

### Fade-In + Slide Menu

**`UI/AnimatedMenu.xml`:**
```xml
<ui>
  <style>
    .menu-root { anchor: full-stretch; }
    .overlay {
      anchor: full-stretch;
      background-color: rgba(0, 0, 0, 0.6);
      opacity: 0;
    }
    .title {
      anchor: top-center;
      color: #ffffff;
      font-size: 42px;
      top: -40px;
      opacity: 0;
    }
    .btn-column {
      anchor: center;
      flex-direction: column;
      gap: 10px;
      width: 220px;
      height: 280px;
    }
    .menu-btn {
      width: 200px;
      height: 48px;
      background-color: #2a5db0;
      --hovered-color: #3a6dc0;
      --pressed-color: #1a4da0;
      color: #ffffff;
      font-size: 18px;
      text-align: center;
      opacity: 0;
    }
  </style>

  <div class="menu-root">
    <img id="overlay" class="overlay" />
    <text id="title" class="title">My Game</text>

    <flex class="btn-column">
      <button id="btn-0" class="menu-btn" on-click="OnPlay">Play</button>
      <button id="btn-1" class="menu-btn" on-click="OnOptions">Options</button>
      <button id="btn-2" class="menu-btn" on-click="OnCredits">Credits</button>
      <button id="btn-3" class="menu-btn" on-click="OnQuit">Quit</button>
    </flex>
  </div>
</ui>
```

**Lua (attach to a Canvas widget):**
```lua
AnimatedMenu = {}

function AnimatedMenu:Start()
    self.ui = UI.Load("UI/AnimatedMenu.xml")
    self.ui:Mount(self)

    -- 1. Fade in the background overlay
    local overlay = self.ui:FindById("overlay")
    Tween(0.5, function(t)
        overlay:SetOpacityFloat(t * 0.6)
    end)

    -- 2. Slide the title down from above
    local title = self.ui:FindById("title")
    Tween(0.6, function(t)
        title:SetOpacityFloat(t)
        -- Slide from y=-40 to y=60
        local y = Maths.Lerp(-40.0, 60.0, t)
        title:SetY(y)
    end)

    -- 3. Stagger-fade each button with a delay
    for i = 0, 3 do
        local btn = self.ui:FindById("btn-" .. tostring(i))
        local delay = 0.3 + i * 0.1  -- stagger by 100ms each

        TimerManager.SetTimer(function()
            -- Scale bounce: start small, overshoot, settle
            Tween(0.35, function(t)
                btn:SetOpacityFloat(t)
                -- Overshoot curve: ease out back
                local s = 1.0 + math.sin(t * 3.14159) * 0.15
                btn:SetScale(s, s)
            end)
        end, delay, false)
    end
end

function AnimatedMenu:Tick(deltaTime)
    self.ui:Tick()
end

function AnimatedMenu:Stop()
    self.ui:Unmount()
    self.ui:Destroy()
end
```

### Smooth Health Bar with Damp

**Lua (attach to a Canvas widget):**
```lua
SmoothHUD = {}

function SmoothHUD:Start()
    self.hud = UI.Load("UI/HUD.xml")
    self.hud:Mount(self)
    self.displayHealth = 100.0
    self.targetHealth = 100.0
end

function SmoothHUD:TakeDamage(amount)
    self.targetHealth = math.max(self.targetHealth - amount, 0.0)
end

function SmoothHUD:Tick(deltaTime)
    -- Smoothly animate toward target (framerate-independent)
    self.displayHealth = Maths.Damp(self.displayHealth, self.targetHealth, 0.1, deltaTime)

    local healthFill = self.hud:FindById("health-fill")
    if healthFill then
        healthFill:SetWidthRatio(self.displayHealth / 100.0)

        -- Color: green at full, red at low
        local pct = self.displayHealth / 100.0
        local r = Maths.Lerp(0.13, 0.8, 1.0 - pct)
        local g = Maths.Lerp(0.8, 0.13, 1.0 - pct)
        healthFill:SetColor(r, g, 0.13, 1.0)
    end

    self.hud:Tick()
end

function SmoothHUD:Stop()
    self.hud:Unmount()
    self.hud:Destroy()
end
```

### Pulsing Notification

**`UI/Notification.xml`:**
```xml
<ui>
  <style>
    .notif-box {
      anchor: top-center;
      width: 300px;
      height: 50px;
      top: -60px;
      background-color: rgba(40, 120, 200, 0.9);
      opacity: 0;
    }
    .notif-text {
      anchor: center;
      color: #ffffff;
      font-size: 18px;
      text-align: center;
    }
  </style>

  <div id="notif" class="notif-box">
    <text class="notif-text">{{message}}</text>
  </div>
</ui>
```

**Lua:**
```lua
-- parentWidget: the Canvas widget to show notifications on
function ShowNotification(parentWidget, message, duration)
    local notif = UI.Load("UI/Notification.xml")
    notif:Mount(parentWidget)
    notif:SetData("message", message)

    local box = notif:FindById("notif")
    duration = duration or 2.0

    -- Slide in from top
    Tween(0.3, function(t)
        box:SetOpacityFloat(t)
        box:SetY(Maths.Lerp(-60.0, 16.0, t))
    end, function()
        -- Hold, then slide out
        TimerManager.SetTimer(function()
            Tween(0.3, function(t)
                box:SetOpacityFloat(1.0 - t)
                box:SetY(Maths.Lerp(16.0, -60.0, t))
            end, function()
                notif:Unmount()
                notif:Destroy()
            end)
        end, duration, false)
    end)
end

-- Usage (from a script attached to a Canvas):
ShowNotification(self, "Level Complete!", 2.0)
ShowNotification(self, "New Item Acquired", 1.5)
```
