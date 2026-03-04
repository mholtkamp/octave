-- 10_MultiGraphOrchestrator.lua
-- Advanced: Run multiple NodeGraphPlayers simultaneously and chain their outputs.
-- Graph A computes a value, Graph B uses that value as input.
--
-- Setup:
--   1. Create two NodeGraphAssets:
--      - Graph A: InputNode "Time" (Float) -> math -> OutputNode pin 0 (Float "WaveValue")
--      - Graph B: InputNode "WaveValue" (Float), "Offset" (Vector)
--                 -> processing -> OutputNode pin 0 (Vector "FinalPosition")
--   2. Attach this script, assign both graph assets

MultiGraphOrchestrator = {}

function MultiGraphOrchestrator:Create()
    self.offset = Vec(0.0, 5.0, 0.0)

    self.playerA = nil
    self.playerB = nil
    self.elapsed = 0.0
end

function MultiGraphOrchestrator:GatherProperties()
    return
    {
        { name = "graphA_Asset", type = DatumType.Asset },
        { name = "graphB_Asset", type = DatumType.Asset },
        { name = "offset", type = DatumType.Vector },
        { name = "target", type = DatumType.Node }
    }
end

function MultiGraphOrchestrator:Start()
    -- Create two players as children
    ---@type NodeGraphPlayer
    self.playerA = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.playerA)

    ---@type NodeGraphPlayer
    self.playerB = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.playerB)

    -- Start both graphs
    if self.graphA_Asset then
        self.playerA:SetNodeGraphAsset(self.graphA_Asset)
        self.playerA:Play()
    end

    if self.graphB_Asset then
        self.playerB:SetNodeGraphAsset(self.graphB_Asset)
        self.playerB:Play()
    end
end

function MultiGraphOrchestrator:Tick(deltaTime)
    self.elapsed = self.elapsed + deltaTime
    -- Step 1: Feed time into Graph A
    if self.playerA and self.playerA:IsPlaying() then
        self.playerA:SetInputFloat("Time", self.elapsed)
    end

    -- Step 2: Read Graph A's output and chain into Graph B
    if self.playerA and self.playerB
       and self.playerA:IsPlaying()
       and self.playerB:IsPlaying() then

        local waveValue = self.playerA:GetOutputFloat(0)
        self.playerB:SetInputNode3D("Target", self.target)

        self.playerB:SetInputFloat("WaveValue", waveValue)
        self.playerB:SetInputVector("Offset", self.offset)

        -- Step 3: Read Graph B's final output and apply
        local finalPos = self.playerB:GetOutputVector(0)
        self:SetPosition(finalPos)
    end
end

function MultiGraphOrchestrator:Stop()
    if self.playerA then
        self.playerA:Stop()
        self.playerA:Reset()
    end
    if self.playerB then
        self.playerB:Stop()
        self.playerB:Reset()
    end
end
