-- 06_InputOutputLoop.lua
-- Intermediate: Full input/output feedback loop. Read graph outputs and
-- feed them back as inputs next frame, creating an iterative system.
--
-- Setup:
--   1. Create a NodeGraphAsset with:
--      - InputNode "Value" (Float)
--      - Some processing (e.g. Add 1.0, Clamp 0-100)
--      - OutputNode pin 0 (Float)
--   2. Attach this script and assign the asset
--   3. Press Play - "Value" increments each frame via feedback

InputOutputLoop = {}

function InputOutputLoop:Create()
    self.graphAsset = Property.Create(DatumType.Asset, nil)
    self.player = nil
    self.currentValue = 0.0
    self.maxValue = 100.0
end

function InputOutputLoop:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset },
        { name = "maxValue", type = DatumType.Float }
    }
end

function InputOutputLoop:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        self.player:Play()
    end

    self.currentValue = 0.0
end

function InputOutputLoop:Tick(deltaTime)
    if not self.player or not self.player:IsPlaying() then
        return
    end

    -- Feed the current value into the graph
    self.player:SetInputFloat("Value", self.currentValue)

    -- Read the graph's computed result
    local result = self.player:GetOutputFloat(0)

    -- Clamp and store for next frame
    self.currentValue = math.min(result, self.maxValue)

    -- Visualize: scale this node based on the accumulating value
    local t = self.currentValue / self.maxValue
    self:SetScale(Vec(1.0 + t, 1.0 + t, 1.0 + t))
end
