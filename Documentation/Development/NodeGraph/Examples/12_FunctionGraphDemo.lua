-- 12_FunctionGraphDemo.lua
-- Advanced: Work with NodeGraphAssets that contain user-defined functions.
--
-- This example demonstrates the function graph system. When a NodeGraphAsset
-- contains FunctionCallNodes, the runtime automatically evaluates them inline.
-- From Lua, you interact with the top-level graph normally - functions are
-- transparent to the scripting layer.
--
-- Setup:
--   1. Create a NodeGraphAsset with a function defined:
--      a. Click "+" in the Functions sidebar to create a function (e.g. "Remap01")
--      b. In the function graph: add a FloatInput "Value", some math nodes
--         (Subtract min, Divide by range), and a FunctionOutput with a Float pin
--      c. Switch back to Event Graph
--      d. Drag "Remap01" from the sidebar to create a FunctionCallNode
--      e. Connect inputs/outputs as normal
--   2. Attach this script and assign the asset
--
-- Key insight: FunctionCallNodes are evaluated automatically by the GraphProcessor.
-- Lua just feeds inputs and reads outputs from the top-level graph as usual.

FunctionGraphDemo = {}

function FunctionGraphDemo:Create()
    self.graphAsset = Property.Create(DatumType.Asset, nil)
    self.player = nil
    self.elapsed = 0.0
end

function FunctionGraphDemo:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset }
    }
end

function FunctionGraphDemo:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        -- PlayOnStart can also be set from Lua:
        self.player:SetPlayOnStart(false)
        self.player:Play()
        Log.Debug("Function graph demo started - functions evaluate transparently")
    end
end

function FunctionGraphDemo:Tick(deltaTime)
    if not self.player or not self.player:IsPlaying() then
        return
    end

    self.elapsed = self.elapsed + deltaTime

    -- Feed a raw value that the graph's function will remap
    local rawValue = math.sin(self.elapsed) * 50.0 + 50.0  -- range [0, 100]
    self.player:SetInputFloat("RawValue", rawValue)

    -- The graph internally calls "Remap01" to normalize [0,100] -> [0,1]
    -- We just read the final output
    local remapped = self.player:GetOutputFloat(0)

    Log.Debug(string.format("Raw=%.1f  Remapped=%.3f", rawValue, remapped))

    -- Apply the remapped value
    local s = 0.5 + remapped * 2.0
    self:SetScale(Vec(s, s, s))
end
