-- 05_SwapGraphAtRuntime.lua
-- Intermediate: Hot-swap between different NodeGraphAssets at runtime.
--
-- Setup:
--   1. Create two different NodeGraphAssets (e.g. "DayLighting" and "NightLighting")
--   2. Assign both to this script's properties
--   3. Press Play - the script alternates between them every 5 seconds

SwapGraphAtRuntime = {}

function SwapGraphAtRuntime:Create()
    self.player = nil
    self.elapsed = 0.0
    self.swapInterval = 5.0
    self.usingA = true
end

function SwapGraphAtRuntime:GatherProperties()
    return
    {
        { name = "graphA", type = DatumType.Asset },
        { name = "graphB", type = DatumType.Asset },
        { name = "swapInterval", type = DatumType.Float }
    }
end

function SwapGraphAtRuntime:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    -- Start with graph A
    if self.graphA then
        self.player:SetNodeGraphAsset(self.graphA)
        self.player:Play()
        self.usingA = true
        Log.Debug("Started with Graph A")
    end
end

function SwapGraphAtRuntime:Tick(deltaTime)
    if not self.player then return end

    self.elapsed = self.elapsed + deltaTime

    if self.elapsed >= self.swapInterval then
        self.elapsed = 0.0

        -- Stop current graph, swap asset, restart
        self.player:Stop()
        self.player:Reset()

        if self.usingA and self.graphB then
            self.player:SetNodeGraphAsset(self.graphB)
            self.usingA = false
            Log.Debug("Swapped to Graph B")
        elseif self.graphA then
            self.player:SetNodeGraphAsset(self.graphA)
            self.usingA = true
            Log.Debug("Swapped to Graph A")
        end

        self.player:Play()
    end
end
