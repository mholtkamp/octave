Interactor = {}

Script.Inherit(Interactor, "Primitive3D")


-- [Script hierarchy]
--   Root
--     Interactor (Primitive3D, with this script)

function Interactor:Create()
    self.target = nil
    self.useHintText = Property.Create(DatumType.Bool, true)
end

function Interactor:Start()

    self:EnableOverlaps(true)
    self:EnableCollision(false)
    self:EnablePhysics(false)

    if (self.useHintText) then
        self.hintText = Node.Construct("Text")
        self.hintText:SetVisible(false)
        self.hintText:SetAnchorMode(AnchorMode.FullStretch)
        self.hintText:SetRatios(0.0, 0.85, 1.0, 0.1)
        self.hintText:SetTextSize(20.0)
        self.hintText:SetColor(Vec(1, 0, 0, 1))
        self.hintText:SetHorizontalJustification(Justification.Center)
        self.hintText:Attach(self)
    end

end

function Interactor:Tick(deltaTime)

    if (self.target and (Input.IsKeyPressed(Key.E) or Input.IsGamepadPressed(Gamepad.B))) then
        self.target:Interact()
    end

    -- Update hint text
    if (self.hintText) then
        self.hintText:SetVisible(self.target ~= nil)
        if (self.target) then
            self.hintText:SetText(self.target:GetHint())
        end
    end
end

function Interactor:BeginOverlap(thisNode, otherNode)

    if (otherNode:HasTag("Interactive")) then
        self.target = otherNode
    end

end

function Interactor:EndOverlap(thisNode, otherNode)

    if (self.target == otherNode) then
        self.target = nil
    end

end
