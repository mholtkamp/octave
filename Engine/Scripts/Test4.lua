
function dump(o)
    if type(o) == 'table' then
       local s = '{ '
       for k,v in pairs(o) do
          if type(k) ~= 'number' then k = '"'..k..'"' end
          s = s .. '['..k..'] = ' .. dump(v) .. ','
       end
       return s .. '} '
    else
       return tostring(o)
    end
 end


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

Test4 = { time = 0 }

function Test4:Create()

    local skelMesh = self.actor:GetComponent('Skeletal Mesh')

    if (skelMesh) then
        skelMesh:PlayAnimation("Attack0", true)
        skelMesh:SetAnimEventHandler('AnimFunc')
    else
        Log.Debug("Could not find Skmesh comp")
    end

    self.animSpeed = 1.0
    Log.Debug(self.animSpeed)
end

function Test4:Tick(deltaTime)

    self.time = self.time + deltaTime

    local skelMesh = self.actor:GetComponent('Skeletal Mesh')

    if (Input.IsKeyDown(Key.Space)) then
        skelMesh:StopAnimation('Attack0')
    end

    if (Input.IsKeyDown(Key.N2)) then
        self.animSpeed = self.animSpeed + deltaTime * 1.0
    elseif (Input.IsKeyDown(Key.N1)) then
        self.animSpeed = self.animSpeed - deltaTime * 1.0
    end

    skelMesh:SetAnimationSpeed(self.animSpeed)

end

function Test4:AnimFunc(actor, component, eventName, animName, time, value)
    Log.Debug("AnimEvent handler!!!")
    Log.Debug(eventName ..": " .. tostring(value))
end

