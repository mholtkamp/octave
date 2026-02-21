-- 07_AllInputTypes.lua
-- Intermediate: Demonstrates every SetInput* type supported by NodeGraphPlayer.
--
-- Setup:
--   1. Create a NodeGraphAsset with InputNodes:
--      - "MyFloat" (Float), "MyInt" (Integer), "MyBool" (Bool)
--      - "MyString" (String), "MyVector" (Vector), "MyColor" (Color)
--   2. Attach this script, assign the asset, press Play

AllInputTypes = {}

function AllInputTypes:Create()
    self.graphAsset = Property.Create(DatumType.Asset, nil)
    self.player = nil
    self.elapsed = 0.0
end

function AllInputTypes:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset }
    }
end

function AllInputTypes:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        self.player:Play()
    end
end

function AllInputTypes:Tick(deltaTime)
    if not self.player or not self.player:IsPlaying() then
        return
    end

    self.elapsed = self.elapsed + deltaTime

    -- Float: sine wave
    self.player:SetInputFloat("MyFloat", math.sin(self.elapsed) * 5.0)

    -- Integer: frame counter
    self.player:SetInputInt("MyInt", math.floor(self.elapsed * 60.0))

    -- Bool: toggles every second
    self.player:SetInputBool("MyBool", math.floor(self.elapsed) % 2 == 0)

    -- String: formatted label
    self.player:SetInputString("MyString", string.format("T=%.1f", self.elapsed))

    -- Vector: circular motion
    self.player:SetInputVector("MyVector", Vec(
        math.cos(self.elapsed) * 3.0,
        1.0,
        math.sin(self.elapsed) * 3.0
    ))

    -- Color: pulsing red-to-blue (vec4: r, g, b, a)
    local t = (math.sin(self.elapsed) + 1.0) * 0.5
    self.player:SetInputColor("MyColor", Vec(t, 0.2, 1.0 - t, 1.0))
end
