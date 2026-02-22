-- 08_MultipleOutputs.lua
-- Intermediate: Read multiple output types from a graph and apply them to scene nodes.
--
-- Setup:
--   1. Create a NodeGraphAsset with an OutputNode containing:
--      - Pin 0: Float  (e.g. "Scale")
--      - Pin 1: Vector (e.g. "Position")
--      - Pin 2: Color  (e.g. "Tint")
--      - Pin 3: Bool   (e.g. "Visible")
--      - Pin 4: Int    (e.g. "State")
--   2. Attach this script to a Node, assign the asset
--   3. Optionally assign a target node to receive the computed transform

MultipleOutputs = {}

function MultipleOutputs:Create()
    self.player = nil
end

function MultipleOutputs:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset },
        { name = "targetNode", type = DatumType.Node }
    }
end

function MultipleOutputs:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        self.player:Play()
    end
end

function MultipleOutputs:Tick(deltaTime)
    if not self.player or not self.player:IsPlaying() then
        return
    end

    local target = self.targetNode or self

    -- Pin 0: Float scale
    local scale = self.player:GetOutputFloat(0)
    target:SetScale(Vec(scale, scale, scale))

    -- Pin 1: Vector position
    local pos = self.player:GetOutputVector(1)
    target:SetPosition(pos)

    -- Pin 2: Color tint (vec4)
    local tint = self.player:GetOutputColor(2)
    Log.Debug(string.format("Tint: (%.2f, %.2f, %.2f, %.2f)", tint.x, tint.y, tint.z, tint.w))

    -- Pin 3: Bool visibility
    local visible = self.player:GetOutputBool(3)
    target:SetVisible(visible)

    -- Pin 4: Int state index
    local state = self.player:GetOutputInt(4)
    if state == 0 then
        -- idle
    elseif state == 1 then
        -- active
    elseif state == 2 then
        -- cooldown
    end
end
