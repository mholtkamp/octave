Test6 = 
{
    val1 = 1,
    val2 = 2,

    Create = function(self)
        Log.Info('Test6')
        Log.Console('Platform = ' .. Engine.GetPlatform(), Vec(1.0, 0.2, 1.0, 1.0))
    end,


    Tick = function (self, deltaTime)

        if (Input.IsKeyJustDown(Key.F1)) then
            Audio.PlaySound2D(LoadAsset("SW_Jump"))
        end

        if (Input.IsKeyJustDown(Key.F2)) then
            Audio.PlaySound3D(LoadAsset('SW_Boost'), self.actor:GetPosition(), 3.0, 30.0, 1, 1.0, 1.5, 0.0, true)
        end

        if (Input.IsKeyJustDown(Key.F3)) then
            Audio.StopAllSounds()
        end

        if (Input.IsKeyJustDown(Key.F4)) then
            Log.Debug('7 power of 2: ' .. tostring(Maths.IsPowerOfTwo(7)))
            Log.Debug('8 power of 2: ' .. tostring(Maths.IsPowerOfTwo(8)))
            Log.Debug(tostring(Maths.Damp(0.0, 1.0, 0.005, deltaTime)))
        end

        if (Input.IsKeyJustDown(Key.F5)) then
            local res = Renderer.GetScreenResolution()
            Log.Debug('Resolution = [' .. res.x .. ' x ' .. res.y .. ']')
        end

        if (Input.IsKeyJustDown(Key.F6)) then
            Renderer.AddDebugDraw(LoadAsset('SM_Cone'),
            world:FindActor('Default Camera'):GetPosition(),
            Vec(0,0,0), 
            Vec(1,1,1), 
            Vec(1.0, 0.0, 0.0, 1.0),
            5.0)
        end

        if (Input.IsKeyJustDown(Key.F7)) then
            Log.Debug('Server = ' .. tostring(Network.IsServer()))
            Log.Debug('Client = ' .. tostring(Network.IsClient()))
            Log.Debug('Local = ' .. tostring(Network.IsLocal()))
            Log.Debug('Authority = ' .. tostring(Network.IsAuthority()))
        end

    end,

    Destroy = function(self)
        Log.Debug("Destroy on Test script!!!")
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

    val3 = 3
}

