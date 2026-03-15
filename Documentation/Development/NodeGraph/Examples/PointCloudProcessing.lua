-- PointCloudProcessing.lua
-- Point cloud filtering and transformation pipeline.
-- Demonstrates: CreatePointsGrid -> TransformPoints -> FilterPoints -> MergePoints
--
-- Setup:
--   1. Create a NodeGraphAsset (Procedural domain) with:
--      - StartEvent -> CreatePointsGrid (10x1x10)
--      - -> Noise (sample at each point position) -> SetPointAttribute("pscale", noise)
--      - -> FilterPoints (Attribute="pscale", Min=0.3, Max=1.0)
--      - Passed output -> TransformPoints (Translate Y by noise height)
--      - Create second grid -> TransformPoints (offset) -> MergePoints with first
--      - -> CopyToPoints
--   2. Create an InstancedMesh3D in your scene
--   3. Attach this script and assign the graph + mesh + target

PointCloudProcessing = {}

function PointCloudProcessing:Create()
    self.player = nil
    self.filterMin = 0.3
    self.filterMax = 1.0
    self.heightScale = 5.0
end

function PointCloudProcessing:GatherProperties()
    return
    {
        { name = "graphAsset",      type = DatumType.Asset },
        { name = "objectMesh",      type = DatumType.Asset },
        { name = "targetInstance",  type = DatumType.Node3D },
        { name = "filterMin",       type = DatumType.Float },
        { name = "filterMax",       type = DatumType.Float },
        { name = "heightScale",     type = DatumType.Float }
    }
end

function PointCloudProcessing:Start()
    local graphAsset = self:GetProperty("graphAsset")
    if graphAsset == nil then
        LogWarning("PointCloudProcessing: No graph asset assigned")
        return
    end

    self.player = NodeGraphPlayer.Create()
    self.player:SetNodeGraph(graphAsset)

    -- Feed processing parameters
    self.player:SetInputDatum("FilterMin", self.filterMin)
    self.player:SetInputDatum("FilterMax", self.filterMax)
    self.player:SetInputDatum("HeightScale", self.heightScale)
    self.player:SetInputDatum("Mesh", self:GetProperty("objectMesh"))
    self.player:SetInputDatum("Target", self:GetProperty("targetInstance"))

    -- Execute the processing pipeline
    self.player:Execute()
    LogDebug("PointCloudProcessing: Pipeline executed")
end

function PointCloudProcessing:Stop()
    if self.player then
        self.player:Destroy()
        self.player = nil
    end
end
