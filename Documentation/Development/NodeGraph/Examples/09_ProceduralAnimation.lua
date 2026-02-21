-- 09_ProceduralAnimation.lua
-- Advanced: Use a NodeGraph to drive procedural animation.
-- Feeds time + speed into the graph, reads back position/rotation offsets.
--
-- Setup:
--   1. Create a NodeGraphAsset (SceneGraph or Animation domain) with:
--      - InputNodes: "Time" (Float), "Speed" (Float), "Amplitude" (Float)
--      - Math nodes to compute sine/cosine bobbing motion
--      - OutputNode pins: 0=Position (Vector), 1=Rotation (Vector)
--   2. Attach this script to the node you want to animate

ProceduralAnimation = {}

function ProceduralAnimation:Create()
    self.graphAsset = Property.Create(DatumType.Asset, nil)
    self.speed = 1.0
    self.amplitude = 2.0
    self.player = nil
    self.elapsed = 0.0
    self.basePosition = Vec()
    self.baseRotation = Vec()
end

function ProceduralAnimation:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset },
        { name = "speed", type = DatumType.Float },
        { name = "amplitude", type = DatumType.Float }
    }
end

function ProceduralAnimation:Start()
    -- Remember starting transform so we can add offsets
    self.basePosition = self:GetPosition()
    self.baseRotation = self:GetRotation()

    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        self.player:Play()
    end
end

function ProceduralAnimation:Tick(deltaTime)
    if not self.player or not self.player:IsPlaying() then
        return
    end

    self.elapsed = self.elapsed + deltaTime

    -- Feed parameters into the graph
    self.player:SetInputFloat("Time", self.elapsed)
    self.player:SetInputFloat("Speed", self.speed)
    self.player:SetInputFloat("Amplitude", self.amplitude)

    -- Read computed offsets
    local posOffset = self.player:GetOutputVector(0)
    local rotOffset = self.player:GetOutputVector(1)

    -- Apply as additive offsets to the base transform
    self:SetPosition(Vec(
        self.basePosition.x + posOffset.x,
        self.basePosition.y + posOffset.y,
        self.basePosition.z + posOffset.z
    ))
    self:SetRotation(Vec(
        self.baseRotation.x + rotOffset.x,
        self.baseRotation.y + rotOffset.y,
        self.baseRotation.z + rotOffset.z
    ))
end

function ProceduralAnimation:Stop()
    -- Restore original transform
    self:SetPosition(self.basePosition)
    self:SetRotation(self.baseRotation)

    if self.player then
        self.player:Stop()
        self.player:Reset()
    end
end
