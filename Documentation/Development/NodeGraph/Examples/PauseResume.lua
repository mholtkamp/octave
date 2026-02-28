-- 04_PauseResume.lua
-- Beginner: Pause, resume, and reset a NodeGraphPlayer at runtime.
--
-- Setup:
--   1. Attach to a Node with a NodeGraphAsset assigned
--   2. This example simulates pausing on a key press (here we use a timer toggle)

PauseResume = {}

function PauseResume:Create()
    self.player = nil
    self.elapsed = 0.0
    self.pauseInterval = 3.0  -- toggle pause every 3 seconds
end

function PauseResume:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset },
        { name = "pauseInterval", type = DatumType.Float }
    }
end

function PauseResume:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        self.player:Play()
    end
end

function PauseResume:Tick(deltaTime)
    if not self.player then return end

    self.elapsed = self.elapsed + deltaTime

    -- Toggle pause every N seconds
    if self.elapsed >= self.pauseInterval then
        self.elapsed = 0.0

        if self.player:IsPlaying() then
            self.player:Pause()
            Log.Debug("Graph PAUSED")
        elseif self.player:IsPaused() then
            self.player:Play()
            Log.Debug("Graph RESUMED")
        end
    end
end

function PauseResume:Stop()
    if self.player then
        self.player:Stop()
        self.player:Reset()
    end
end
