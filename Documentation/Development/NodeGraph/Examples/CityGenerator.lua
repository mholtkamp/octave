-- CityGenerator.lua
-- Simple city block generator using the PointCloud system.
-- Demonstrates: CreatePointsGrid -> ForEachPoint -> Branch (noise) -> CopyToPoints
--
-- Setup:
--   1. Create a NodeGraphAsset (Procedural domain) with:
--      - StartEvent -> CreatePointsGrid (CountX=input, CountZ=input, Spacing=input)
--      - -> ForEachPoint -> Noise(Position) -> Branch (value > threshold)
--      - True: SetPointAttribute("pscale", noise * scaleMultiplier)
--      - -> CopyToPoints (Mesh=building mesh, Target=InstancedMesh3D)
--   2. Create an InstancedMesh3D for buildings in your scene
--   3. Attach this script and assign the graph + building mesh

CityGenerator = {}

function CityGenerator:Create()
    self.player = nil
    self.gridSizeX = 10
    self.gridSizeZ = 10
    self.blockSpacing = 5.0
    self.noiseThreshold = 0.4
    self.buildingScaleMin = 1.0
    self.buildingScaleMax = 5.0
end

function CityGenerator:GatherProperties()
    return
    {
        { name = "graphAsset",      type = DatumType.Asset },
        { name = "buildingMesh",    type = DatumType.Asset },
        { name = "targetInstance",  type = DatumType.Node3D },
        { name = "gridSizeX",       type = DatumType.Integer },
        { name = "gridSizeZ",       type = DatumType.Integer },
        { name = "blockSpacing",    type = DatumType.Float },
        { name = "noiseThreshold",  type = DatumType.Float },
        { name = "buildingScaleMax",type = DatumType.Float }
    }
end

function CityGenerator:Start()
    local graphAsset = self:GetProperty("graphAsset")
    if graphAsset == nil then
        LogWarning("CityGenerator: No graph asset assigned")
        return
    end

    self.player = NodeGraphPlayer.Create()
    self.player:SetNodeGraph(graphAsset)

    -- Feed city parameters into the graph
    self.player:SetInputDatum("GridX", self.gridSizeX)
    self.player:SetInputDatum("GridZ", self.gridSizeZ)
    self.player:SetInputDatum("Spacing", self.blockSpacing)
    self.player:SetInputDatum("Threshold", self.noiseThreshold)
    self.player:SetInputDatum("ScaleMax", self.buildingScaleMax)
    self.player:SetInputDatum("BuildingMesh", self:GetProperty("buildingMesh"))
    self.player:SetInputDatum("Target", self:GetProperty("targetInstance"))

    -- Execute the city generation graph
    self.player:Execute()
    LogDebug("CityGenerator: Generated city grid " ..
        self.gridSizeX .. "x" .. self.gridSizeZ)
end

function CityGenerator:Stop()
    if self.player then
        self.player:Destroy()
        self.player = nil
    end
end
