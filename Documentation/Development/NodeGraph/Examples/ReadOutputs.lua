-- 03_ReadOutputs.lua
-- Beginner: Read computed values from the graph's output pins.
--
-- Setup:
--   1. Create a NodeGraphAsset with an OutputNode
--      - Pin 0: Float (e.g. computed brightness)
--      - Pin 1: Vector (e.g. computed color as vec3)
--   2. Attach this script and assign the asset
--   3. Press Play - outputs are read and applied each frame
--
-- Note: GetOutput* methods use 0-based pin indices matching the
--       order of output pins on the graph's OutputNode.

ReadOutputs = {}

function ReadOutputs:Create()
    self.player = nil
end

function ReadOutputs:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset }
    }
end

function ReadOutputs:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        self.player:Play()
    end
end

function ReadOutputs:Tick(deltaTime)
    if not self.player or not self.player:IsPlaying() then
        return
    end

    -- Read a float from output pin 0
    local brightness = self.player:GetOutputFloat(0)

    -- Read a vector from output pin 1
    local color = self.player:GetOutputVector(1)

    -- Read a bool from output pin 2 (as int: 0 or 1)
    local isActive = self.player:GetOutputBool(2)

    -- Use the computed values
    Log.Debug(string.format(
        "Brightness=%.2f  Color=(%.1f, %.1f, %.1f)  Active=%s",
        brightness, color.x, color.y, color.z,
        tostring(isActive)
    ))

    -- Example: apply brightness to this node's scale
    local s = 0.5 + brightness * 0.5
    self:SetScale(Vec(s, s, s))
end
