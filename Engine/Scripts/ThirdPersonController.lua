-- [Script hierarchy]
--   Root (Primitive3D)
--     Camera (Camera3D)
--     Controller (Node, with this script)

ThirdPersonController = {}

ThirdPersonController.kGroundingDot = 0.9

function ThirdPersonController:Create()

    -- Properties
    self.collider = nil
    self.camera = nil
    self.cameraPivot = nil
    self.mesh = nil
    self.cameraDistance = 10.0
    self.gravity = -9.8
    self.moveSpeed = 7.0
    self.lookSpeed = 200.0
    self.moveAccel = 100.0
    self.jumpSpeed = 7.0
    self.jumpGravScale = 0.5
    self.moveDrag = 100.0
    self.extDrag = 20.0
    self.airControl = 0.1
    self.enableControl = true
    self.enableJump = true
    self.enableFollowCam = false
    self.enableCameraTrace = true
    self.mouseSensitivity = 0.05

    -- State
    self.moveDir = Vec()
    self.lookVec = Vec()
    self.jumpTimer = 0.0
    self.isJumping = false
    self.isJumpHeld = false
    self.ignoreGroundingTimer = 0.0
    self.timeSinceGrounded = 0.0
    self.grounded = false
    self.extVelocity = Vec()
    self.moveVelocity = Vec()
    self.meshYaw = 0.0

end

function ThirdPersonController:GatherProperties()

    return
    {
        { name = "collider", type = DatumType.Node },
        { name = "camera", type = DatumType.Node },
        { name = "cameraPivot", type = DatumType.Node },
        { name = "mesh", type = DatumType.Node },
        { name = "cameraDistance", type = DatumType.Float },
        { name = "gravity", type = DatumType.Float },
        { name = "moveSpeed", type = DatumType.Float },
        { name = "lookSpeed", type = DatumType.Float },
        { name = "moveAccel", type = DatumType.Float },
        { name = "jumpSpeed", type = DatumType.Float },
        { name = "jumpGravScale", type = DatumType.Float },
        { name = "moveDrag", type = DatumType.Float },
        { name = "extDrag", type = DatumType.Float },
        { name = "airControl", type = DatumType.Float },
        { name = "enableControl", type = DatumType.Bool },
        { name = "enableJump", type = DatumType.Bool },
        { name = "enableFollowCam", type = DatumType.Bool },
        { name = "enableCameraTrace", type = DatumType.Bool },
        { name = "mouseSensitivity", type = DatumType.Float },
    }

end

function ThirdPersonController:Start()

    self:AddTag("Controller")

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

function ThirdPersonController:Stop()

    Input.LockCursor(false)
    Input.TrapCursor(false)
    Input.ShowCursor(true)

end

function ThirdPersonController:Tick(deltaTime)

    self:UpdateInput(deltaTime)
    self:UpdateJump(deltaTime)
    self:UpdateDrag(deltaTime)
    self:UpdateMovement(deltaTime)
    self:UpdateGrounding(deltaTime)
    self:UpdateLook(deltaTime)
    self:UpdateMesh(deltaTime)

end

function ThirdPersonController:UpdateInput(deltaTime)

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
        local rightAxisDeadZone = 0.1
        if (math.abs(rightAxisX) > rightAxisDeadZone) then
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

function ThirdPersonController:UpdateJump(deltaTime)

    local jumpPressed = Input.IsKeyPressed(Key.Space) or Input.IsGamepadPressed(Gamepad.A)

    if (self.isJumpHeld) then
        self.isJumpHeld = Input.IsKeyDown(Key.Space) or Input.IsGamepadDown(Gamepad.A)
    end

    self.jumpTimer = math.max(self.jumpTimer - deltaTime, 0.0)

    if (jumpPressed) then
        self.jumpTimer = 0.2
    end

    if (self.grounded and self.jumpTimer > 0.0) then
        self:Jump()
    end

end

function ThirdPersonController:UpdateDrag(deltaTime)

    -- Update drag
    local function updateDrag(velocity, drag)
        local velXZ = Vec(velocity.x, 0, velocity.z)
        local speed = velXZ:Magnitude()
        local dir = speed > 0.0  and (velXZ / speed) or Vec()
        speed = math.max(speed - drag * deltaTime, 0)
        return dir * speed
    end

    if (self.grounded) then
        -- Only apply move drag when player is not moving
        if (self.moveDir == Vec(0,0,0)) then
            self.moveVelocity = updateDrag(self.moveVelocity, self.moveDrag)
        end
        self.extVelocity = updateDrag(self.extVelocity, self.extDrag)
    end

end

function ThirdPersonController:UpdateMovement(deltaTime)

    -- Apply gravity
    if (not self.grounded) then
        local gravity = self.gravity
        if (self.isJumping and self.isJumpHeld) then
            gravity = gravity * self.jumpGravScale
        end
        self.extVelocity.y = self.extVelocity.y + gravity * deltaTime
    end

    -- Add velocity based on player input vector
    local deltaMoveVel = self.moveDir * self.moveAccel * deltaTime
    local yaw = self.cameraPivot:GetRotation().y
    deltaMoveVel = Vector.Rotate(deltaMoveVel, yaw, Vec(0,1,0))

    -- Reduce move velocity when in air
    if (not self.grounded) then
        deltaMoveVel = deltaMoveVel * self.airControl
    end

    self.moveVelocity = self.moveVelocity + deltaMoveVel

    if (self.moveVelocity:Magnitude() > self.moveSpeed) then
        self.moveVelocity = self.moveVelocity:Normalize() * self.moveSpeed
    end

    -- First apply motion based on internal move velocity
    self.moveVelocity = self:Move(self.moveVelocity, deltaTime, 0.3)

    -- Then apply motion based on external velocity (like gravity)
    self.extVelocity = self:Move(self.extVelocity, deltaTime, 0.0)

