
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

Test2 = { time = 0, yoffset = Vector.Create(0, 0, 0) }

function Test2:Tick(deltaTime)
    --Log.Debug("deltaTime = " .. deltaTime)

    -- if (getmetatable(self) == nil) then 
    --     Log.Debug("Metatable is null!")
    -- else
    --     Log.Debug("Metatable is valid")
    --     Log.Debug(dumpKeys(getmetatable(self)))
    -- end

    --Log.Debug('self = ' .. self)
    --Log.Debug('self metatable = ' .. getmetatable(self))

    --local compMt = getmetatable(self.component) 
    local compMt = getmetatable(self.actor) 

    
    --Log.Debug('self.actor keys: ' .. dumpKeys(self.actor))
    --Log.Debug('self.component keys: ' .. dumpKeys(self.component))


    -- if (compMt == nil) then 
    --     Log.Debug("Metatable is null!")
    -- else
    --     Log.Debug("Metatable is valid " .. compMt.__name)
    --     Log.Debug(dumpKeys(compMt))
    -- end

    --Log.Debug("Name = " .. self.component:GetName())
    local rootComp = self.actor:GetRootComponent()


    local rootActive = math.floor(self.time) % 2 == 1
   rootComp:SetActive(rootActive)

end

