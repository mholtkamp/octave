Rotator = {}

function Rotator:Create()

    self.angularVelocity = Vec()

end

function Rotator:GatherProperties()

    return 
    {
        { name = "angularVelocity", type = DatumType.Vector }
    }

end

function Rotator:Tick(deltaTime)

    self:AddRotation(self.angularVelocity * deltaTime)
    
end
