Demo_ObjectSpawner = {}

function Demo_ObjectSpawner:Create()

    self.spawnPoint = Property.Create(DatumType.Node, nil)

end

function Demo_ObjectSpawner:Interact()

    -- Create new mesh node and give it physics
    local newMeshNode = Node.Construct("StaticMesh3D")
    newMeshNode:SetStaticMesh("SM_Cube")
    newMeshNode:EnableCollision(true)
    newMeshNode:EnablePhysics(true)

    -- Position it
    local spawnPos = self.spawnPoint:GetWorldPosition()
    spawnPos.x = spawnPos.x + Math.RandRange(-10.0, 0.0, 10.0)
    spawnPos.z = spawnPos.z + Math.RandRange(-10.0, 0.0, 10.0)
    newMeshNode:SetWorldPosition(spawnPos)

    local velocity = Math.RandRangeVec(Vec(-5, -5, -5), Vec(5, 5, 5))

    newMeshNode:SetRotation(Math.RandRangeVec(Vec(0, 0, 0), Vec(360, 360, 360)))

    -- Add it to the world
    self.world:GetRootNode():AddChild(newMeshNode)
    newMeshNode:UpdateTransform()

    newMeshNode:SetLinearVelocity(velocity)


end

function Demo_ObjectSpawner:GetInteractionHint()
    return "Spawn Object"
end