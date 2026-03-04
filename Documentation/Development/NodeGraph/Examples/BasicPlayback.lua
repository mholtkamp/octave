-- 01_BasicPlayback.lua
-- Beginner: Attach a NodeGraphPlayer and play/pause/stop it.
--
-- Setup:
--   1. Create a NodeGraphAsset (e.g. a simple Material or SceneGraph graph)
--   2. Attach this script to any Node in your scene
--   3. Assign the NodeGraphAsset to the "graphAsset" property in the editor
--   4. Press Play - the graph starts automatically

BasicPlayback = {}

function BasicPlayback:Create()
    -- Exposed property: drag a NodeGraphAsset onto this in the editor

    -- Internal ref to the player node we create at runtime
    self.player = nil
end

function BasicPlayback:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset }
    }
end

function BasicPlayback:Start()
    -- Create a NodeGraphPlayer as a child of this node
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)
    
    -- Assign the graph asset and start playing
    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        self.player:Play()
        Log.Debug("NodeGraph is now playing")
    else
        Log.Warning("No NodeGraphAsset assigned!")
    end
end

function BasicPlayback:Tick(deltaTime)
    -- Check playback state each frame
    if self.player and self.player:IsPlaying() then
        -- Graph is evaluating every frame via NodeGraphPlayer:Tick()
    end
end

function BasicPlayback:Stop()
    if self.player then
        self.player:Stop()
        self.player:Reset()
        Log.Debug("NodeGraph stopped and reset")
    end
end
