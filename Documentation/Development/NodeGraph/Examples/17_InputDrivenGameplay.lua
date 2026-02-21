-- 17_InputDrivenGameplay.lua
-- Advanced: Demonstrates input system nodes driving gameplay autonomously.
--
-- Setup:
--   1. Create a SceneGraph NodeGraphAsset:
--      - TickEvent -> Sequence
--      - Sequence[0] -> InputDownNode (Key=Space) -> Branch
--        - Branch True -> GetSelfNode -> GetPositionNode -> Add(pos, Vec(0,1,0)*dt)
--          -> SetPositionNode (move up while Space held)
--      - Sequence[1] -> InputDownNode (Key=LeftShift) -> Branch
--        - Branch True -> GetSelfNode -> GetPositionNode -> Add(pos, Vec(0,-1,0)*dt)
--          -> SetPositionNode (move down while Shift held)
--      - InputEventNode (Key=R, Pressed) -> GetSelfNode -> SetPosition(0,0,0) (reset)
--   2. Attach this script to the Node3D you want to move
--   3. Assign the asset, press Play
--
-- Key insight: The graph handles all input detection and movement internally.
-- Lua just starts the graph - no per-frame Lua input polling needed.
-- The InputDownNode checks key state each tick, and the graph's execution
-- flow handles the rest.

InputDrivenGameplay = {}

function InputDrivenGameplay:Create()
    self.graphAsset = Property.Create(DatumType.Asset, nil)
    self.player = nil
end

function InputDrivenGameplay:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset }
    }
end

function InputDrivenGameplay:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        self.player:Play()
        Log.Debug("Input-driven gameplay started")
        Log.Debug("  Space = move up, Shift = move down, R = reset position")
    end
end

function InputDrivenGameplay:Tick(deltaTime)
    -- The graph runs autonomously - no Lua input handling needed.
    -- All input detection, branching, and position updates happen inside the graph.
    --
    -- If you want to layer additional Lua logic on top, you can still
    -- read outputs or set inputs here. For example:
    --
    -- local pos = self:GetPosition()
    -- Log.Debug(string.format("Current pos: (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z))
end
