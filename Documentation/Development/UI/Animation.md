# Animating UI Elements

This guide covers two approaches to animating widgets: **manual animation** in `Tick()` for simple tweens, and **Timeline-based animation** for complex, authored sequences.

---

## Manual Animation in Tick

The most direct approach: update widget properties each frame using delta time.

### Fading In

```lua
function FadeIn:Start()
    self.elapsed = 0.0
    self.duration = 1.0
    self:SetOpacityFloat(0.0) -- start invisible
end

function FadeIn:Tick(deltaTime)
    self.elapsed = self.elapsed + deltaTime
    local t = math.min(self.elapsed / self.duration, 1.0)
    self:SetOpacityFloat(t)
end
```

**C++**
```cpp
void FadeWidget::Tick(float deltaTime)
{
    Widget::Tick(deltaTime);

    mElapsed += deltaTime;
    float t = glm::clamp(mElapsed / mDuration, 0.0f, 1.0f);
    SetOpacityFloat(t);
}
```

### Fading Out

```lua
function FadeOut:Tick(deltaTime)
    self.elapsed = self.elapsed + deltaTime
    local t = math.min(self.elapsed / self.duration, 1.0)
    self:SetOpacityFloat(1.0 - t)

    if t >= 1.0 then
        self:SetVisible(false) -- hide when fully faded
    end
end
```

---

## Crossfading Two Images

Overlap two Quads and animate their opacities inversely:

```lua
function Crossfade:Start()
    -- Two overlapping quads
    self.imageA = self:CreateChild("Quad")
    self.imageA:SetAnchorMode(AnchorMode.FullStretch)
    self.imageA:SetRatios(0.0, 0.0, 1.0, 1.0)
    self.imageA:SetTexture(LoadAsset("T_ImageA"))
    self.imageA:SetOpacityFloat(1.0)

    self.imageB = self:CreateChild("Quad")
    self.imageB:SetAnchorMode(AnchorMode.FullStretch)
    self.imageB:SetRatios(0.0, 0.0, 1.0, 1.0)
    self.imageB:SetTexture(LoadAsset("T_ImageB"))
    self.imageB:SetOpacityFloat(0.0)

    self.blendT = 0.0
    self.blending = false
end

function Crossfade:StartBlend()
    self.blendT = 0.0
    self.blending = true
end

function Crossfade:Tick(deltaTime)
    if not self.blending then return end

    self.blendT = self.blendT + deltaTime / 1.0 -- 1 second duration
    if self.blendT >= 1.0 then
        self.blendT = 1.0
        self.blending = false
    end

    self.imageA:SetOpacityFloat(1.0 - self.blendT)
    self.imageB:SetOpacityFloat(self.blendT)
end
```

---

## Moving a Widget

Slide a widget from one position to another:

```lua
function SlideIn:Start()
    self.startPos = Vec(-300.0, 100.0) -- off-screen left
    self.endPos = Vec(50.0, 100.0)     -- on-screen
    self.elapsed = 0.0
    self.duration = 0.5
    self:SetPosition(self.startPos.x, self.startPos.y)
end

function SlideIn:Tick(deltaTime)
    self.elapsed = self.elapsed + deltaTime
    local t = math.min(self.elapsed / self.duration, 1.0)

    -- Ease-out (decelerate)
    local eased = 1.0 - (1.0 - t) * (1.0 - t)

    local x = self.startPos.x + (self.endPos.x - self.startPos.x) * eased
    local y = self.startPos.y + (self.endPos.y - self.startPos.y) * eased
    self:SetPosition(x, y)
end
```

**C++**
```cpp
void SlideWidget::Tick(float deltaTime)
{
    Widget::Tick(deltaTime);

    mElapsed += deltaTime;
    float t = glm::clamp(mElapsed / mDuration, 0.0f, 1.0f);

    // Ease-out
    float eased = 1.0f - (1.0f - t) * (1.0f - t);

    glm::vec2 pos = glm::mix(mStartPos, mEndPos, eased);
    SetPosition(pos.x, pos.y);
}
```

---

## Scaling a Widget

Pulse or grow a widget using scale:

```lua
function PulseWidget:Start()
    self.time = 0.0
end

function PulseWidget:Tick(deltaTime)
    self.time = self.time + deltaTime
    local scale = 1.0 + 0.1 * math.sin(self.time * 4.0) -- gentle pulse
    self:SetScale(scale, scale)
end
```

---

## Color Transitions

Lerp between two colors:

```lua
function ColorFlash:Start()
    self.startColor = Vec(1, 1, 1, 1) -- white
    self.endColor = Vec(1, 0, 0, 1)   -- red
    self.elapsed = 0.0
    self.duration = 0.3
end

function ColorFlash:Tick(deltaTime)
    self.elapsed = self.elapsed + deltaTime
    local t = math.min(self.elapsed / self.duration, 1.0)

    local r = self.startColor.x + (self.endColor.x - self.startColor.x) * t
    local g = self.startColor.y + (self.endColor.y - self.startColor.y) * t
    local b = self.startColor.z + (self.endColor.z - self.startColor.z) * t
    local a = self.startColor.w + (self.endColor.w - self.startColor.w) * t
    self:SetColor(Vec(r, g, b, a))
end
```

---

## Reusable Tween Helper

A simple Lua helper you can use across your project:

```lua
Tween = {}

function Tween.Lerp(a, b, t)
    return a + (b - a) * t
end

function Tween.EaseOut(t)
    return 1.0 - (1.0 - t) * (1.0 - t)
end

function Tween.EaseIn(t)
    return t * t
end

function Tween.EaseInOut(t)
    if t < 0.5 then
        return 2.0 * t * t
    else
        return 1.0 - 2.0 * (1.0 - t) * (1.0 - t)
    end
end
```

Usage:
```lua
local t = math.min(self.elapsed / self.duration, 1.0)
local eased = Tween.EaseOut(t)
self:SetOpacityFloat(Tween.Lerp(0.0, 1.0, eased))
```

---

## Timeline-Based Animation

For more complex, designer-driven animations, use the **Timeline** system. A Timeline asset contains tracks that animate node properties over time, including widget properties like opacity, color, and position.

### Using ScriptValueTrack for Widget Properties

A `ScriptValueTrack` can animate any script property on a node. Attach a script to your widget, expose a property, and animate it from the Timeline.

**Example Lua script with animatable property:**
```lua
function AnimatedPanel:Create()
    -- Expose a property the Timeline can drive
    self.fadeAmount = 0.0
end

function AnimatedPanel:GatherProperties()
    return {
        { name = "fadeAmount", type = "Float" }
    }
end

function AnimatedPanel:Tick(deltaTime)
    -- Apply the animated value to the widget
    self:SetOpacityFloat(self.fadeAmount)
end
```

Then in the editor, create a Timeline asset with a ScriptValueTrack targeting the `fadeAmount` property on this node. Add keyframes to define the animation curve.

### Playing a Timeline

```lua
-- Attach a TimelinePlayer to the scene and play
local player = self:CreateChild("TimelinePlayer")
player:SetTimeline(LoadAsset("TL_UIAnimation"))
player:Play()
```

For full details on the Timeline system, see the [Timeline documentation](../Timeline/Overview.md).

---

## Further Reading

- [Widget System Overview](Overview.md)
- [Displaying Images](DisplayingImages.md) — animate Quad opacity, UV offset
- [Building Complete UIs](BuildingUI.md) — full examples with animation
- [Timeline Overview](../Timeline/Overview.md)
