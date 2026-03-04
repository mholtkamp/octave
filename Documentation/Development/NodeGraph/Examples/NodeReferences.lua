-- 16_NodeReferences.lua
-- Advanced: Demonstrates Node/Node3D input nodes and conversion/validation.
--
-- Setup:
--   1. Create a SceneGraph NodeGraphAsset:
--      - FindNodeNode: Name="TargetMesh" -> outputs Node
--      - IsValidNode: Node=FindNode.Result -> outputs Bool "FoundTarget"
--      - IsNullNode: Node=FindNode.Result -> outputs Bool "IsNull"
--      - NodeToNode3DNode: Node=FindNode.Result -> outputs Node3D + Success bool
--      - If cast succeeds, GetPositionNode from the Node3D output
--      - GraphOutput pins: "FoundTarget" (Bool), "IsNull" (Bool),
--        "CastSuccess" (Bool), "TargetPos" (Vector)
--   2. Add a Node3D named "TargetMesh" to your scene
--   3. Attach this script, assign the asset, press Play
--
-- Key insight: IsNull is the complement of IsValid. NodeToNode3D safely casts
-- a Node reference to Node3D, returning a success bool so you can branch on it.

NodeReferences = {}

function NodeReferences:Create()
    self.player = nil
    self.logTimer = 0.0
end

function NodeReferences:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset }
    }
end

function NodeReferences:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        self.player:Play()
        Log.Debug("Node References demo started")
    end
end

function NodeReferences:Tick(deltaTime)
    if not self.player or not self.player:IsPlaying() then
        return
    end

    self.logTimer = self.logTimer + deltaTime

    -- Read validation results from the graph
    local foundTarget = self.player:GetOutputBool(0)
    local isNull      = self.player:GetOutputBool(1)
    local castSuccess = self.player:GetOutputBool(2)
    local targetPos   = self.player:GetOutputVector(3)

    -- Log periodically
    if self.logTimer >= 1.0 then
        self.logTimer = 0.0
        Log.Debug(string.format(
            "Found=%s IsNull=%s Cast=%s Pos=(%.1f, %.1f, %.1f)",
            tostring(foundTarget),
            tostring(isNull),
            tostring(castSuccess),
            targetPos.x or 0, targetPos.y or 0, targetPos.z or 0
        ))
    end
end