end

function ThirdPersonController:UpdateGrounding(deltaTime)

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
    else
        self.timeSinceGrounded = self.timeSinceGrounded + deltaTime
    end
end

function ThirdPersonController:UpdateLook(deltaTime)

    local lookRot = self.lookVec * self.lookSpeed

    -- Adjust rotation of camera pivot based on player input
    local camPivotRot = self.cameraPivot:GetRotation()
    camPivotRot.y = camPivotRot.y - lookRot.x * deltaTime
    camPivotRot.x = camPivotRot.x - lookRot.y * deltaTime
    camPivotRot.x = Math.Clamp(camPivotRot.x, -89.9, 89.9)
    self.cameraPivot:SetRotation(camPivotRot)

    -- Adjust camera distance
    self.camera:SetPosition(Vec(0, 0, self.cameraDistance))

    if (self.enableCameraTrace) then
        local rayStart = self.collider:GetWorldPosition()
        local rayEnd = self.camera:GetWorldPosition()
        local colMask = 0x02 -- Default collision group for environment
        local res = self.world:RayTest(rayStart, rayEnd, colMask)

        if (res.hitNode) then
            -- The trace hit something, so place the camera at the hit location
            local pos = Vector.Lerp(rayStart, rayEnd, res.hitFraction)
            self.camera:SetWorldPosition(pos)
        end
    end
end

function ThirdPersonController:UpdateMesh(deltaTime)

    -- Update orientation of the mesh if the player is moving
    if (math.abs(self.moveDir.x) >= 0.01 or
        math.abs(self.moveDir.z) >= 0.01) then

        local camYaw = self.cameraPivot:GetRotation().y
        local moveDir = Vector.Rotate(self.moveDir, camYaw, Vec(0,1,0))

        local targetYaw = math.atan(-moveDir.x, -moveDir.z)
        targetYaw = math.deg(targetYaw)

        self.meshYaw = Math.ApproachAngle(self.meshYaw, targetYaw, 1000.0, deltaTime)
        self.mesh:SetRotation(Vec(0, self.meshYaw, 0))
    end

    -- Update looping animation
    if (not self.grounded and self.timeSinceGrounded > 0.1) then
        -- Don't play fall animation if jump animation is playing.
        -- Wait until it finishes so that we get a seamless transition between
        -- the end of the jump animation and the beginning of the fall animation.
        if (not self.mesh:IsAnimationPlaying("Jump")) then
            self.mesh:PlayAnimation("Fall", 0, true, 1, 1)
        end
    elseif (self.moveVelocity:Length2() > 1.0) then
        self.mesh:PlayAnimation("Run", 0, true, 1.5, 1)
    else
        self.mesh:PlayAnimation("Idle", 0, true, 1, 1)
    end
end

function ThirdPersonController:Move(velocity, deltaTime, vertSlideNormalLimit)

    local kMaxIterations = 3

    for i = 1, kMaxIterations do
        local startPos = self.collider:GetWorldPosition()
        local endPos = startPos + velocity * deltaTime
        local sweepRes = self.collider:SweepToWorldPosition(endPos)

        if (sweepRes.hitNode) then

            if (sweepRes.hitNormal.y > self.kGroundingDot) then
                self:SetGrounded(true)
            end

            local initialVelocityY = velocity.y

            velocity = velocity - (sweepRes.hitNormal * Vector.Dot(velocity, sweepRes.hitNormal))
            deltaTime = deltaTime * (1.0 - sweepRes.hitFraction)

            if (math.abs(sweepRes.hitNormal.y) < vertSlideNormalLimit) then
                velocity.y = initialVelocityY
            end
        else
            break
        end
    end

    return velocity

end

function ThirdPersonController:Jump()

    if (self.enableJump and self.grounded) then
        self.isJumping = true
        self.isJumpHeld = true
        self.extVelocity.y = self.jumpSpeed
        self:SetGrounded(false)
        self.ignoreGroundingTimer = 0.2

        self.mesh:StopAnimation("Fall")
        self.mesh:PlayAnimation("Jump", 1, false)
        self.mesh:QueueAnimation("Fall", "Jump", 0, true, 1, 1)
    end

end


function ThirdPersonController:SetGrounded(grounded)

    -- Don't allow grounding if we are ignoring it temporarily (just began jumping)
    if (grounded and self.ignoreGroundingTimer > 0.0) then
        return
    end

    if (self.grounded ~= grounded) then
        self.grounded = grounded

        if (self.grounded) then
            self.extVelocity.y = 0.0
            self.timeSinceGrounded = 0.0
            self.isJumping = false
        end
    end
end