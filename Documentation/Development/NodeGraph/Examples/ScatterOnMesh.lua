-- ScatterOnMesh.lua
-- Scatter objects on a mesh surface using the PointCloud system.
-- Demonstrates: ScatterPointsOnMesh -> RandomizeAttribute("pscale") -> CopyToPoints
--
-- Setup:
--   1. Create a NodeGraphAsset (Procedural or SceneGraph domain) with:
--      - StartEvent -> ScatterPointsOnMesh (Mesh=input, Count=input, Seed=input)
--      - -> RandomizeAttribute(Name="pscale", Min=0.5, Max=1.5)
--      - -> CopyToPoints (Mesh=input, Target=input InstancedMesh3D ref)
--   2. Create an InstancedMesh3D in your scene
--   3. Attach this script to a node with a NodeGraphPlayer component
--   4. Assign the scatter mesh and target InstancedMesh3D

ScatterOnMesh = {}

function ScatterOnMesh:Create()
    self.player = nil
    self.scatterCount = 200
    self.seed = 42
end

function ScatterOnMesh:GatherProperties()
    return
    {
        { name = "graphAsset",      type = DatumType.Asset },
        { name = "sourceMesh",      type = DatumType.Asset },
        { name = "scatterMesh",     type = DatumType.Asset },
        { name = "targetInstance",  type = DatumType.Node3D },
        { name = "scatterCount",    type = DatumType.Integer },
        { name = "seed",            type = DatumType.Integer }
    }
end

function ScatterOnMesh:Start()
    -- Create a NodeGraphPlayer to execute the scatter graph
    local graphAsset = self:GetProperty("graphAsset")
    if graphAsset == nil then
        LogWarning("ScatterOnMesh: No graph asset assigned")
        return
    end

    self.player = NodeGraphPlayer.Create()
    self.player:SetNodeGraph(graphAsset)

    -- Feed inputs into the graph
    self.player:SetInputDatum("Mesh", self:GetProperty("sourceMesh"))
    self.player:SetInputDatum("Count", self.scatterCount)
    self.player:SetInputDatum("Seed", self.seed)
    self.player:SetInputDatum("ScatterMesh", self:GetProperty("scatterMesh"))
    self.player:SetInputDatum("Target", self:GetProperty("targetInstance"))

    -- Execute the graph once to scatter objects
    self.player:Execute()
    LogDebug("ScatterOnMesh: Scattered " .. self.scatterCount .. " objects on mesh")
end

function ScatterOnMesh:Stop()
    if self.player then
        self.player:Destroy()
        self.player = nil
    end
end
