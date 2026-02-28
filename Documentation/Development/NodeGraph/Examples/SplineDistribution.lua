-- SplineDistribution.lua
-- Distribute objects along a spline path using the PointCloud system.
-- Demonstrates: PointsFromSpline -> RandomizeAttribute("pscale") -> CopyToPoints
--
-- Setup:
--   1. Create a NodeGraphAsset (SceneGraph domain) with:
--      - StartEvent -> PointsFromSpline (Spline=input, Count=input)
--      - -> RandomizeAttribute(Name="pscale", Min=0.8, Max=1.2, Seed=input)
--      - -> CopyToPoints (Mesh=input, Target=input InstancedMesh3D)
--   2. Create a Spline3D and an InstancedMesh3D in your scene
--   3. Attach this script and assign the spline + mesh + target

SplineDistribution = {}

function SplineDistribution:Create()
    self.player = nil
    self.objectCount = 30
    self.scaleVariation = 0.3
    self.seed = 0
end

function SplineDistribution:GatherProperties()
    return
    {
        { name = "graphAsset",      type = DatumType.Asset },
        { name = "spline",          type = DatumType.Node3D },
        { name = "objectMesh",      type = DatumType.Asset },
        { name = "targetInstance",  type = DatumType.Node3D },
        { name = "objectCount",     type = DatumType.Integer },
        { name = "scaleVariation",  type = DatumType.Float },
        { name = "seed",            type = DatumType.Integer }
    }
end

function SplineDistribution:Start()
    local graphAsset = self:GetProperty("graphAsset")
    if graphAsset == nil then
        LogWarning("SplineDistribution: No graph asset assigned")
        return
    end

    self.player = NodeGraphPlayer.Create()
    self.player:SetNodeGraph(graphAsset)

    -- Feed inputs
    self.player:SetInputDatum("Spline", self:GetProperty("spline"))
    self.player:SetInputDatum("Count", self.objectCount)
    self.player:SetInputDatum("ScaleMin", 1.0 - self.scaleVariation)
    self.player:SetInputDatum("ScaleMax", 1.0 + self.scaleVariation)
    self.player:SetInputDatum("Seed", self.seed)
    self.player:SetInputDatum("Mesh", self:GetProperty("objectMesh"))
    self.player:SetInputDatum("Target", self:GetProperty("targetInstance"))

    -- Execute once to place objects along the spline
    self.player:Execute()
    LogDebug("SplineDistribution: Placed " .. self.objectCount .. " objects along spline")
end

function SplineDistribution:Stop()
    if self.player then
        self.player:Destroy()
        self.player = nil
    end
end
