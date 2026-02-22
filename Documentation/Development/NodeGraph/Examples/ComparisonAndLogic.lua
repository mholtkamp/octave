-- 13_ComparisonAndLogic.lua
-- Intermediate: Demonstrates comparison and boolean logic nodes.
--
-- Setup:
--   1. Create a SceneGraph NodeGraphAsset with:
--      - Two Float InputNodes: "ValueA", "ValueB"
--      - EqualNode: A=ValueA, B=ValueB -> outputs Bool
--      - GreaterNode: A=ValueA, B=ValueB -> outputs Bool
--      - AndNode: A=Equal.Result, B=Greater.Result -> outputs Bool
--      - SelectNode: Condition=Greater.Result, A=ValueA, B=ValueB -> outputs Float
--      - GraphOutput with pins: "AreEqual" (Bool), "AIsGreater" (Bool),
--        "BothTrue" (Bool), "SelectedValue" (Float)
--   2. Attach this script, assign the asset, press Play

ComparisonAndLogic = {}

function ComparisonAndLogic:Create()
    self.player = nil
    self.elapsed = 0.0
end

function ComparisonAndLogic:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset }
    }
end

function ComparisonAndLogic:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        self.player:Play()
        Log.Debug("Comparison & Logic demo started")
    end
end

function ComparisonAndLogic:Tick(deltaTime)
    if not self.player or not self.player:IsPlaying() then
        return
    end

    self.elapsed = self.elapsed + deltaTime

    -- Feed two values that periodically cross each other
    local a = math.sin(self.elapsed) * 10.0
    local b = math.cos(self.elapsed) * 10.0

    self.player:SetInputFloat("ValueA", a)
    self.player:SetInputFloat("ValueB", b)

    -- Read outputs
    local areEqual    = self.player:GetOutputBool(0)
    local aIsGreater  = self.player:GetOutputBool(1)
    local bothTrue    = self.player:GetOutputBool(2)
    local selectedVal = self.player:GetOutputFloat(3)

    Log.Debug(string.format(
        "A=%.2f B=%.2f | Equal=%s Greater=%s Both=%s Selected=%.2f",
        a, b,
        tostring(areEqual),
        tostring(aIsGreater),
        tostring(bothTrue),
        selectedVal
    ))
end
