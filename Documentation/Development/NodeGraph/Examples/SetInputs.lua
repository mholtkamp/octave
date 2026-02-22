-- 02_SetInputs.lua
-- Beginner: Feed values into a NodeGraph's InputNodes from Lua.
--
-- Setup:
--   1. Create a NodeGraphAsset with InputNodes named "Speed" (Float) and "Direction" (Vector)
--   2. Attach this script to a Node and assign the asset
--   3. Press Play - the script feeds changing values into the graph each frame

SetInputs = {}

function SetInputs:Create()
    self.player = nil
    self.elapsed = 0.0
end

function SetInputs:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset }
    }
end

function SetInputs:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        self.player:Play()
    end
end

function SetInputs:Tick(deltaTime)
    if not self.player or not self.player:IsPlaying() then
        return
    end

    self.elapsed = self.elapsed + deltaTime

    -- SetInputFloat: feed a sine-wave speed value
    local speed = math.sin(self.elapsed) * 10.0
    self.player:SetInputFloat("Speed", speed)

    -- SetInputVector: feed a rotating direction
    local dir = Vec(
        math.cos(self.elapsed),
        0.0,
        math.sin(self.elapsed)
    )
    self.player:SetInputVector("Direction", dir)

    -- SetInputBool: toggle every 2 seconds
    local toggle = (math.floor(self.elapsed / 2.0) % 2) == 0
    self.player:SetInputBool("Enabled", toggle)
end
