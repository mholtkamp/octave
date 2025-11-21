Demo_LightSwitch = {}

function Demo_LightSwitch:Create()

    self.light = Property.Create(DatumType.Node, nil)
    self.meshPivot = Property.Create(DatumType.Node, nil)

end

function Demo_LightSwitch:Interact()

    if (self.light) then
        self.light:SetVisible(not self.light:IsVisible())
    else
        Log.Warning("No light assigned to switch")
    end

    -- Flip the mesh
    local meshAngle = self.light:IsVisible() and -45.0 or 45.0
    local meshRot = self.meshPivot:GetRotation()
    meshRot.x = meshAngle
    self.meshPivot:SetRotation(meshRot)
end

function Demo_LightSwitch:GetInteractionHint()
    return "Toggle Light"
end