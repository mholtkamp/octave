
Test3 = 
{
    Create = function(self)
        -- Vector func testing
        local vec1 = Vector.Create(1,2,3,4)
        Log.Debug('vec1 = ' .. tostring(vec1))
        local vec2 = vec1:Create()
        local vec3 = Vector.Create(10, 12)

        local vec4 = vec1 + vec3
        Log.Debug('vec4 = ' .. tostring(vec4))

        vec4 = vec1 + 1
        vec4 = vec1 * vec4
        Log.Debug('vec4 = ' .. tostring(vec4))

        local vec5 = Vector.Create(4,1,5,-2)
        local dotResult = vec5:Dot(vec1)
        Log.Debug('dotResult = ' .. dotResult)

        local vec6 = Vector.Create(1, 0, 0)
        local vec7 = Vector.Create(0, 1, 0)
        Log.Debug('vec6 X vec7 = ' .. tostring(vec6:Cross(vec7)))


        vec7:Set(-1, -2, -3, -4)
        Log.Debug('Set vec7 to :' .. tostring(vec7))


        Log.Debug('lerp(vec7, vec1, 0.5) = ' .. tostring(Vector.Lerp(vec7, vec1, 0.5)))
        Log.Debug('lerp(vec7, vec1, 0.3) = ' .. tostring(Vector.Lerp(vec7, vec1, 0.3)))


        local vec8 = Vector.Create(-10, 11, -12, 13)
        Log.Debug('max(vec1, vec8) = ' .. tostring(Vector.Max(vec1, vec8)))
        Log.Debug('min(vec1, vec8) = ' .. tostring(Vector.Min(vec1, vec8)))
        Log.Debug('clamp(vec8, vec7, vec1 = ' .. tostring(vec8:Clamp(vec7, vec1)))
        Log.Debug('vec7 * 2 = ' .. tostring(vec7 * 2))
        Log.Debug('vec7 / 4 = ' .. tostring(vec7 / 4))
        Log.Debug('vec1 - vec7 = ' .. tostring(vec1 - vec7))
        Log.Debug('vec7 / vec1 = ' .. tostring(vec7 / vec1))
        Log.Debug('vec1 - 2 = ' .. tostring(vec1 - 2))


        local vec9 = Vector.Create(-2.0, -2.0, 0.0)
        local vec10 = Vector.Create(0.0, 1.0, 0.0)
        Log.Debug('normalize(vec9) = ' .. tostring(vec9:Normalize()))
        Log.Debug('reflect(vec9, vec10) = ' .. tostring(Vector.Reflect(vec9, vec10)))
    end
}