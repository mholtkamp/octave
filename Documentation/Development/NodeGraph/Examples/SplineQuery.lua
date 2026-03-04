-- SplineQuery.lua
-- Spline query nodes demo: move a node along a spline path.
-- Demonstrates: SplinePositionAt, SplineTangentAt, SplineNearestPercent, SplineLength
--
-- Setup:
--   1. Create a NodeGraphAsset (SceneGraph domain) with:
--      - TickEvent -> Time -> Fract -> SplinePositionAt (Spline=input, Percent=frac(time*speed))
--      - -> SetPosition (Self, position)
--      - SplineTangentAt -> (optional: orient along tangent)
--      - SplineLength -> DebugLog (shows total path length)
--   2. Create a Spline3D with several control points
--   3. Attach this script to the node you want to move along the spline

SplineQuery = {}

function SplineQuery:Create()
    self.player = nil
    self.speed = 0.2
    self.elapsed = 0.0
end

function SplineQuery:GatherProperties()
    return
    {
        { name = "graphAsset",  type = DatumType.Asset },
        { name = "spline",      type = DatumType.Node3D },
        { name = "speed",       type = DatumType.Float }
    }
end

function SplineQuery:Start()
    local graphAsset = self:GetProperty("graphAsset")
    if graphAsset == nil then
        LogWarning("SplineQuery: No graph asset assigned")
        return
    end

    self.player = NodeGraphPlayer.Create()
    self.player:SetNodeGraph(graphAsset)
    self.player:SetInputDatum("Spline", self:GetProperty("spline"))
    self.player:SetInputDatum("Speed", self.speed)

    -- Query spline length on startup
    self.player:Execute()

    local length = self.player:GetOutputDatum("Length")
    if length then
        LogDebug("SplineQuery: Spline length = " .. tostring(length))
    end
end

function SplineQuery:Tick(dt)
    if self.player == nil then
        return
    end

    self.elapsed = self.elapsed + dt

    -- Update time input and re-evaluate graph each frame
    self.player:SetInputDatum("Time", self.elapsed)
    self.player:SetInputDatum("Speed", self.speed)
    self.player:Execute()

    -- Read back position from the graph output
    local pos = self.player:GetOutputDatum("Position")
    if pos then
        self:SetPosition(pos)
    end

    -- Optionally read tangent for orientation
    local tangent = self.player:GetOutputDatum("Tangent")
    if tangent then
        -- Use tangent for forward direction (look-at style)
        -- This would require a LookAt or rotation conversion
    end
end

function SplineQuery:Stop()
    if self.player then
        self.player:Destroy()
        self.player = nil
    end
end
