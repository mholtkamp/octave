function dumpKeys(o)
    if type(o) == 'table' then
        local s = '{ '
        for k in pairs(o) do
            s = s .. ' ' .. k .. ' '
        end
        return s .. '} '
     else
        return tostring(o)
     end
end

Test = 
{
    val1 = 1,
    val2 = 2,
    time = 0.0,

    Create = function(self)
        Log.Debug("Create on Test script!!!")

        local lightComp = self.actor:GetComponent('Point Light')

        if (not lightComp) then
            lightComp = self.actor:CreateComponent('PointLightComponent')
            lightComp:SetPosition(Vector.Create(2, 2, 2))
            lightComp:SetRadius(10.0)
            lightComp:SetColor(Vector.Create(2.0, 0.0, 0.0, 1.0))
        end
    end,


    Tick = function (self, deltaTime)

        self.time = self.time + deltaTime
        self.val3 = self.val3 + 1
        if (self.val3 % 100 == 0) then
            Log.Debug('Val3 = ' .. self.val3)
        end

        if (Input.IsKeyJustDown(Key.F1)) then
            Log.Debug('Audio Test')
            local comp = self.actor:GetComponent('Audio Component')

            if (not comp) then comp = self.actor:CreateComponent('AudioComponent') end

            comp:SetSoundWave(LoadAsset("SW_Goal"))
            comp:SetLoop(false)
            comp:Play()

            local duration = comp:GetSoundWave():GetDuration()
            Log.Info('Sound duration = ' .. duration)
        end

        if (Input.IsKeyJustDown(Key.F2)) then
            Log.Debug('Box Test')
            local comp = self.actor:GetComponent('Box Component')

            if (not comp) then comp = self.actor:CreateComponent('BoxComponent') end
            
            comp:SetExtents(Vec(5, 10, 2))
        end

        if (Input.IsKeyJustDown(Key.F3)) then
            Log.Debug('Sphere Test')
            local comp = self.actor:GetComponent('Sphere Component')

            if (not comp) then comp = self.actor:CreateComponent('SphereComponent') end
            
            comp:SetRadius(comp:GetRadius() * 5.75)
        end

        if (Input.IsKeyJustDown(Key.F4)) then
            Log.Debug('Capsule Test')
            local comp = self.actor:GetComponent('Capsule Component')

            if (not comp) then comp = self.actor:CreateComponent('CapsuleComponent') end
            
            comp:SetHeight(10.0)
            comp:SetRadius(comp:GetHeight() / 3.0)
        end

        if (Input.IsKeyJustDown(Key.F5)) then
            Log.Debug('Particle Test')
            local comp = self.actor:GetComponent('TestPart')

            if (not comp) then
                local ps = ParticleSystemInstance.Create('P_GoalExplosion')
                ps:SetColorStart(Vec(0.0, 1.0, 0.0, 1.0))
                ps:SetColorEnd(Vec(0.0, 0.2, 1.0, 1.0))
                comp = self.actor:CreateComponent('ParticleComponent')
                comp:SetName('TestPart')
                comp:SetParticleSystem(ps)
            end

            comp:EnableEmission(true)
        end

        if (Input.IsKeyJustDown(Key.F6)) then
            Log.Debug('Scrip Test')

            local spawnLoc = Vec(math.random(), math.random(), math.random()) * 50 - 25.0

            local newActor = world:SpawnActor('Actor')
            local newMesh = newActor:CreateComponent('StaticMeshComponent')
            newMesh:SetStaticMesh(LoadAsset('SM_GH_PalmTree'))
            newMesh:SetScale(Vec(0.3, 0.3, 0.3))
            newActor:SetPosition(spawnLoc)

            local newScript = newActor:CreateComponent('ScriptComponent')
            newScript:SetFile('Test5')
            newScript:RestartScript()
        end

        if (Input.IsKeyJustDown(Key.F7)) then

            self.parentWidget = Widget.Create()

            self.testQuad = Quad.Create()
            self.testQuad:SetDimensions(200, 100)
            self.testQuad:SetTexture(LoadAsset('T_Splash'))
            self.testQuad:SetTint(Vec(1.0, 0.0, 0.0, 1.0))
            self.testQuad:SetPosition(400, 50)

            self.testText = Text.Create()
            self.testText:SetSize(30)
            self.testText:SetPosition(200, 500)
            self.testText:SetText('This is a test widget!\nNo way')

            self.testButton = Button.Create()
            self.testButton:SetTextString('Beep I\'m Button')
            self.testButton:SetHoverHandler('ButtonHover')
            self.testButton:SetPosition(400, 400)

            self.testCombo = ComboBox.Create()
            self.testCombo:SetPosition(333, 200)
            self.testCombo:AddSelection('One')
            self.testCombo:AddSelection('2')
            self.testCombo:AddSelection('THREE')
            self.testCombo:AddSelection('Four?')
            self.testCombo:SetSelectionIndex(3)
            self.testCombo:SetSelectionChangeHandler('SelChange')

            self.testField = TextField.Create()
            self.testField:SetTextEditHandler('TextEdit')
            self.testField:SetTextConfirmHandler('TextConfirm')
            self.testField:SetPosition(200, 600)

            self.parentWidget:AddChild(self.testQuad)
            self.parentWidget:AddChild(self.testText)
            self.parentWidget:AddChild(self.testButton)
            self.parentWidget:AddChild(self.testCombo)
            self.parentWidget:AddChild(self.testField)
            

            --[[
            local gcTest = Text.Create()

            local debugMt = getmetatable(self.testText)
            dumpKeys(debugMt)
            ]]--

            --Renderer.AddWidget(self.testQuad)
            --Renderer.AddWidget(self.testText)
            Renderer.AddWidget(self.parentWidget)
        end

        if (Input.IsKeyJustDown(Key.F8)) then
            --collectgarbage()

            if (self.test2Script) then
                if (self.test2Script.destroyed) then
                    Log.Warning('Clearing self.test2Script because it has been destroyed')
                    Log.Warning('self.test2Script.actor = ' .. tostring(self.test2Script.actor))
                    Log.Warning('self.test2Script.component = ' .. tostring(self.test2Script.component))
                    self.test2Script = nil
                else
                    Log.Info('self.test2Script.actor = ' .. tostring(self.test2Script.actor))
                    Log.Info('self.test2Script.component = ' .. tostring(self.test2Script.component))
                end
            end

            local testActor = world:FindActor('TestActor')
            if (testActor) then
                self.test2Script = testActor:GetScript('Test2')
                if self.test2Script then
                    self.test2Script.timeScale = 10.0
                else
                    Log.Warning('Could not grab Test2 script from TestActor')
                end
            end
        end

        if (Input.IsKeyJustDown(Key.F9)) then
            world:EnableInternalEdgeSmoothing(not world:IsInternalEdgeSmoothingEnabled())
            Log.Debug('Edge smoothing = ' .. tostring(world:IsInternalEdgeSmoothingEnabled()))
        end

        if (self.parentWidget) then
            self.parentWidget:SetPosition(math.sin(self.time) * 200.0 + 200, 0.0)
        end

    end,

    Destroy = function(self)
        Log.Debug("Destroy on Test script!!!")

        if (self.parentWidget) then
            Renderer.RemoveWidget(self.parentWidget)
        end
    end,

    BeginPlay = function(self)
        Log.Debug("Begin Play on Test script!!!")
    end,
    
    EndPlay = function(self)
        Log.Debug("End Play on Test script!!!")
    end,

    BeginOverlap = function(self, thisComp, otherComp)
        Log.Debug("Begin Overlap!! " .. thisComp:GetName() .. " + " .. otherComp:GetName())
    end,

    EndOverlap = function(self, thisComp, otherComp)
        Log.Debug("End Overlap!! " .. thisComp:GetName() .. " + " .. otherComp:GetName())
    end,

    OnCollision = function(self, thisComp, otherComp, impactPosition, impactNormal)
        Log.Debug("OnCollision!! " .. thisComp:GetName() .. " + " .. otherComp:GetName() .. " at " .. tostring(impactPosition))
    end,

    ButtonHover = function(self, button)
        Log.Debug('Button hovered!!')
        button:SetTextString('HOVER')
    end,

    SelChange = function(self, selector)
        Log.Debug('New sel = ' .. selector:GetSelectionString())
        selector:GetQuad():SetColor(Vec(0.0, 1.0, 0.2, 1.0))
    end,

    TextEdit = function(self, tf)
        Log.Debug('Edit: ' .. tf:GetTextString())
    end,

    TextConfirm = function(self, tf)
        Log.Warning('Confirm: ' .. tf:GetTextString())
    end,

    val3 = 3
}

