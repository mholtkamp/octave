-- [Script hierarchy]
--   Root (Primitive3D)
--     Camera (Camera3D)
--     Controller (Node, with this script)

FirstPersonController = {}

FirstPersonController.kGroundingDot = 0.9

function FirstPersonController:Create()

    self.collider = Property.Create(DatumType.Node, nil)
    self.camera = Property.Create(DatumType.Node, nil)

    self.gravity = Property.Create(DatumType.Float, -9.8)
    self.moveSpeed = Property.Create(DatumType.Float, 7.0)
    self.lookSpeed = Property.Create(DatumType.Float, 200.0)
    self.jumpSpeed = Property.Create(DatumType.Float, 7.0)
    self.drag = Property.Create(DatumType.Float, 0.001)
    self.enableControl = Property.Create(DatumType.Bool, true)
    self.enableJump = Property.Create(DatumType.Bool, true)
    self.mouseSensitivity = Property.Create(DatumType.Float, 0.05)

    self.moveDir = Vec()
    self.lookVec = Vec()

    self.jumpTimer = 0.0
    self.ignoreGroundingTimer = 0.0
    self.grounded = false
    self.velocity = Vec()

end

function FirstPersonController:Start()

    Input.LockCursor(true)
    Input.TrapCursor(true)
    Input.ShowCursor(false)

    if (not self.collider) then
        self.collider = self:GetParent()
    end

    if (not self.camera) then
        self.camera = self.collider:FindChild("Camera", true)
    end

end

function FirstPersonController:Tick(deltaTime)

    self:UpdateInput(deltaTime)
    self:UpdateJump(deltaTime)
    self:UpdateMovement(deltaTime)
    self:UpdateGrounding(deltaTime)
    self:UpdateLook(deltaTime)

end

function FirstPersonController:UpdateInput(deltaTime)

    if (self.enableControl) then

        -- moveDir
        self.moveDir = Vec()
        if (Input.IsKeyDown(Key.A)) then
            self.moveDir.x = self.moveDir.x + -1.0
        end

        if (Input.IsKeyDown(Key.D)) then
            self.moveDir.x = self.moveDir.x + 1.0
        end

        if (Input.IsKeyDown(Key.W)) then
            self.moveDir.z = self.moveDir.z + -1.0
        end

        if (Input.IsKeyDown(Key.S)) then
            self.moveDir.z = self.moveDir.z + 1.0
        end

        local leftAxisX = Input.GetGamepadAxis(Gamepad.AxisLX)
        local leftAxisY = Input.GetGamepadAxis(Gamepad.AxisLY)

        -- Only add analog stick input beyond a deadzone limit
        if (math.abs(leftAxisX) > 0.1) then
            self.moveDir.x = self.moveDir.x + leftAxisX
        end
        if (math.abs(leftAxisY) > 0.1) then
            self.moveDir.z = self.moveDir.z - leftAxisY
        end

        -- Ensure length of moveDir is at most 1.0.
        local moveMag = self.moveDir:Magnitude()
        moveMag = math.min(moveMag, 1.0)
        self.moveDir = self.moveDir:Normalize()
        self.moveDir = self.moveDir * moveMag

        -- lookDelta
        self.lookVec.x, self.lookVec.y = Input.GetMouseDelta()
        self.lookVec = self.lookVec * self.mouseSensitivity
        local gamepadLook = Vec()
        local rightAxisX = Input.GetGamepadAxis(Gamepad.AxisRX)
        local rightAxisY = Input.GetGamepadAxis(Gamepad.AxisRY)
        if (math.abs(rightAxisX) > 0.1) then
            gamepadLook.x = rightAxisX
        end
        if (math.abs(rightAxisY) > 0.1) then
            gamepadLook.y = -rightAxisY
        end
        self.lookVec = self.lookVec + gamepadLook

    else
        self.moveDir = Vec()
        self.lookDelta = Vec()
    end

end

