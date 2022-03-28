
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

Test5 = { time = 0, yoffset = Vector.Create(0, 0, 0) }

function Test5:Create(deltaTime)

end


function Test5:Tick(deltaTime)

   local rootComp = self.actor:GetRootComponent()
   self.time = self.time + deltaTime
   rootComp:AddRotation(Vec(0.0, 200.0, 0.0) * deltaTime)



   --[[
   if (Input.IsKeyDown(Key.F1)) then
      local testSphere = Find
   end
   ]]--

end

