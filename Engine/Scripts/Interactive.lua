Interactive = {}

Script.Inherit(Interactive, "Primitive3D")


-- [Script hierarchy]
--   Root
--     Interactive (Primitive3D, with this script)

function Interactive:Create()

end

function Interactive:Start()

    self:AddTag("Interactive")

    self:EnableOverlaps(true)
    self:EnableCollision(false)
    self:EnablePhysics(false)

end

function Interactive:Interact()

    self:GetParent():Interact()

end

function Interactive:GetHint()

    local hint = "Interact"
    if (self:GetParent().GetInteractionHint) then
        hint = self:GetParent():GetInteractionHint()
    end

    return hint
end