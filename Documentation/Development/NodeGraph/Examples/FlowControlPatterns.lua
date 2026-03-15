-- 15_FlowControlPatterns.lua
-- Advanced: Demonstrates flow control nodes (FlipFlop, DoOnce, Gate, DoN).
--
-- Setup:
--   1. Create a SceneGraph NodeGraphAsset:
--      - TickEvent -> FlipFlopNode
--      - FlipFlop "A" exec -> SetAttribute(Float) on self, key="PathA", value=1.0
--      - FlipFlop "B" exec -> SetAttribute(Float) on self, key="PathB", value=1.0
--      - FlipFlop "Is A" bool output -> GraphOutput pin "IsPathA" (Bool)
--      - Also connect TickEvent -> DoOnceNode -> SetAttribute(String) key="InitMsg" value="Initialized"
--      - GraphOutput pin "FlipCount" (Integer) from a counter if desired
--   2. Attach this script, assign the asset, press Play
--
-- Key insight: FlipFlop alternates between two execution paths each time it fires.
-- DoOnce fires its output only on the first trigger, then ignores subsequent calls.

FlowControlPatterns = {}

function FlowControlPatterns:Create()
    self.player = nil
    self.tickCount = 0
end

function FlowControlPatterns:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset }
    }
end

function FlowControlPatterns:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        self.player:Play()
        Log.Debug("Flow Control demo started")
    end
end

function FlowControlPatterns:Tick(deltaTime)
    if not self.player or not self.player:IsPlaying() then
        return
    end

    self.tickCount = self.tickCount + 1

    -- The graph evaluates each tick automatically via the TickEvent.
    -- FlipFlop alternates paths A and B each evaluation.
    local isPathA = self.player:GetOutputBool(0)

    -- Log every 60 ticks to avoid spam
    if self.tickCount % 60 == 0 then
        Log.Debug(string.format(
            "Tick %d | FlipFlop path: %s",
            self.tickCount,
            isPathA and "A" or "B"
        ))
    end
end