function FirstPersonController:UpdateJump(deltaTime)

    local jumpPressed = Input.IsKeyPressed(Key.Space) or Input.IsGamepadPressed(Gamepad.A)

    self.jumpTimer = math.max(self.jumpTimer - deltaTime, 0.0)

    if (jumpPressed) then
        self.jumpTimer = 0.2
    end

    if (self.grounded and self.jumpTimer > 0.0) then
        self:Jump()
    end

end

function FirstPersonController:UpdateMovement(deltaTime)

    -- Apply gravity
    if (not self.grounded) then
        self.velocity.y = self.velocity.y + self.gravity * deltaTime
    end

    local moveVelocity = self.moveDir * self.moveSpeed

    local yaw = self.collider:GetRotation().y
    moveVelocity = Vector.Rotate(moveVelocity, yaw, Vec(0,1,0))

    -- First apply motion based on internal move velocity
    moveVelocity = self:Move(moveVelocity, deltaTime)

    -- Then apply motion based on external velocity (like gravity)
    self.velocity = self:Move(self.velocity, deltaTime)


    -- Apply drag on horizontal movement
    if (self.grounded) then
        self.velocity.x = Math.Damp(self.velocity.x, 0, self.drag, deltaTime)
        self.velocity.z = Math.Damp(self.velocity.z, 0, self.drag, deltaTime)
    end

end

function FirstPersonController:UpdateGrounding(deltaTime)

    self.ignoreGroundingTimer = math.max(self.ignoreGroundingTimer - deltaTime, 0.0)

    if (self.grounded) then
        -- Sweep to the ground.
        local startPos = self.collider:GetWorldPosition()
        local endPos = startPos + Vec(0, -0.1, 0)
        local sweepRes = self.collider:SweepToWorldPosition(endPos, 0, true)

        if (sweepRes.hitNode and sweepRes.hitNormal.y > self.kGroundingDot) then
            local pos = startPos + sweepRes.hitFraction * (endPos - startPos) + Vec(0, 0.00101, 0)
            self.collider:SetWorldPosition(pos)
            self:SetGrounded(true)
        else
            self.collider:SetWorldPosition(startPos)
            self:SetGrounded(false)
        end
    end

end

function FirstPersonController:UpdateLook(deltaTime)

    local lookRot = self.lookVec * self.lookSpeed

    -- Adjust yaw of root node
    local rootRot = self.collider:GetRotation()
    rootRot.y = rootRot.y - lookRot.x * deltaTime
    self.collider:SetRotation(rootRot)

    -- Adjust pitch of camera (and possible mesh bone in future)
    local camRot = self.camera:GetRotation()
    camRot.x = camRot.x - lookRot.y * deltaTime
    camRot.x = Math.Clamp(camRot.x, -89.9, 89.9)
    self.camera:SetRotation(camRot)

end

function FirstPersonController:Move(velocity, deltaTime)

    local kMaxIterations = 3

    for i = 1, kMaxIterations do
        local startPos = self.collider:GetWorldPosition()
        local endPos = startPos + velocity * deltaTime
        local sweepRes = self.collider:SweepToWorldPosition(endPos)

        if (sweepRes.hitNode) then

            if (sweepRes.hitNormal.y > self.kGroundingDot) then
                self:SetGrounded(true)
            end

            velocity = velocity - (sweepRes.hitNormal * Vector.Dot(velocity, sweepRes.hitNormal))
            deltaTime = deltaTime * (1.0 - sweepRes.hitFraction)
        else
            break
        end
    end

    return velocity

end

function FirstPersonController:Jump()

    if (self.enableJump and self.grounded) then
        self.velocity.y = self.jumpSpeed
        self:SetGrounded(false)
        self.ignoreGroundingTimer = 0.2
    end

end


function FirstPersonController:SetGrounded(grounded)

    -- Don't allow grounding if we are ignoring it temporarily (just began jumping)
    if (grounded and self.ignoreGroundingTimer > 0.0) then
        return
    end

    if (self.grounded ~= grounded) then
        self.grounded = grounded

        if (self.grounded) then
            self.velocity.y = 0.0
        end
    end
end